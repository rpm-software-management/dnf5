R"**(
    CREATE TABLE trans (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        dt_begin INTEGER NOT NULL,      /* (unix timestamp) date and time of transaction begin */
        dt_end INTEGER,                 /* (unix timestamp) date and time of transaction end */
        rpmdb_version_begin TEXT,
        rpmdb_version_end TEXT,
        releasever TEXT NOT NULL,       /* var: $releasever */
        user_id INTEGER NOT NULL,       /* user ID (UID) */
        cmdline TEXT,                   /* recorded command line (program, options, arguments) */
        state INTEGER NOT NULL          /* (enum) */
    );
    CREATE TABLE repo (
        id INTEGER PRIMARY KEY,
        repoid TEXT NOT NULL            /* repository ID aka 'repoid' */
    );
    CREATE TABLE console_output (
        id INTEGER PRIMARY KEY,
        trans_id INTEGER REFERENCES trans(id),
        file_descriptor INTEGER NOT NULL,       /* stdout: 1, stderr : 2 */
        line TEXT NOT NULL
    );
    CREATE TABLE item (
        id INTEGER PRIMARY KEY,
        item_type INTEGER NOT NULL              /* (enum) 1: rpm, 2: group, 3: env ...*/
    );
    CREATE TABLE trans_item (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        trans_id INTEGER REFERENCES trans(id),
        item_id INTEGER REFERENCES item(id),
        repo_id INTEGER REFERENCES repo(id),
        action INTEGER NOT NULL,                                /* (enum) */
        reason INTEGER NOT NULL,                                /* (enum) */
        state INTEGER NOT NULL                                  /* (enum) */
    );
    CREATE TABLE item_replaced_by (              /* M:N relationship between transaction items */
        trans_item_id INTEGER REFERENCES trans_item(id),
        by_trans_item_id INTEGER REFERENCES trans_item(id),
        PRIMARY KEY (trans_item_id, by_trans_item_id)
    );
    CREATE TABLE trans_with (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        trans_id INTEGER REFERENCES trans(id),
        item_id INTEGER REFERENCES item(id),
        CONSTRAINT trans_with_unique_trans_item UNIQUE (trans_id, item_id)
    );

    /* item: rpm */
    CREATE TABLE rpm (
        item_id INTEGER UNIQUE NOT NULL,
        name TEXT NOT NULL,
        epoch INTEGER NOT NULL,                 /* empty epoch is stored as 0 */
        version TEXT NOT NULL,
        release TEXT NOT NULL,
        arch TEXT NOT NULL,
        FOREIGN KEY(item_id) REFERENCES item(id),
        CONSTRAINT rpm_unique_nevra UNIQUE (name, epoch, version, release, arch)
    );

    /* item: comps-group */
    CREATE TABLE comps_group (
        item_id INTEGER UNIQUE NOT NULL,
        groupid TEXT NOT NULL,
        name TEXT NOT NULL,
        translated_name TEXT NOT NULL,
        pkg_types INTEGER NOT NULL,
        FOREIGN KEY(item_id) REFERENCES item(id)
    );
    CREATE TABLE comps_group_package (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        group_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        installed INTEGER NOT NULL,
        pkg_type INTEGER NOT NULL,
        FOREIGN KEY(group_id) REFERENCES comps_group(item_id),
        CONSTRAINT comps_group_package_unique_name UNIQUE (group_id, name)
    );

    /* item: comps-environment */
    CREATE TABLE comps_environment (
        item_id INTEGER UNIQUE NOT NULL,
        environmentid TEXT NOT NULL,
        name TEXT NOT NULL,
        translated_name TEXT NOT NULL,
        pkg_types INTEGER NOT NULL,
        FOREIGN KEY(item_id) REFERENCES item(id)
    );
    CREATE TABLE comps_environment_group (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        environment_id INTEGER NOT NULL,
        groupid TEXT NOT NULL,
        installed INTEGER NOT NULL,
        group_type INTEGER NOT NULL,
        FOREIGN KEY(environment_id) REFERENCES comps_environment(item_id),
        CONSTRAINT comps_environment_group_unique_groupid UNIQUE (environment_id, groupid)
    );

    CREATE INDEX rpm_name ON rpm(name);
    CREATE INDEX trans_item_trans_id ON trans_item(trans_id);
    CREATE INDEX trans_item_item_id ON trans_item(item_id);

    CREATE TABLE config (
        key TEXT PRIMARY KEY,
        value TEXT NOT NULL
    );
    INSERT INTO config VALUES (
        'version',
        '1.1'
    );
)**"
