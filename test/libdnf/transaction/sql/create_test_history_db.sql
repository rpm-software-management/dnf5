R"**(
    CREATE TABLE pkgtups (
        pkgtupid INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        arch TEXT NOT NULL,
        epoch TEXT NOT NULL,
        version TEXT NOT NULL,
        release TEXT NOT NULL,
        checksum TEXT
    );
    CREATE TABLE trans_beg (
        tid INTEGER PRIMARY KEY,
        timestamp INTEGER NOT NULL,
        rpmdb_version TEXT NOT NULL,
        loginuid INTEGER
    );
    CREATE TABLE trans_end (
        tid INTEGER PRIMARY KEY REFERENCES trans_beg,
        timestamp INTEGER NOT NULL,
        rpmdb_version TEXT NOT NULL,
        return_code INTEGER NOT NULL
    );
    CREATE TABLE trans_cmdline (
        tid INTEGER NOT NULL REFERENCES trans_beg,
        cmdline TEXT NOT NULL
    );
    CREATE TABLE trans_data_pkgs (
        tid INTEGER NOT NULL REFERENCES trans_beg,
        pkgtupid INTEGER NOT NULL REFERENCES pkgtups,
        done BOOL NOT NULL DEFAULT FALSE, state TEXT NOT NULL
    );
    CREATE TABLE trans_script_stdout (
        lid INTEGER PRIMARY KEY,
        tid INTEGER NOT NULL REFERENCES trans_beg,
        line TEXT NOT NULL
    );
    CREATE TABLE pkg_yumdb (
        pkgtupid INTEGER NOT NULL REFERENCES pkgtups,
        yumdb_key TEXT NOT NULL,
        yumdb_val TEXT NOT NULL
    );
    CREATE TABLE trans_with_pkgs (
        tid INTEGER NOT NULL REFERENCES trans_beg,
        pkgtupid INTEGER NOT NULL REFERENCES pkgtups
    );
    CREATE TABLE trans_error (
        mid INTEGER PRIMARY KEY,
        tid INTEGER NOT NULL REFERENCES trans_beg,
        msg TEXT NOT NULL
    );

    /* Initialize the history database */

    INSERT INTO
        pkgtups
    VALUES
        (1, 'chrony', 'x86_64', 1, '3.1',   '4.fc26',   'sha256:6cec2091'),
        (2, 'kernel', 'x86_64', 0, '4.11',  '301.fc26', 'sha256:8dc6bb96'),
        (3, 'chrony', 'x86_64', 1, '3.2',   '4.fc26',   'sha256:6asd1231');

    INSERT INTO
        pkg_yumdb
    VALUES
        (1, 'releasever',   '26'),
        (1, 'reason',       'user'),
        (2, 'releasever',   'rawhide'),
        (2, 'reason',       'dep'),
        (3, 'releasever',   '26'),
        (3, 'reason',       'user');

    INSERT INTO
        trans_beg
    VALUES
        (1, 1513267401, '2213:9795b6a4db5e5368628b5240ec63a629833c5594', 1000),
        (2, 1513267535, '2213:9eab991133c166f8bcf3ecea9fb422b853f7aebc', 1000);

    INSERT INTO
        trans_end
    VALUES
        (1, 1513267509, '2213:9eab991133c166f8bcf3ecea9fb422b853f7aebc', 0),
        (2, 1513267539, '2214:e02004142740afb5b6d148d50bc84be4ab41ad13', 0);

    INSERT INTO
        trans_cmdline
    VALUES
        (1, 'upgrade -y'),
        (2, '-y install Foo');

    INSERT INTO
        trans_script_stdout
    VALUES
        (1, 1, 'line1'),
        (2, 1, 'line2');

    INSERT INTO
        trans_error
    VALUES
        (1, 2, 'msg1'),
        (2, 2, 'msg2');

    INSERT INTO
        trans_data_pkgs
    VALUES
        (1, 3, 'TRUE', 'Update'),
        (1, 1, 'TRUE', 'Updated'),
        (2, 2, 'TRUE', 'Install');

    INSERT INTO
        trans_with_pkgs
    VALUES
        (1,1),
        (2,1),
        (2,2);
)**"
