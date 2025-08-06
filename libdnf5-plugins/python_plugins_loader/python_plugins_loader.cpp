// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include <Python.h>
#include <fmt/format.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/plugin/iplugin.hpp>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>

using namespace libdnf5;

namespace fs = std::filesystem;

namespace {

constexpr const char * PLUGIN_NAME = "python_plugins_loader";
constexpr plugin::Version PLUGIN_VERSION{0, 1, 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Jaroslav Rohel", "jrohel@redhat.com", "Plugin for loading Python plugins."};

class PythonPluginLoader : public plugin::IPlugin {
public:
    PythonPluginLoader(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin(data), data(data) {}
    virtual ~PythonPluginLoader();

    PluginAPIVersion get_api_version() const noexcept override { return REQUIRED_PLUGIN_API_VERSION; }

    const char * get_name() const noexcept override { return PLUGIN_NAME; }

    plugin::Version get_version() const noexcept override { return PLUGIN_VERSION; }

    const char * const * get_attributes() const noexcept override { return attrs; }

    const char * get_attribute(const char * attribute) const noexcept override {
        for (size_t i = 0; attrs[i]; ++i) {
            if (std::strcmp(attribute, attrs[i]) == 0) {
                return attrs_value[i];
            }
        }
        return nullptr;
    }

    void load_plugins() override;

private:
    void load_plugin_file(const fs::path & file);
    void load_plugins_from_dir(const fs::path & dir_path);

    static int python_ref_counter;
    bool active{false};
    // Store the plugin data so we can pass them to each python plugin
    libdnf5::plugin::IPluginData & data;
};

int PythonPluginLoader::python_ref_counter{0};


/// Smart pointer to PyObject
///
/// Owns and manages PyObject. Decrements the reference count for the PyObject
/// (calls Py_XDECREF on it) when  UniquePtrPyObject goes out of scope.
///
/// Implements subset of standard std::unique_ptr methods.
///
class UniquePtrPyObject {
public:
    constexpr UniquePtrPyObject() noexcept : py_obj(NULL) {}
    explicit UniquePtrPyObject(PyObject * pyObj) noexcept : py_obj(pyObj) {}
    UniquePtrPyObject(UniquePtrPyObject && src) noexcept : py_obj(src.py_obj) { src.py_obj = NULL; }
    UniquePtrPyObject & operator=(UniquePtrPyObject && src) noexcept;
    explicit operator bool() const noexcept { return py_obj != NULL; }
    PyObject * get() const noexcept { return py_obj; }
    PyObject * release() noexcept;
    void reset(PyObject * pyObj = NULL) noexcept;
    ~UniquePtrPyObject() { Py_XDECREF(py_obj); }

private:
    PyObject * py_obj;
};

inline PyObject * UniquePtrPyObject::release() noexcept {
    auto tmpObj = py_obj;
    py_obj = NULL;
    return tmpObj;
}

inline UniquePtrPyObject & UniquePtrPyObject::operator=(UniquePtrPyObject && src) noexcept {
    if (this == &src)
        return *this;
    Py_XDECREF(py_obj);
    py_obj = src.py_obj;
    src.py_obj = NULL;
    return *this;
}

inline void UniquePtrPyObject::reset(PyObject * pyObj) noexcept {
    Py_XDECREF(this->py_obj);
    this->py_obj = pyObj;
}


/// bytes or unicode string in Python 3 to c string converter
class PycompString {
public:
    PycompString() = default;
    explicit PycompString(PyObject * str);
    const std::string & get_string() const noexcept { return cpp_string; }
    const char * get_cstring() const noexcept { return is_null ? nullptr : cpp_string.c_str(); }

private:
    bool is_null{true};
    std::string cpp_string;
};

PycompString::PycompString(PyObject * str) {
    if (PyUnicode_Check(str)) {
        UniquePtrPyObject py_string(PyUnicode_AsEncodedString(str, "utf-8", "replace"));
        if (py_string) {
            // The c_string refers to the internal buffer of py_string
            char * c_string = PyBytes_AsString(py_string.get());
            if (c_string) {
                cpp_string = c_string;
                is_null = false;
            }
        }
    } else if (PyBytes_Check(str)) {
        auto c_string = PyBytes_AsString(str);
        if (c_string) {
            cpp_string = c_string;
            is_null = false;
        }
    } else {
        throw std::runtime_error("Expected a string or a unicode object");
    }
}

PythonPluginLoader::~PythonPluginLoader() {
    if (active) {
        std::lock_guard<libdnf5::Base> guard(get_base());
        if (--python_ref_counter == 0) {
            Py_Finalize();
        }
    }
}

/// Fetch Python error (if exist) and throw C++ exception
static void fetch_python_error_to_exception(const char * msg) {
    if (!PyErr_Occurred()) {
        return;
    }
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    UniquePtrPyObject objectsRepresentation(PyObject_Repr(value));
    auto pycomp_str = PycompString(objectsRepresentation.get());
    throw std::runtime_error(msg + pycomp_str.get_string());
}

/// Load Python plugin from path
void PythonPluginLoader::load_plugin_file(const fs::path & file_path) {
    // Very High Level Embedding
    // std::string python_code = "import " + file_path.stem().string() +";";
    // python_code += "import libdnf5;";
    // python_code += "plug = " + file_path.stem().string() +".Plugin();";
    // python_code += "locked_base = libdnf5.base.Base.get_locked_base();";
    // python_code += "locked_base.add_plugin(plug)";
    // PyRun_SimpleString(python_code.c_str());

    // Similar but Pure Embedding
    auto * module_name = file_path.stem().c_str();
    PyObject * plugin_module = PyImport_ImportModule(module_name);
    if (!plugin_module) {
        fetch_python_error_to_exception("PyImport_ImportModule(): ");
    }
    PyObject * plugin_module_dict = PyModule_GetDict(plugin_module);
    if (!plugin_module_dict) {
        fetch_python_error_to_exception("PyModule_GetDict(plugin_module): ");
    }
    PyObject * plugin_class = PyDict_GetItemString(plugin_module_dict, "Plugin");
    if (!plugin_class) {
        fetch_python_error_to_exception("PyDict_GetItemString(plugin_module_dict, \"Plugin\"): ");
    }
    PyObject * plugin_class_constructor = PyInstanceMethod_New(plugin_class);
    if (!plugin_class_constructor) {
        fetch_python_error_to_exception("PyInstanceMethod_New(plugin_class): ");
    }
    PyObject * plugin_instance = PyObject_CallObject(plugin_class_constructor, nullptr);
    if (!plugin_instance) {
        fetch_python_error_to_exception("PyObject_CallObject(plugin_class_constructor, nullptr): ");
    }
    PyObject * libdnf5 = PyDict_GetItemString(plugin_module_dict, "libdnf5");
    if (!libdnf5) {
        fetch_python_error_to_exception("PyDict_GetItemString(plugin_module_dict, \"libdnf5\"): ");
    }
    PyObject * libdnf5_dict = PyModule_GetDict(libdnf5);
    if (!libdnf5_dict) {
        fetch_python_error_to_exception("PyModule_GetDict(libdnf5): ");
    }
    PyObject * base_module = PyDict_GetItemString(libdnf5_dict, "base");
    if (!base_module) {
        fetch_python_error_to_exception("PyDict_GetItemString(plugin_module_dict, \"libdnf5\"): ");
    }
    PyObject * base_module_dict = PyModule_GetDict(base_module);
    if (!base_module_dict) {
        fetch_python_error_to_exception("PyModule_GetDict(base_module): ");
    }
    PyObject * base_class = PyDict_GetItemString(base_module_dict, "Base");
    if (!base_class) {
        fetch_python_error_to_exception("PyDict_GetItemString(base_module_dict, \"Base\"): ");
    }
    UniquePtrPyObject locked_base(PyObject_CallMethod(base_class, "get_locked_base", NULL));
    if (!locked_base) {
        fetch_python_error_to_exception("PyDict_CallMethod(base_class, \"get_locked_base\", NULL): ");
    }
    UniquePtrPyObject add_plugin_string(PyUnicode_FromString("add_plugin"));
    PyObject_CallMethodObjArgs(locked_base.get(), add_plugin_string.get(), plugin_instance, NULL);
}


void PythonPluginLoader::load_plugins_from_dir(const fs::path & dir_path) {
    auto & logger = *get_base().get_logger();

    if (dir_path.empty())
        throw std::runtime_error("PythonPluginLoader::load_from_dir() dir_path cannot be empty");

    std::vector<fs::path> lib_names;
    std::error_code ec;
    for (auto & p : std::filesystem::directory_iterator(dir_path, ec)) {
        if ((p.is_regular_file() || p.is_symlink()) && p.path().extension() == ".py") {
            lib_names.emplace_back(p.path());
        }
    }
    if (ec) {
        logger.warning("PythonPluginLoader: Cannot read plugins directory \"{}\": {}", dir_path.string(), ec.message());
        return;
    }
    std::sort(lib_names.begin(), lib_names.end());

    std::string error_msgs;
    for (auto & p : lib_names) {
        try {
            load_plugin_file(p);
        } catch (const std::exception & ex) {
            std::string msg = fmt::format("Cannot load plugin \"{}\": {}", p.string(), ex.what());
            logger.error(msg);
            error_msgs += msg + '\n';
        }
    }

    if (!error_msgs.empty()) {
        throw std::runtime_error(error_msgs);
    }
}

void PythonPluginLoader::load_plugins() {
    const char * plugin_dir = std::getenv("LIBDNF_PYTHON_PLUGIN_DIR");
    if (!plugin_dir) {
        return;
    }
    const fs::path path(plugin_dir);

    std::lock_guard<libdnf5::Base> guard(get_base());

    if (python_ref_counter == 0) {
        Py_InitializeEx(0);
        if (!Py_IsInitialized()) {
            return;
        }
    }
    active = true;
    ++python_ref_counter;

    // PyRun_SimpleString("import sys");
    PyObject * sys_module = PyImport_ImportModule("sys");
    if (!sys_module) {
        fetch_python_error_to_exception("PyImport_ImportModule(): ");
    }

    // PyRun_SimpleString(("sys.path.append('" + path.string() + "')").c_str());
    PyObject * sys_dict = PyModule_GetDict(sys_module);
    if (!sys_dict) {
        fetch_python_error_to_exception("PyModule_GetDict(sys_module): ");
    }
    PyObject * path_object = PyDict_GetItemString(sys_dict, "path");
    if (!path_object) {
        fetch_python_error_to_exception("PyDict_GetItemString(sys_dict, \"path\"): ");
    }
    UniquePtrPyObject append(PyObject_CallMethod(path_object, "append", "(s)", path.c_str()));
    if (!append) {
        fetch_python_error_to_exception(
            ("PyDict_CallMethod(path_object, \"append\", \"(s)\", " + path.string() + "): ").c_str());
    }

    load_plugins_from_dir(path);
}


std::exception_ptr last_exception;

}  // namespace


PluginAPIVersion libdnf_plugin_get_api_version(void) {
    return REQUIRED_PLUGIN_API_VERSION;
}

const char * libdnf_plugin_get_name(void) {
    return PLUGIN_NAME;
}

plugin::Version libdnf_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

plugin::IPlugin * libdnf_plugin_new_instance(
    [[maybe_unused]] LibraryVersion library_version,
    libdnf5::plugin::IPluginData & data,
    libdnf5::ConfigParser & parser) try {
    return new PythonPluginLoader(data, parser);
} catch (...) {
    last_exception = std::current_exception();
    return nullptr;
}

void libdnf_plugin_delete_instance(plugin::IPlugin * plugin_object) {
    delete plugin_object;
}

std::exception_ptr * libdnf_plugin_get_last_exception(void) {
    return &last_exception;
}
