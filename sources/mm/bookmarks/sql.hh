/*
 * mmbookmarks
 * Copyright (C) 2022  Maruf Sarker
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#pragma once

#include <vector>
#include <string>

namespace mm
{
namespace bookmarks
{
namespace sql
{
namespace sqlite
{
static std::string const tables = "SELECT * FROM pragma_database_list;";


static std::string const vacuum = "VACUUM;";
} // namespace sqlite


namespace versions
{
static std::vector<std::string> const create = {
    R"EOF(
CREATE TABLE IF NOT EXISTS
mm_versions
(
    [identifier]
        INTEGER PRIMARY KEY,
    [table_name]
        TEXT UNIQUE NOT NULL,
    [version_number]
        TEXT UNIQUE NOT NULL,
    [created]
        TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
    [modified]
        TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
);
    )EOF",


    // -- current version
    R"EOF(
INSERT OR IGNORE INTO
    mm_versions
    ([table_name], [version_number])
VALUES
    ('mm_bookmarks', '1');
    )EOF",
};
}


namespace bookmarks
{
namespace helpers
{
namespace type
{
static std::string const container = "CONTAINER";
static std::string const url       = "URL";
} // namespace type


namespace defaults
{
static std::string const container = "0";
}
} // namespace helpers


static std::vector<std::string> const create = {
    R"EOF(
CREATE TABLE IF NOT EXISTS
mm_bookmarks
(
    [identifier]
        TEXT PRIMARY KEY DEFAULT
        (
            substr
            (
                ('00' || strftime('%Y%m%d%H%M%S', 'now') || hex(randomblob(8))),
                -32, 32
            )
        ),
    [container]
        TEXT NOT NULL DEFAULT '0',
    [type]
        TEXT NOT NULL,
    [url]
        TEXT UNIQUE,
    [title]
        TEXT,
    [note]
        TEXT,
    [created]
        TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
    [modified]
        TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
    FOREIGN KEY ([container])
        REFERENCES mm_bookmarks ([identifier])
        ON UPDATE CASCADE
        ON DELETE SET DEFAULT
);
    )EOF",


    // -- TRIGGER


    R"EOF(
-- [identifier]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_identifier_after_insert
AFTER INSERT ON
    mm_bookmarks
WHEN
(
    NEW.[identifier] = '0'
    OR
    NEW.[identifier] = ''
    OR
    NEW.[identifier] IS NULL
)
BEGIN
    SELECT RAISE(ABORT, '[identifier] can not be 0/empty/null');
END;
    )EOF",


    R"EOF(
-- [identifier]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_identifier_after_update
AFTER UPDATE ON
    mm_bookmarks
WHEN
    NEW.[identifier] != OLD.[identifier]
BEGIN
    SELECT RAISE(ROLLBACK, '[identifier] can not be modified');
END;
    )EOF",


    R"EOF(
-- [container]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_container_after_insert
AFTER INSERT ON
    mm_bookmarks
WHEN
(
    NEW.[container] != '0'
    AND
    (
        NEW.[identifier] == NEW.[container]
        OR
        NOT EXISTS
        (
            SELECT
                *
            FROM
                mm_bookmarks
            WHERE
                mm_bookmarks.[identifier] = NEW.[container]
                AND
                mm_bookmarks.[type] = 'CONTAINER'
        )
    )
)
BEGIN
    SELECT RAISE(ABORT, '[container] does not exists');
END;
    )EOF",


    R"EOF(
-- [container]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_container_after_update_exists
AFTER UPDATE ON
    mm_bookmarks
WHEN
(
    NEW.[container] != '0'
    AND
    (
        NEW.[identifier] == NEW.[container]
        OR
        NOT EXISTS
        (
            SELECT
                *
            FROM
                mm_bookmarks
            WHERE
                mm_bookmarks.[identifier] = NEW.[container]
                AND
                mm_bookmarks.[type] = 'CONTAINER'
        )
    )
)
BEGIN
    SELECT RAISE(ABORT, '[container] does not exists');
END;
    )EOF",


    R"EOF(
-- [container]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_container_after_update_invalid
AFTER UPDATE ON
    mm_bookmarks
WHEN
(
    EXISTS
    (
        -- a view of item containing identifier of container
        -- and all of its predecessors are being listed
        -- which will be used to check if
        -- active (/NEW) item is container of requested container or not
        WITH
            cte_parents
            (
                [identifier], [container]
            )
        AS
        (
            SELECT
                mm_bookmarks.[identifier], mm_bookmarks.[container]
            FROM
                mm_bookmarks
            WHERE
                mm_bookmarks.[identifier] = NEW.[container]

            -- UNION to avoid infinite loop
            UNION

            SELECT
                mm_bookmarks.[identifier], mm_bookmarks.[container]
            FROM
                mm_bookmarks
            JOIN
                cte_parents
            ON
                mm_bookmarks.[identifier] = cte_parents.[container]
        )
        SELECT
            -- do not use COUNT(*) along with EXISTS()
            -- COUNT() ==> 0 results EXISTS() ==> 1
            -- COUNT() ==> 1 results EXISTS() ==> 1
            -- verify
            --   SELECT EXISTS(SELECT 0); // 1
            --   SELECT EXISTS(SELECT 1); // 1
            *
        FROM
            cte_parents
        WHERE
            cte_parents.[container] == NEW.[identifier]
    )
)
BEGIN
    SELECT RAISE(ABORT, '[container] is invalid');
END;
    )EOF",


    R"EOF(
-- [container]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_container_before_delete
BEFORE DELETE ON
    mm_bookmarks
WHEN
    EXISTS
    (
        SELECT
            *
        FROM
            mm_bookmarks
        WHERE
            [container] == OLD.[identifier]
    )
BEGIN
    SELECT RAISE(ROLLBACK, '[container] is not empty');
END;
    )EOF",


    R"EOF(
-- [type]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_type_after_insert_invalid
AFTER INSERT ON
    mm_bookmarks
WHEN
    NEW.[type] != 'CONTAINER'
    AND
    NEW.[type] != 'URL'
BEGIN
    SELECT RAISE(ABORT, '[type] is invalid');
END;
    )EOF",


    R"EOF(
-- [type]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_type_after_update
AFTER UPDATE ON
    mm_bookmarks
WHEN
    NEW.[type] != OLD.[type]
BEGIN
    SELECT RAISE(ROLLBACK, '[type] can not be modified');
END;
    )EOF",


    R"EOF(
-- [created]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_created_after_insert
AFTER INSERT ON
    mm_bookmarks
WHEN
    NEW.[created] IS NULL OR NEW.[created] = ''
BEGIN
    UPDATE
        mm_bookmarks
    SET
        [created]  = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
    WHERE
        [identifier] == NEW.[identifier];
END;
    )EOF",


    R"EOF(
-- [modified]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_modified_after_insert
AFTER INSERT ON
    mm_bookmarks
WHEN
    NEW.[modified] IS NULL OR NEW.[modified] = ''
BEGIN
    UPDATE
        mm_bookmarks
    SET
        [modified] = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
    WHERE
        [identifier] == NEW.[identifier];
END;
    )EOF",


    R"EOF(
-- [modified]
CREATE TRIGGER IF NOT EXISTS
    mm_bookmarks_modified_after_update
AFTER UPDATE ON
    mm_bookmarks
BEGIN
    UPDATE
        mm_bookmarks
    SET
        [modified] = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
    WHERE
        [identifier] == NEW.[identifier];
END;
    )EOF",


    // -- reserved bookmarks
    R"EOF(
INSERT OR IGNORE INTO
    mm_bookmarks
    (
        [identifier],
        [container],
        [type],
        [title]
    )
VALUES
    ('1', '0', 'CONTAINER', 'MM Bookmarks'),
    ('2', '1', 'CONTAINER', 'Main Bookmarks'),
    ('3', '1', 'CONTAINER', 'Other Bookmarks'),
    ('4', '1', 'CONTAINER', 'Removed Bookmarks');
    )EOF",
};
} // namespace bookmarks


namespace imports
{
namespace mm_bookmarks
{
// for {0} ::
// c++17 and below:
//  use custom mechanism to replace {0} with path of external database path
// c++20 and above:
//  use std::format
static std::string const attach =
    "ATTACH DATABASE '{0}' AS attached_mm_bookmarks;";


static std::string const detach = "DETACH DATABASE attached_mm_bookmarks;";


static std::vector<std::string> const cleanup = {
    "DROP TRIGGER IF EXISTS tmp_trgr_mm_bookmarks_to_tmp_other_containers;",
    "DROP TABLE IF EXISTS tmp_other_containers;",
    "DROP TABLE IF EXISTS tmp_other_entries;",
};


static std::vector<std::string> const preparation = {
    R"EOF(
-- new urls and their containers
CREATE TABLE IF NOT EXISTS
tmp_other_entries
(
    [identifier]
        TEXT PRIMARY KEY,
    [container]
        TEXT NOT NULL DEFAULT '0',
    [type]
        TEXT NOT NULL,
    [url]
        TEXT UNIQUE,
    [title]
        TEXT,
    [note]
        TEXT,
    [created]
        TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
    [modified]
        TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
    FOREIGN KEY ([container])
    REFERENCES tmp_other_entries ([identifier])
    ON UPDATE CASCADE
    ON DELETE SET DEFAULT
);
    )EOF",


    R"EOF(
-- containers identifier holder
CREATE TABLE IF NOT EXISTS
tmp_other_containers
(
    [mm_bookmarks_identifier]
        TEXT UNIQUE NOT NULL,
    [other_identifier]
        TEXT UNIQUE NOT NULL,
    [mm_bookmarks_container]
        TEXT,
    [other_container]
        TEXT
);
    )EOF",


    R"EOF(
-- server-client mechanism to gather inserted containers identifier
-- probably have edge cases, specially in multi connection situation
CREATE TRIGGER IF NOT EXISTS
    tmp_trgr_mm_bookmarks_to_tmp_other_containers
AFTER INSERT ON
    mm_bookmarks
BEGIN
    INSERT INTO
        tmp_other_containers ([mm_bookmarks_identifier], [other_identifier])
    VALUES
        (NEW.[identifier], NEW.[note]);

    -- remove note as it was for other_identifier transfer purpose only
    UPDATE
        mm_bookmarks
    SET
        [note] = NULL
    WHERE
        [identifier] == NEW.[identifier];
END;
    )EOF",
};


static std::vector<std::string> process = {
    R"EOF(
-- insert base
INSERT INTO
    mm_bookmarks
    (
        [container],
        [type],
        [title],
        [note]
    )
VALUES
(
    '0',
    'CONTAINER',
    (
        "Imported Bookmarks [mm_bookmarks] ["
        ||
        (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
        ||
        "]"
    ),
    '-1'
);
    )EOF",


    R"EOF(
-- find new urls and their containers
WITH CTEParents AS
(
    SELECT
        attached_mm_bookmarks.mm_bookmarks.[identifier],
        attached_mm_bookmarks.mm_bookmarks.[container],
        attached_mm_bookmarks.mm_bookmarks.[type],
        attached_mm_bookmarks.mm_bookmarks.[url],
        attached_mm_bookmarks.mm_bookmarks.[title],
        attached_mm_bookmarks.mm_bookmarks.[note],
        attached_mm_bookmarks.mm_bookmarks.[created],
        attached_mm_bookmarks.mm_bookmarks.[modified]
    FROM
        attached_mm_bookmarks.mm_bookmarks
    WHERE
        (
            attached_mm_bookmarks.mm_bookmarks.[type] == 'URL'
            AND
            NOT EXISTS
            (
                SELECT
                    *
                FROM
                    main.mm_bookmarks
                WHERE
                    main.mm_bookmarks.[url] ==
attached_mm_bookmarks.mm_bookmarks.[url]
            )
        )

    -- UNION to avoid infinite loop
    UNION

    SELECT
        attached_mm_bookmarks.mm_bookmarks.[identifier],
        attached_mm_bookmarks.mm_bookmarks.[container],
        attached_mm_bookmarks.mm_bookmarks.[type],
        attached_mm_bookmarks.mm_bookmarks.[url],
        attached_mm_bookmarks.mm_bookmarks.[title],
        attached_mm_bookmarks.mm_bookmarks.[note],
        attached_mm_bookmarks.mm_bookmarks.[created],
        attached_mm_bookmarks.mm_bookmarks.[modified]
    FROM
        attached_mm_bookmarks.mm_bookmarks
    JOIN
        CTEParents
    ON
        attached_mm_bookmarks.mm_bookmarks.[identifier] = CTEParents.[container]
)
INSERT INTO
    tmp_other_entries
    (
        [identifier],
        [container],
        [type],
        [url],
        [title],
        [note],
        [created],
        [modified]
    )
SELECT
    CTEParents.[identifier],
    CTEParents.[container],
    CTEParents.[type],
    CTEParents.[url],
    CTEParents.[title],
    CTEParents.[note],
    CTEParents.[created],
    CTEParents.[modified]
FROM
    CTEParents;
    )EOF",


    R"EOF(
-- insert containers
INSERT INTO
    mm_bookmarks
    (
        [container],
        [type],
        [title],
        [note],
        [created],
        [modified]
    )
SELECT
    '0',
    'CONTAINER',
    tmp_other_entries.[title],
    tmp_other_entries.[identifier],
    tmp_other_entries.[created],
    tmp_other_entries.[modified]
FROM
    tmp_other_entries
WHERE
    tmp_other_entries.[type] == 'CONTAINER'
ORDER BY
    tmp_other_entries.[container];
    )EOF",


    R"EOF(
-- required for containers only
DROP TRIGGER IF EXISTS tmp_trgr_mm_bookmarks_to_tmp_other_containers;
    )EOF",


    R"EOF(
-- fetch containers identifier
UPDATE
    tmp_other_containers
SET
    [other_container] =
    (
        SELECT
            tmp_other_entries.[container]
        FROM
            tmp_other_entries
        WHERE
            tmp_other_entries.[identifier]
            ==
            tmp_other_containers.[other_identifier]
    );
    )EOF",


    R"EOF(
-- set base container's container
UPDATE
    tmp_other_containers
SET
    [mm_bookmarks_container] = '0'
WHERE
    [other_identifier] == '-1';
    )EOF",


    R"EOF(
-- set importing one's real base's base container
UPDATE
    tmp_other_containers
SET
    [mm_bookmarks_container] =
    (
        SELECT
            tmp_other_containers.[mm_bookmarks_identifier]
        FROM
            tmp_other_containers
        WHERE
            tmp_other_containers.[other_identifier] == '-1'
    )
WHERE
    [other_container] == '0';
    )EOF",


    R"EOF(
-- update importing one's container to appropriate ones
UPDATE
    tmp_other_containers
SET
    [mm_bookmarks_container] =
    (
        WITH
            tmp_con_view
        AS
        (
            SELECT
                *
            FROM
                tmp_other_containers
        )
        SELECT
            tmp_con_view.[mm_bookmarks_identifier]
        FROM
            tmp_con_view
        WHERE
            tmp_con_view.[other_identifier] ==
tmp_other_containers.[other_container]
    )
WHERE
    [mm_bookmarks_container] IS NULL;
    )EOF",


    R"EOF(
-- update note to actual value
-- update containers accordingly
UPDATE
    mm_bookmarks
SET
    [note] =
    (
        SELECT
            tmp_other_entries.[note]
        FROM
            tmp_other_entries
        WHERE
            tmp_other_entries.[identifier] ==
            (
                SELECT
                    tmp_other_containers.[other_identifier]
                FROM
                    tmp_other_containers
                WHERE
                    tmp_other_containers.[mm_bookmarks_identifier]
                    ==
                    main.mm_bookmarks.[identifier]
            )
    )
    ,
    [container] =
    (
        SELECT
            tmp_other_containers.[mm_bookmarks_container]
        FROM
            tmp_other_containers
        WHERE
            tmp_other_containers.[mm_bookmarks_identifier]
            ==
            main.mm_bookmarks.[identifier]
    )
WHERE
(
    mm_bookmarks.[identifier] ==
    (
        SELECT
            tmp_other_containers.[mm_bookmarks_identifier]
        FROM
            tmp_other_containers
        WHERE
        (
            mm_bookmarks.[identifier]
            ==
            tmp_other_containers.[mm_bookmarks_identifier]
        )
    )
);
    )EOF",


    R"EOF(
-- import urls
INSERT INTO
    mm_bookmarks
    (
        [container],
        [type],
        [url],
        [title],
        [note],
        [created],
        [modified]
    )
SELECT
    COALESCE
    (
        (
            SELECT
                tmp_other_containers.[mm_bookmarks_identifier]
            FROM
                tmp_other_containers
            WHERE
                tmp_other_containers.[other_identifier]
                ==
                tmp_other_entries.[container]
        ),
        (
            SELECT
                tmp_other_containers.[mm_bookmarks_identifier]
            FROM
                tmp_other_containers
            WHERE
                tmp_other_containers.[other_identifier] == '-1'
        ),
        '0'
    ),
    tmp_other_entries.[type],
    tmp_other_entries.[url],
    tmp_other_entries.[title],
    tmp_other_entries.[note],
    tmp_other_entries.[created],
    tmp_other_entries.[modified]
FROM
    tmp_other_entries
WHERE
    tmp_other_entries.[type] == 'URL'
    AND
    NOT EXISTS
    (
        SELECT
            *
        FROM
            main.mm_bookmarks
        WHERE
            main.mm_bookmarks.[url] == tmp_other_entries.[url]
    );
    )EOF",
};
} // namespace mm_bookmarks


namespace firefox_places_sqlite
{
// for {0} ::
// c++17 and below:
//  use custom mechanism to replace {0} with path of external database path
// c++20 and above:
//  use std::format
static std::string const attach =
    "ATTACH DATABASE '{0}' AS attached_firefox_database;";


static std::string const detach = "DETACH DATABASE attached_firefox_database;";


static std::vector<std::string> const cleanup = {
    "DROP TRIGGER IF EXISTS temp_firefox_acquire_identifiers;",
    "DROP TABLE IF EXISTS temp_firefox_folders;",
};


static std::vector<std::string> const preparation = {
    R"EOF(
-- containers identifier holder
CREATE TABLE IF NOT EXISTS
temp_firefox_folders
(
    [mm_bookmarks_identifier]
        TEXT UNIQUE NOT NULL,
    [moz_bookmarks_id]
        INTEGER UNIQUE NOT NULL,
    [moz_bookmarks_title]
        TEXT NOT NULL DEFAULT 'unnamed'
);
    )EOF",


    R"EOF(
-- server-client mechanism to gather inserted containers identifier
-- probably have edge cases, specially in multi connection situation
CREATE TRIGGER IF NOT EXISTS
    temp_firefox_acquire_identifiers
AFTER INSERT ON
    mm_bookmarks
BEGIN
    INSERT INTO
        temp_firefox_folders
        (
            mm_bookmarks_identifier,
            moz_bookmarks_title,
            moz_bookmarks_id
        )
    VALUES
        (
            NEW.[identifier],
            NEW.[title],
            NEW.[note]
        );

    UPDATE
        mm_bookmarks
    SET
        [note] = NULL
    WHERE
        [identifier] == NEW.[identifier];
END;
    )EOF",
};


static std::vector<std::string> process = {
    R"EOF(
-- import base container
INSERT INTO
    mm_bookmarks
    (
        [container],
        [type],
        [title],
        [note]
    )
VALUES
    (
        '0',
        'CONTAINER',
        (
            "Imported Bookmarks [FireFox] ["
            ||
            (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
            ||
            "]"
        ),
        0
    );
    )EOF",


    R"EOF(
-- all the folders
INSERT INTO
    mm_bookmarks
    (
        [container],
        [type],
        [title],
        [note],
        [created]
    )
SELECT
    '0',
    'CONTAINER',
    attached_firefox_database.moz_bookmarks.[title],
    attached_firefox_database.moz_bookmarks.[id],
    (
        SELECT
        (
            strftime
            (
                '%Y-%m-%dT%H:%M:%S+00:00',
                substr
                (
                    attached_firefox_database.moz_bookmarks.[dateAdded]
                    ||
                    0000000000,
                    1,
                    10
                ),
                'unixepoch'
            )
        )
    )
FROM
    attached_firefox_database.moz_bookmarks
WHERE
    attached_firefox_database.moz_bookmarks.[type] == 2;
    )EOF",


    R"EOF(
-- update containers with gathered mm_bookmarks_identifier
UPDATE
    mm_bookmarks
SET
    [container] = COALESCE
    (
        (
            SELECT
                temp_firefox_folders.[mm_bookmarks_identifier]
            FROM
                temp_firefox_folders
            WHERE
            (
                temp_firefox_folders.[moz_bookmarks_id] ==
                (
                    SELECT
                        attached_firefox_database.moz_bookmarks.[parent]
                    FROM
                        attached_firefox_database.moz_bookmarks
                    WHERE
                    (
                        attached_firefox_database.moz_bookmarks.[id] ==
                        (
                            SELECT
                                temp_firefox_folders.[moz_bookmarks_id]
                            FROM
                                temp_firefox_folders
                            WHERE
                            (
                                mm_bookmarks.[identifier]
                                ==
                                temp_firefox_folders.[mm_bookmarks_identifier]
                            )
                        )
                    )
                )
            )
        ),
        '0'
    )
WHERE
(
    mm_bookmarks.[identifier] ==
    (
        SELECT
            temp_firefox_folders.[mm_bookmarks_identifier]
        FROM
            temp_firefox_folders
        WHERE
        (
            mm_bookmarks.[identifier]
            ==
            temp_firefox_folders.[mm_bookmarks_identifier]
        )
    )
);
    )EOF",


    R"EOF(
-- required for containers only
DROP TRIGGER IF EXISTS temp_firefox_acquire_identifiers;
    )EOF",


    R"EOF(
-- insert urls
-- parent should match mm_bookmarks_identifier, otherwise 0
INSERT OR IGNORE INTO
    mm_bookmarks
    (
        [type],
        [container],
        [title],
        [url],
        [note],
        [created]
    )
SELECT
    'URL',
    COALESCE
    (
        (
            SELECT
                temp_firefox_folders.[mm_bookmarks_identifier]
            FROM
                temp_firefox_folders
            WHERE
            (
                attached_firefox_database.moz_bookmarks.[parent]
                ==
                temp_firefox_folders.[moz_bookmarks_id]
            )
        ),
        '0'
    ),
    attached_firefox_database.moz_bookmarks.[title],
    attached_firefox_database.moz_places.[url],
    attached_firefox_database.moz_places.[description],
    (
        SELECT
        (
            strftime
            (
                '%Y-%m-%dT%H:%M:%S+00:00',
                substr
                (
                    attached_firefox_database.moz_bookmarks.[dateAdded]
                    ||
                    0000000000,
                    1,
                    10
                ),
                'unixepoch'
            )
        )
    )
FROM
    attached_firefox_database.moz_bookmarks
LEFT JOIN
    attached_firefox_database.moz_places
ON
    (
        attached_firefox_database.moz_bookmarks.[fk]
        ==
        attached_firefox_database.moz_places.[id]
    )
    AND
    (
        attached_firefox_database.moz_bookmarks.[type]
        ==
        1
    )
WHERE
    (
        attached_firefox_database.moz_bookmarks.[type]
        ==
        1
    )
;
    )EOF",
};
} // namespace firefox_places_sqlite
} // namespace imports
} // namespace sql
} // namespace bookmarks
} // namespace mm
