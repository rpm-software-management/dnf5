/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "libdnf5/rpm/nevra.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <rpm/rpmver.h>


namespace libdnf5::rpm {


static const std::vector<Nevra::Form> PKG_SPEC_FORMS{
    Nevra::Form::NEVRA, Nevra::Form::NA, Nevra::Form::NAME, Nevra::Form::NEVR, Nevra::Form::NEV};

const std::vector<Nevra::Form> & Nevra::get_default_pkg_spec_forms() {
    return PKG_SPEC_FORMS;
}

std::vector<Nevra> Nevra::parse(const std::string & nevra_str, const std::vector<Form> & forms) {
    std::vector<Nevra> result;
    const char * before_last_delim = nullptr;
    const char * epoch_delim = nullptr;
    const char * last_delim = nullptr;
    const char * arch_delim = nullptr;
    const char * end;

    // parse nevra
    const char * nevra_pattern = nevra_str.c_str();
    // detect whether string contains a glob range [a-z]
    bool start_range = false;
    for (end = nevra_pattern; *end != '\0'; ++end) {
        // skip all characteres before glob range is closed
        if (start_range) {
            if (*end == ']') {
                start_range = false;
            } else {
                continue;
            }
        }
        if (*end == '[') {
            start_range = true;
        } else if (*end == '-') {
            before_last_delim = last_delim;
            last_delim = end;
        } else if (*end == '.') {
            arch_delim = end;
        } else if (*end == ':') {
            // ':' can be only once in nevra
            if (epoch_delim != nullptr) {
                throw NevraIncorrectInputError(M_("NEVRA string \"{}\" contains ':' multiple times"), nevra_str);
            }
            epoch_delim = end;
        } else if (*end == '(' || *end == '/' || *end == '=' || *end == '<' || *end == '>' || *end == ' ') {
            throw NevraIncorrectInputError(M_("Invalid character '{}' in NEVRA string \"{}\""), *end, nevra_str);
        }
    }
    for (auto form : forms) {
        Nevra nevra;
        switch (form) {
            case Form::NEVRA: {
                if (before_last_delim == nullptr || last_delim == nullptr || arch_delim == nullptr) {
                    continue;
                }

                // test name presence
                if (before_last_delim == nevra_pattern) {
                    continue;
                }

                const char * evr_delim_for_nevra = before_last_delim;
                const char * release_delim_for_nevra = last_delim;
                const char * arch_delim_for_nevra = arch_delim;

                nevra.name.assign(nevra_pattern, evr_delim_for_nevra);
                ++evr_delim_for_nevra;

                // test presence of epoch (optional)
                if (epoch_delim != nullptr) {
                    // test that ':' was in range of evr. ':' sign is only allowed in evr as an epoch deliminator
                    if (epoch_delim - evr_delim_for_nevra < 1 || release_delim_for_nevra - epoch_delim < 2) {
                        continue;
                    }
                    nevra.epoch.assign(evr_delim_for_nevra, epoch_delim);
                    evr_delim_for_nevra = epoch_delim + 1;
                }

                // test presence of version
                if (release_delim_for_nevra - evr_delim_for_nevra < 1) {
                    continue;
                }
                nevra.version.assign(evr_delim_for_nevra, release_delim_for_nevra);
                ++release_delim_for_nevra;

                // test presence of release
                if (arch_delim_for_nevra - release_delim_for_nevra < 1) {
                    continue;
                }
                nevra.release.assign(release_delim_for_nevra, arch_delim_for_nevra);
                ++arch_delim_for_nevra;

                // test presence of arch
                if (end - arch_delim_for_nevra < 1) {
                    continue;
                }
                nevra.arch.assign(arch_delim_for_nevra);

                result.push_back(std::move(nevra));
            } break;
            case Form::NEVR: {
                if (before_last_delim == nullptr || last_delim == nullptr) {
                    continue;
                }

                // test name presence
                if (before_last_delim == nevra_pattern) {
                    continue;
                }

                const char * evr_delim_for_nevr = before_last_delim;
                const char * release_delim_for_nevr = last_delim;

                nevra.name.assign(nevra_pattern, evr_delim_for_nevr);
                ++evr_delim_for_nevr;

                // test presence of epoch (optional)
                if (epoch_delim != nullptr) {
                    // test that ':' was in range of evr. ':' sign is only allowed in evr as an epoch deliminator
                    if (epoch_delim - evr_delim_for_nevr < 1 || release_delim_for_nevr - epoch_delim < 2) {
                        continue;
                    }
                    nevra.epoch.assign(evr_delim_for_nevr, epoch_delim);
                    evr_delim_for_nevr = epoch_delim + 1;
                }

                // test presence of version
                if (release_delim_for_nevr - evr_delim_for_nevr < 1) {
                    continue;
                }
                nevra.version.assign(evr_delim_for_nevr, release_delim_for_nevr);
                ++release_delim_for_nevr;

                // test presence of release
                if (end - release_delim_for_nevr < 1) {
                    continue;
                }
                nevra.release.assign(release_delim_for_nevr);

                result.push_back(std::move(nevra));
            } break;
            case Form::NEV: {
                if (last_delim == nullptr) {
                    continue;
                }

                // test name presence
                if (before_last_delim == nevra_pattern) {
                    continue;
                }

                const char * evr_delim_for_nev = last_delim;

                nevra.name.assign(nevra_pattern, evr_delim_for_nev);
                ++evr_delim_for_nev;

                // test presence of epoch (optional)
                if (epoch_delim != nullptr) {
                    // test that ':' was in range of evr. ':' sign is only allowed in evr as an epoch deliminator
                    if (epoch_delim - evr_delim_for_nev < 1 || end - epoch_delim < 2) {
                        continue;
                    }
                    nevra.epoch.assign(evr_delim_for_nev, epoch_delim);
                    evr_delim_for_nev = epoch_delim + 1;
                }

                // test presence of version
                if (end - evr_delim_for_nev < 1) {
                    continue;
                }
                nevra.version.assign(evr_delim_for_nev);

                result.push_back(std::move(nevra));
            } break;
            case Form::NA: {
                if (arch_delim == nullptr) {
                    continue;
                }
                // test: NA cannot contain ':'
                if (epoch_delim != nullptr) {
                    continue;
                }

                // test name presence
                if (arch_delim == nevra_pattern) {
                    continue;
                }

                // test arch for absence of '-'
                if (last_delim != nullptr && last_delim > arch_delim) {
                    continue;
                }

                // test arch presence
                if (end - arch_delim == 1) {
                    continue;
                }

                nevra.name.assign(nevra_pattern, arch_delim);
                nevra.arch.assign(arch_delim + 1);

                result.push_back(std::move(nevra));
            } break;
            case Form::NAME:
                // test: Name cannot contain ':'
                if (epoch_delim != nullptr) {
                    continue;
                }

                // test name presence
                if (end == nevra_pattern) {
                    continue;
                }

                nevra.name.assign(nevra_pattern);

                result.push_back(std::move(nevra));
                break;
        }
    }
    return result;
}


void Nevra::clear() noexcept {
    name.clear();
    epoch.clear();
    version.clear();
    release.clear();
    arch.clear();
}

bool Nevra::has_just_name() const {
    return !name.empty() && epoch.empty() && version.empty() && release.empty() && arch.empty();
}


bool Nevra::operator==(const Nevra & other) const {
    return name == other.name && epoch == other.epoch && version == other.version && release == other.release &&
           arch == other.arch;
}


std::ostringstream & operator<<(std::ostringstream & out, const Nevra & nevra) {
    out << to_full_nevra_string(nevra);
    return out;
}


int rpmvercmp(const char * lhs, const char * rhs) {
    return ::rpmvercmp(lhs, rhs);
}


}  // namespace libdnf5::rpm
