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


#ifndef LIBDNF_RPM_TRANSACTION_HPP
#define LIBDNF_RPM_TRANSACTION_HPP


#include "libdnf/base/base_weak.hpp"
#include "libdnf/base/transaction_package.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/package.hpp"
#include "libdnf/rpm/transaction_callbacks.hpp"

#include <rpm/rpmts.h>

#include <memory>


namespace libdnf::rpm {

class RpmProblemSet;


/// Class represents one item in transaction set.
using TransactionItem = base::TransactionPackage;


/// Class for access RPM header
class RpmHeader {
public:
    RpmHeader(const RpmHeader & src);
    RpmHeader(RpmHeader && src);
    ~RpmHeader();
    RpmHeader & operator=(const RpmHeader & src);
    RpmHeader & operator=(RpmHeader && src);

    std::string get_name() const;
    uint64_t get_epoch() const noexcept;
    std::string get_version() const;
    std::string get_release() const;
    std::string get_arch() const;
    std::string get_evr() const;
    std::string get_nevra() const;
    std::string get_full_nevra() const;
    unsigned int get_rpmdbid() const noexcept;

private:
    friend class Transaction;
    explicit RpmHeader(void * hdr);
    void * header;
};


class RpmProblem {
public:
    ~RpmProblem();

    bool operator==(RpmProblem & other) const noexcept;
    bool operator!=(RpmProblem & other) const noexcept;

    /// Return package NEVR
    std::string get_pkg_nevr() const;

    /// Return related (e.g. through a dependency) package NEVR
    std::string get_alt_nevr() const;

    /// Return type of problem (dependency, diskpace etc)
    rpmProblemType get_type() const;

    /// Return pointer to transaction item associated to the problem or nullptr.
    const TransactionItem * get_transaction_item() const;

    /// Return a generic data string from a problem
    std::string get_generic_string() const;

    /// Return disk requirement (needed disk space / number of inodes)
    /// depending on problem type. On problem types other than DISKSPACE
    /// and DISKNODES return value is undefined.
    uint64_t get_disk_need() const;

    /// Return formatted string representation of a problem.
    std::string to_string() const;

private:
    friend RpmProblemSet;
    class Impl;
    explicit RpmProblem(std::unique_ptr<Impl> && p_impl);
    std::unique_ptr<Impl> p_impl;
};


class RpmProblemSet {
public:
    class Iterator {
    public:
        // iterator traits
        using difference_type = int;
        using value_type = RpmProblem;
        using pointer = const RpmProblem *;
        using reference = const RpmProblem &;
        using iterator_category = std::forward_iterator_tag;

        Iterator();
        ~Iterator();

        Iterator & operator++();
        // TODO(jrohel): postfix operator ++, Bad iterator support in the librpm
        // iterator operator++(int);
        bool operator==(Iterator & other) const;
        bool operator!=(Iterator other) const { return !(*this == other); }
        RpmProblem operator*();

    private:
        friend RpmProblemSet;
        class Impl;
        explicit Iterator(std::unique_ptr<Impl> && p_impl);
        std::unique_ptr<Impl> p_impl;
    };

    Iterator begin();
    Iterator end();

    ~RpmProblemSet();

    /// Return number of problems in set.
    /// @return  number of problems
    int size() noexcept;

    bool empty() noexcept { return size() == 0; }

private:
    friend class Transaction;
    class Impl;
    explicit RpmProblemSet(std::unique_ptr<Impl> && p_impl);
    std::unique_ptr<Impl> p_impl;
};


class TransactionError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::rpm"; }
    const char * get_name() const noexcept override { return "TransactionError"; }
};


class Transaction {
public:
    explicit Transaction(Base & base);
    explicit Transaction(const BaseWeakPtr & base);
    Transaction(const Transaction &) = delete;
    Transaction(Transaction &&) = delete;
    ~Transaction();

    Transaction & operator=(const Transaction &) = delete;
    Transaction & operator=(Transaction &&) = delete;

    /// Set transaction root directory, i.e. path to chroot(2).
    /// @param root_dir  new transaction root directory (or NULL)
    /// @return  0 on success, -1 on error (invalid root directory)
    void set_root_dir(const char * root_dir);

    /// Get transaction root directory, i.e. path to chroot(2).
    /// @return  transaction root directory
    const char * get_root_dir() const;

    /// Get transaction flags, i.e. bits that control rpmtsRun().
    /// @return  transaction flags
    rpmtransFlags get_flags() const;

    /// Set transaction flags, i.e. bits that control rpmtsRun().
    /// @param flags  new transaction flags
    /// @return  previous transaction flags
    rpmtransFlags set_flags(rpmtransFlags flags);

    /// Get verify signatures flag(s).
    /// @return  verify signatures flags
    rpmVSFlags get_signature_verify_flags() const;

    /// Set verify signatures flag(s).
    /// @param verify_flags  new verify signatures flags
    /// @return         previous value
    rpmVSFlags set_signature_verify_flags(rpmVSFlags verify_flags);

    /// Get package verify flag(s).
    /// @return  verify signatures flags
    rpmVSFlags get_pkg_verify_flags() const;

    /// Set package verify flag(s).
    /// @param verify_flags  new package verify flags
    /// @return  old package verify flags
    rpmVSFlags set_pkg_verify_flags(rpmVSFlags verify_flags);

    /// Get enforced package verify level
    /// @return  package verify level
    int get_pkg_verify_level() const;

    /// Set enforced package verify level
    /// @param verify_level  new package verify level
    /// @return  old package verify level
    int set_pkg_verify_level(int verify_level);

    /// Retrieve rpm database cookie.
    /// Useful for eg. determining cache validity.
    /// @return rpm database cookie
    std::string get_db_cookie() const;

    /// Get transaction id, i.e. transaction time stamp.
    /// @return  transaction id
    rpm_tid_t get_id() const;

    /// Set transaction callbacks.
    /// @param callbacks a `unique_ptr` containing the callbacks object
    void set_callbacks(std::unique_ptr<TransactionCallbacks> && callbacks);

    /// Fill the RPM transaction from base::Transaction.
    /// @param transcation The base::Transaction object.
    void fill(const base::Transaction & transaction);

    /// Perform a dependency check on the transaction set.
    /// After headers have been added to a transaction set,
    /// a dependency check can be performed to make sure that all package dependencies are satisfied.
    /// Any found problems can be examined by retrieving the problem set with rpmtsProblems().
    /// @return  true on dependencies are ok
    bool check();

    /// Process all package elements in a transaction set.
    /// Before calling run() be sure to have:
    ///
    ///    - setup the rpm root dir via set_root_dir().
    ///    - setup the rpm notify callback via register_cb().
    ///    - setup the rpm transaction flags via set_flags().
    ///
    /// Additionally, though not required you may want to:
    ///
    ///    - setup the rpm verify signature flags via set_signature_verify_flags().
    ///
    /// @return		0 on success, -1 on error, >0 with newProbs set
    int run();

    /// Get current transaction set problems.
    RpmProblemSet get_problems();

    /// Set transaction script file descriptor, i.e. stdout/stderr on scriptlet execution.
    /// The file descriptor is copied using dup().
    /// @param fd  new script file descriptor (or -1)
    void set_script_out_fd(int fd);

    /// Set transaction script file, i.e. stdout/stderr on scriptlet execution.
    /// The set_script_fd() can be used to pass file handle.
    /// @param file_path  new file path
    void set_script_out_file(const std::string & file_path);

    /// @return A `Base` object to which the transaction belongs.
    /// @since 5.0
    BaseWeakPtr get_base() const;

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_TRANSACTION_HPP
