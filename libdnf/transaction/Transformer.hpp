/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef LIBDNF_TRANSACTION_TRANSFORMER_HPP
#define LIBDNF_TRANSACTION_TRANSFORMER_HPP

#include <json.h>
#include <memory>
#include <vector>

#include "libdnf/utils/sqlite3/sqlite3.hpp"

#include "CompsEnvironmentItem.hpp"
#include "CompsGroupItem.hpp"
#include "RPMItem.hpp"
#include "Transaction.hpp"
#include "private/TransformerTransaction.hpp"
#include "TransactionItem.hpp"

namespace libdnf {

/**
 * Class providing an interface to the database transformation
 */
class Transformer {
public:
    class Exception : public std::runtime_error {
    public:
        Exception(const std::string &msg)
          : runtime_error(msg)
        {
        }
        Exception(const char *msg)
          : runtime_error(msg)
        {
        }
    };

    Transformer(const std::string &inputDir, const std::string &outputFile);
    void transform();

    static void createDatabase(libdnf::utils::SQLite3Ptr conn);

    static TransactionItemReason getReason(const std::string &reason);

protected:
    void transformTrans(libdnf::utils::SQLite3Ptr swdb, libdnf::utils::SQLite3Ptr history);

    void transformGroups(libdnf::utils::SQLite3Ptr swdb);
    void processGroupPersistor(libdnf::utils::SQLite3Ptr swdb, struct json_object *root);

private:
    void transformRPMItems(libdnf::utils::SQLite3Ptr swdb,
                           libdnf::utils::SQLite3Ptr history,
                           std::shared_ptr< TransformerTransaction > trans);
    void transformOutput(libdnf::utils::SQLite3Ptr history, std::shared_ptr< TransformerTransaction > trans);
    void transformTransWith(libdnf::utils::SQLite3Ptr swdb,
                            libdnf::utils::SQLite3Ptr history,
                            std::shared_ptr< TransformerTransaction > trans);
    CompsGroupItemPtr processGroup(libdnf::utils::SQLite3Ptr swdb,
                                   const char *groupId,
                                   struct json_object *group);
    std::shared_ptr<CompsEnvironmentItem> processEnvironment(libdnf::utils::SQLite3Ptr swdb,
                                                             const char *envId,
                                                             struct json_object *env);
    std::string historyPath();
    const std::string inputDir;
    const std::string outputFile;
    const std::string transformFile;
};

} // namespace libdnf

#endif
