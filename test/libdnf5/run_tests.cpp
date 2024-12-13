/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "../shared/base_test_case.hpp"
#include "../shared/test_logger.hpp"

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <libdnf5/logger/memory_buffer_logger.hpp>
#include <libdnf5/logger/stream_logger.hpp>

#include <chrono>
#include <iostream>
#include <memory>


class TimingListener : public CppUnit::TestListener {
public:
    void startTest(CppUnit::Test *) override { start = std::chrono::high_resolution_clock::now(); }

    void endTest(CppUnit::Test *) override {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << " (duration: " << duration << "ms)";
    }

private:
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::from_time_t(0);
};


class LogCaptureListener : public CppUnit::TestListener {
public:
    void startTest(CppUnit::Test *) override {
        // Global test_logger is used. Clear it befor starting new test.
        test_logger.clear();
    }

    void addFailure(const CppUnit::TestFailure &) override {
        std::cout << std::endl << "Dnf log:" << std::endl;
        libdnf5::StdCStreamLogger cout_logger(std::cout);
        test_logger.write_to_logger(cout_logger);
        std::cout << std::endl;
    }
};


int main(int argc, char * argv[]) {
    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // Uncomment to stop cppunit from catching exceptions (for e.g. gdb debugging)
    //controller.popProtector();

    // Add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    TimingListener timer;
    controller.addListener(&timer);

    LogCaptureListener log_capture;
    controller.addListener(&log_capture);

    // Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    std::string test_path;

    if (argc > 1) {
        test_path = argv[1];
    }

    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller, test_path);

    // Print test in a compiler compatible format.
    CPPUNIT_NS::CompilerOutputter outputter(&result, CPPUNIT_NS::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
