R"**(
    BEGIN TRANSACTION;

    CREATE TABLE "trans_state" (
        "id" INTEGER,
        "name" TEXT,
        PRIMARY KEY("id")
    );
    INSERT INTO "trans_state" VALUES (1, 'Started'), (2, 'Ok'), (3, 'Error');

    CREATE TABLE "trans" (
        "id" INTEGER,
        "dt_begin" INTEGER NOT NULL,                      /* (unix timestamp) date and time of transaction begin */
        "dt_end" INTEGER,                                 /* (unix timestamp) date and time of transaction end */
        "rpmdb_version_begin" TEXT,
        "rpmdb_version_end" TEXT,
        "releasever" TEXT NOT NULL,                       /* var: $releasever */
        "user_id" INTEGER NOT NULL,                       /* user ID (UID) */
        "description" TEXT,                               /* A description of the transaction (e.g. the CLI command being executed) */
        "comment" TEXT,                                   /* An arbitrary comment */
        "state_id" INTEGER,                               /* (enum) */
        PRIMARY KEY("id" AUTOINCREMENT),
	FOREIGN KEY("state_id") REFERENCES "trans_state"("id")
    );

    CREATE TABLE "repo" (
        "id" INTEGER,
        "repoid" TEXT NOT NULL,            /* repository ID aka 'repoid' */
        PRIMARY KEY("id")
    );

    CREATE TABLE "pkg_names" (
        "id" INTEGER,
        "name" TEXT NOT NULL UNIQUE,
        PRIMARY KEY("id")
    );

    CREATE TABLE "archs" (
        "id" INTEGER,
        "name" TEXT NOT NULL UNIQUE,
        PRIMARY KEY("id")
    );

    CREATE TABLE "console_output" (
        "id" INTEGER,
        "trans_id" INTEGER,
        "file_descriptor" INTEGER NOT NULL,       /* stdout: 1, stderr : 2 */
        "line" TEXT NOT NULL,
        PRIMARY KEY("id"),
        FOREIGN KEY("trans_id") REFERENCES "trans"("id")
    );

    CREATE TABLE "item" (
        "id" INTEGER,
        PRIMARY KEY("id")
    );

    CREATE TABLE "trans_item_action" (
        "id" INTEGER,
        "name" TEXT,
        PRIMARY KEY("id")
    );
    INSERT INTO "trans_item_action" VALUES (1, 'Install'), (2, 'Upgrade'), (3, 'Downgrade'), (4, 'Reinstall'), (5, 'Remove'), (6, 'Replaced'), (7, 'Reason Change');

    CREATE TABLE "trans_item_reason" (
        "id" INTEGER,
        "name" TEXT,
        PRIMARY KEY("id")
    );
    INSERT INTO "trans_item_reason" VALUES (0, 'None'), (1, 'Dependency'), (2, 'User'), (3, 'Clean'), (4, 'Weak Dependency'), (5, 'Group'), (6, 'External User');

    CREATE TABLE "trans_item_state" (
        "id" INTEGER,
        "name" TEXT,
        PRIMARY KEY("id")
    );
    INSERT INTO "trans_item_state" VALUES (1, 'Started'), (2, 'Ok'), (3, 'Error');

    CREATE TABLE "trans_item" (
        "id" INTEGER,
        "trans_id" INTEGER,
        "item_id" INTEGER,
        "repo_id" INTEGER,
        "action_id" INTEGER NOT NULL,    /* (enum) */
        "reason_id" INTEGER NOT NULL,    /* (enum) */
        "state_id" INTEGER NOT NULL,     /* (enum) */
        PRIMARY KEY("id" AUTOINCREMENT),
        FOREIGN KEY("trans_id") REFERENCES "trans"("id"),
        FOREIGN KEY("item_id") REFERENCES "item"("id"),
        FOREIGN KEY("repo_id") REFERENCES "repo"("id"),
        FOREIGN KEY("action_id") REFERENCES "trans_item_action"("id"),
        FOREIGN KEY("reason_id") REFERENCES "trans_item_reason"("id"),
        FOREIGN KEY("state_id") REFERENCES "trans_item_state"("id")
    );

    CREATE TABLE "item_replaced_by" (              /* M:N relationship between transaction items */
        "trans_item_id" INTEGER,
        "by_trans_item_id" INTEGER,
        PRIMARY KEY ("trans_item_id", "by_trans_item_id"),
        FOREIGN KEY("trans_item_id") REFERENCES "trans_item"("id"),
        FOREIGN KEY("by_trans_item_id") REFERENCES "trans_item"("id")
    );

    /* item: rpm */
    CREATE TABLE "rpm" (
        "item_id" INTEGER NOT NULL UNIQUE,
        "name_id" INTEGER NOT NULL,
        "epoch" INTEGER NOT NULL,                 /* empty epoch is stored as 0 */
        "version" TEXT NOT NULL,
        "release" TEXT NOT NULL,
        "arch_id" INTEGER NOT NULL,
        FOREIGN KEY("item_id") REFERENCES "item"("id"),
        FOREIGN KEY("name_id") REFERENCES "pkg_names"("id"),
        FOREIGN KEY("arch_id") REFERENCES "archs"("id"),
        CONSTRAINT "rpm_unique_nevra" UNIQUE ("name_id", "epoch", "version", "release", "arch_id")
    );

    /* item: comps-group */
    CREATE TABLE "comps_group" (
        "item_id" INTEGER NOT NULL UNIQUE,
        "groupid" TEXT NOT NULL,
        "name" TEXT NOT NULL,
        "translated_name" TEXT NOT NULL,
        "pkg_types" INTEGER NOT NULL,
        FOREIGN KEY("item_id") REFERENCES "item"("id")
    );

    CREATE TABLE "comps_group_package" (
        "id" INTEGER,
        "group_id" INTEGER NOT NULL,
        "name_id" INTEGER NOT NULL,
        "installed" INTEGER NOT NULL,
        "pkg_type" INTEGER NOT NULL,
        FOREIGN KEY("group_id") REFERENCES "comps_group"("item_id"),
        FOREIGN KEY("name_id") REFERENCES "pkg_names"("id"),
        CONSTRAINT "comps_group_package_unique_name" UNIQUE ("group_id", "name_id"),
        PRIMARY KEY("id" AUTOINCREMENT)
    );

    /* item: comps-environment */
    CREATE TABLE "comps_environment" (
        "item_id" INTEGER NOT NULL UNIQUE,
        "environmentid" TEXT NOT NULL,
        "name" TEXT NOT NULL,
        "translated_name" TEXT NOT NULL,
        "pkg_types" INTEGER NOT NULL,
        FOREIGN KEY("item_id") REFERENCES "item"("id")
    );

    CREATE TABLE "comps_environment_group" (
        "id" INTEGER,
        "environment_id" INTEGER NOT NULL,
        "groupid" TEXT NOT NULL,
        "installed" INTEGER NOT NULL,
        "group_type" INTEGER NOT NULL,
        FOREIGN KEY("environment_id") REFERENCES "comps_environment"("item_id"),
        CONSTRAINT "comps_environment_group_unique_groupid" UNIQUE ("environment_id", "groupid"),
        PRIMARY KEY("id" AUTOINCREMENT)
    );

    CREATE TABLE "config" (
        "key" TEXT,
        "value" TEXT NOT NULL,
        PRIMARY KEY("key")
    );
    INSERT INTO "config" VALUES ('version', '1.1');

    DELETE FROM "sqlite_sequence";

    CREATE INDEX "pkg_name" ON "pkg_names"("name");
    CREATE INDEX "trans_item_trans_id" ON "trans_item"("trans_id");
    CREATE INDEX "trans_item_item_id" ON "trans_item"("item_id");

    COMMIT;
)**"
