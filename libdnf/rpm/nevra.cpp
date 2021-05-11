/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/rpm/nevra.hpp"


namespace libdnf::rpm {


static const std::vector<Nevra::Form> PKG_SPEC_FORMS{
    Nevra::Form::NEVRA, Nevra::Form::NA, Nevra::Form::NAME, Nevra::Form::NEVR, Nevra::Form::NEV};

const std::vector<Nevra::Form> & Nevra::get_default_pkg_spec_forms() {
    return PKG_SPEC_FORMS;
}

std::vector<Nevra> Nevra::parse(const std::string & nevra_str, const std::vector<Form> & forms) {
    std::vector<Nevra> result;
    const char * evr_delim = nullptr;
    const char * epoch_delim = nullptr;
    const char * release_delim = nullptr;
    const char * arch_delim = nullptr;
    const char * end;

    // parse nevra
    const char * nevra_pattern = nevra_str.c_str();
    for (end = nevra_pattern; *end != '\0'; ++end) {
        if (*end == '-') {
            evr_delim = release_delim;
            release_delim = end;
        } else if (*end == '.') {
            arch_delim = end;
        } else if (*end == ':') {
            // ':' can be only once in nevra
            if (epoch_delim != nullptr) {
                return result;
            }
            epoch_delim = end;
        } else if (*end == '(' || *end == '/' || *end == '=' || *end == '<' || *end == '>' || *end == ' ') {
            return result;
        }
    }
    for (auto form : forms) {
        Nevra nevra;
        switch (form) {
            case Form::NEVRA: {
                if (evr_delim == nullptr || release_delim == nullptr || arch_delim == nullptr) {
                    continue;
                }

                // test name presence
                if (evr_delim == nevra_pattern) {
                    continue;
                }

                const char * evr_delim_for_nevra = evr_delim;
                const char * release_delim_for_nevra = release_delim;
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
                if (evr_delim == nullptr || release_delim == nullptr) {
                    continue;
                }

                // test name presence
                if (evr_delim == nevra_pattern) {
                    continue;
                }

                const char * evr_delim_for_nevr = evr_delim;
                const char * release_delim_for_nevr = release_delim;

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
                if (evr_delim == nullptr) {
                    continue;
                }

                // test name presence
                if (evr_delim == nevra_pattern) {
                    continue;
                }

                const char * evr_delim_for_nev = release_delim == nullptr ? evr_delim : release_delim;

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

                const char * arch_delim_for_nev = arch_delim;

                nevra.name.assign(nevra_pattern, arch_delim_for_nev);
                ++arch_delim_for_nev;

                // test arch for absence of '-'
                if (evr_delim != nullptr) {
                    const char * evr_delim_for_na = release_delim == nullptr ? evr_delim : release_delim;
                    if (arch_delim_for_nev < evr_delim_for_na) {
                        continue;
                    }
                }

                // test arch presence
                if (end - arch_delim_for_nev < 1) {
                    continue;
                }
                nevra.arch.assign(arch_delim_for_nev);

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


}  // namespace libdnf::rpm
