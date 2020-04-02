/*
Copyright (C) 2020 Red Hat, Inc.

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


#include "libdnf/rpm/package.hpp"
#include "sack-impl.hpp"
#include "solv/package-private.hpp"



inline static std::string
cstring2string(const char * input)
{
    return input ? std::string(input) : std::string();
}


namespace libdnf::rpm {

const char *
Package::get_name_cstring() const noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_name(pool, id);
}

std::string
Package::get_name() const
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_name(pool, id));
}

const char *
Package::get_epoch_cstring()
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_epoch_cstring(pool, id);
}

unsigned long
Package::get_epoch()
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_epoch(pool, id);
}

const char *
Package::get_version_cstring() noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_version(pool, id);
}

std::string
Package::get_version()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_version(pool, id));
}

const char *
Package::get_release_cstring() noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_release(pool, id);
}

std::string
Package::get_release()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_release(pool, id));
}

const char *
Package::get_arch_cstring() const noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_arch(pool, id);
}

std::string
Package::get_arch() const
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_arch(pool, id));
}

const char *
Package::get_evr_cstring() const noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_evr(pool, id);
}

std::string
Package::get_evr() const
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_evr(pool, id));
}

std::string
Package::get_nevra()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_nevra(pool, id));
}

std::string
Package::get_group()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_group(pool, id));
}

unsigned long long
Package::get_size() noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_size(pool, id);
}

unsigned long long
Package::get_download_size() noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_download_size(pool, id);
}

unsigned long long
Package::get_install_size() noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_install_size(pool, id);
}

std::string
Package::get_license()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_license(pool, id));
}

std::string
Package::get_sourcerpm()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_sourcerpm(pool, id));
}

unsigned long long
Package::get_build_time() noexcept
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_build_time(pool, id);
}

std::string
Package::get_build_host()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_build_host(pool, id));
}

std::string
Package::get_packager()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_packager(pool, id));
}

std::string
Package::get_vendor()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_vendor(pool, id));
}

std::string
Package::get_url()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_url(pool, id));
}

std::string
Package::get_summary()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_summary(pool, id));
}

std::string
Package::get_description()
{
    Pool * pool = sack->pImpl->pool;
    return cstring2string(libdnf::rpm::solv::get_description(pool, id));
}

std::vector<std::string>
Package::get_files()
{
    Pool * pool = sack->pImpl->pool;
    return libdnf::rpm::solv::get_files(pool, id);
}

}  // namespace libdnf::rpm
