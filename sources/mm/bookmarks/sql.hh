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

#include <array>

namespace mm
{
namespace bookmarks
{
namespace sql
{
namespace sqlite
{
constexpr static char const* tables_v3_16 = R"EOF(
    -- requires sqlite 3.16+
    -- PRAGMA database_list
    --  is equivalent to
    SELECT * FROM pragma_database_list;
    -- https://www.sqlite.org/pragma.html#pragfunc
)EOF";


constexpr static char const* vacuum = R"EOF(
    VACUUM;
)EOF";
} // namespace sqlite


namespace versions
{
constexpr static char const* create = R"EOF(
    -- mm_versions create
    CREATE TABLE IF NOT EXISTS
        mm_versions
        (
            [identifier]
                INTEGER PRIMARY KEY,
            [table_name]
                TEXT UNIQUE NOT NULL,
            [version_number]
                INT UNIQUE NOT NULL,
            [created]
                TEXT NOT NULL
                DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
            [modified]
                TEXT NOT NULL
                DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
        )
    ;
)EOF";


constexpr static std::array<char const*, 2> triggers = {
    R"EOF(
    -- mm_versions update modified
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_versions_general_time_records_after_update
    AFTER UPDATE ON
        mm_versions
    BEGIN
        UPDATE
            mm_versions
        SET
            [modified] = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
        WHERE
            [identifier] == NEW.[identifier];
    END
    ;
    )EOF",


    R"EOF(
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_versions_general_time_records_after_insert
    AFTER INSERT ON
        mm_versions
    BEGIN
        UPDATE
            mm_versions
        SET
            [created]  = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
            [modified] = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
        WHERE
            [identifier] == NEW.[identifier];
    END
    ;
    )EOF",
};

/*
constexpr static char const* table_exists = R"EOF(
    -- mm_versions exists
    SELECT
        COUNT(*)
    FROM
        sqlite_master
    WHERE
        [type] = 'table'
        AND
        [name] = 'mm_versions'
        AND
        [tbl_name] = 'mm_versions'
    ;
)EOF";
*/

constexpr static char const* insert_version = R"EOF(
    -- insert current version into mm_versions
    INSERT OR IGNORE INTO
        mm_versions
        (
            [table_name],
            [version_number]
        )
    VALUES
    (
        'mm_bookmarks',
        1
    );
)EOF";

/*
constexpr static char const* select_versions = R"EOF(
    -- get current version from mm_versions
    SELECT
        *
    FROM
        mm_versions
    ;
)EOF";
*/
} // namespace versions


namespace bookmarks
{
namespace helpers
{
constexpr static int type_container = 0;
constexpr static int type_url       = 1;

constexpr static int default_container = 0;
} // namespace helpers


constexpr static char const* create = R"EOF(
    -- mm_bookmarks create table
    -- [type] : 0 - container, 1 - url
    CREATE TABLE IF NOT EXISTS
        mm_bookmarks
        (
            [identifier]
                INTEGER PRIMARY KEY,
            [container]
                INTEGER NOT NULL DEFAULT 0,
            [type]
                INT NOT NULL,
            [url]
                TEXT UNIQUE,
            [title]
                TEXT,
            [note]
                TEXT,
            [created]
                TEXT NOT NULL
                DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
            [modified]
                TEXT NOT NULL
                DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
            FOREIGN KEY ([container])
                REFERENCES
                    mm_bookmarks
                    (
                        [identifier]
                    )
                ON UPDATE
                    CASCADE
                ON DELETE
                    SET DEFAULT
        )
    ;
)EOF";


constexpr static std::array<char const*, 9> triggers = {
    R"EOF(
    -- mm_bookmarks trigger for modified
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_general_time_records_after_update
    AFTER UPDATE ON
        mm_bookmarks
    BEGIN
        UPDATE
            mm_bookmarks
        SET
            [modified] = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
        WHERE
            [identifier] == NEW.[identifier];
    END
    ;
    )EOF",


    R"EOF(
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_general_time_records_after_insert
    AFTER INSERT ON
        mm_bookmarks
    BEGIN
        UPDATE
            mm_bookmarks
        SET
            [created]  = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
            [modified] = (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
        WHERE
            [identifier] == NEW.[identifier];
    END
    ;
    )EOF",


    /**
    R"EOF(
    -- mm_bookmarks trigger to prevent overriding of identifier
    -- from documentation
    --   The value of NEW.rowid is undefined in a BEFORE INSERT trigger
    --   in which the rowid is not explicitly set to an integer.
    -- mm_bookmarks sets
    --   [identifier] INTEGER PRIMARY KEY
    -- so it should not be an issue
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_identifier_overridability_before_insert
    BEFORE INSERT ON
        mm_bookmarks
    WHEN
        NEW.[identifier] IN (SELECT DISTINCT [identifier] FROM mm_bookmarks)
    BEGIN
        SELECT RAISE(ABORT, "[identifier] can not be overridden");
    END
    ;
    )EOF",
    **/


    /**
    R"EOF(
    -- mm_bookmarks trigger to prevent update of identifier
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_identifier_immutability_after_update
    AFTER UPDATE ON
        mm_bookmarks
    WHEN
        NEW.[identifier] != OLD.[identifier]
    BEGIN
        SELECT RAISE(ROLLBACK, "[identifier] can not be modified");
    END
    ;
    )EOF",
    **/


    R"EOF(
    -- mm_bookmarks trigger to prevent update of type
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_type_immutability_after_update
    AFTER UPDATE ON
        mm_bookmarks
    WHEN
        NEW.[type] != OLD.[type]
    BEGIN
        SELECT RAISE(ROLLBACK, "[type] can not be modified");
    END
    ;
    )EOF",


    R"EOF(
    -- mm_bookmarks trigger to prevent undesired type
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_type_validity_before_insert
    BEFORE INSERT ON
        mm_bookmarks
    WHEN
        -- CAST AS TEXT,
        --   as it seems 0/1 probably is being interpreted
        --   as false/true in terms of C language
        -- CAST to make it free from table definition
        CAST(NEW.[type] AS TEXT) != '0'
        AND
        CAST(NEW.[type] AS TEXT) != '1'
    BEGIN
        SELECT RAISE(ABORT, "[type] is invalid");
    END
    ;
    )EOF",


    R"EOF(
    -- mm_bookmarks trigger for identifier-container similarity check
    -- identifier and container can not be same
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_identifier_container_similarity_check_after_insert
    AFTER INSERT ON
        mm_bookmarks
    WHEN
    (
        NEW.[identifier] == NEW.[container]
        AND
        -- does not apply for 0/root container
        NEW.[identifier] != 0
    )
    BEGIN
        UPDATE
            mm_bookmarks
        SET
            [container] = 0
        WHERE
        (
            [identifier] == NEW.[identifier]
            AND
            NEW.[identifier] == NEW.[container]
            AND
            -- does not apply for 0/root container
            NEW.[identifier] != 0
        );
    END
    ;
    )EOF",


    R"EOF(
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_identifier_container_similarity_check_after_update
    AFTER UPDATE ON
        mm_bookmarks
    WHEN
    (
        NEW.[identifier] == NEW.[container]
        AND
        -- does not apply for 0/root container
        NEW.[identifier] != 0
    )
    BEGIN
        UPDATE
            mm_bookmarks
        SET
            [container] = 0
        WHERE
        (
            [identifier] == NEW.[identifier]
            AND
            NEW.[identifier] == NEW.[container]
            AND
            -- does not apply for 0/root container
            NEW.[identifier] != 0
        );
    END
    ;
    )EOF",


    /**
    R"EOF(
    -- mm_bookmarks trigger for container existence
    -- parent must preexists as a container (unless root containers)
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_container_existence_check_after_insert
    AFTER INSERT ON
        mm_bookmarks
    WHEN
    (
        NEW.[container] NOT IN (SELECT DISTINCT [identifier] FROM
        mm_bookmarks) AND
        -- does not apply for 0/root container
        NEW.[identifier] != 0
        OR
        -- container has to be of type 0 (container)
        NOT EXISTS
        (
            SELECT
                *
            FROM
                mm_bookmarks
            WHERE
                mm_bookmarks.[identifier] == NEW.[container]
                AND
                mm_bookmarks.[type] == 0
        )
    )
    BEGIN
        UPDATE
            mm_bookmarks
        SET
            -- 0/root as default container
            [container] = 0
        WHERE
        (
            [identifier] == NEW.[identifier]
            AND
            -- does not apply for 0/root container
            NEW.[identifier] != 0
        );
    END
    ;
    )EOF",
    **/


    /**
    R"EOF(
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_container_existence_check_after_update
    AFTER UPDATE ON
        mm_bookmarks
    WHEN
    (
        NEW.[container] NOT IN (SELECT DISTINCT [identifier] FROM
        mm_bookmarks) AND
        -- does not apply for 0/root container
        NEW.[identifier] != 0
        OR
        -- container has to be of type 0 (container)
        NOT EXISTS
        (
            SELECT
                *
            FROM
                mm_bookmarks
            WHERE
                mm_bookmarks.[identifier] == NEW.[container]
                AND
                mm_bookmarks.[type] == 0
        )
    )
    BEGIN
        UPDATE
            mm_bookmarks
        SET
            -- 0/root as default container
            [container] = 0
        WHERE
        (
            [identifier] == NEW.[identifier]
            AND
            -- does not apply for 0/root container
            NEW.[identifier] != 0
        );
    END
    ;
    )EOF",
    **/


    R"EOF(
    -- mm_bookmarks trigger container-identifier relationship check
    -- container can not be child of identifier
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_container_identifier_relationship_check_before_insert
    -- AFTER INSERT is not working for this one
    -- for triggers AFTER is recommended than BEFORE by official document
    -- so, try to see if this can be migrated to AFTER INSERT or not
    BEFORE INSERT ON
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
                    [identifier],
                    [container]
                )
            AS
            (
                SELECT
                    [identifier], [container]
                FROM
                    mm_bookmarks
                WHERE
                    [identifier] = NEW.[container]

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
        UPDATE
            mm_bookmarks
        SET
            [container] = 0
        WHERE
            [identifier] == NEW.[identifier];
    END
    ;
    )EOF",


    R"EOF(
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_container_identifier_relationship_check_after_update
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
                    [identifier],
                    [container]
                )
            AS
            (
                SELECT
                    [identifier], [container]
                FROM
                    mm_bookmarks
                WHERE
                    [identifier] = NEW.[container]

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
        UPDATE
            mm_bookmarks
        SET
            [container] = 0
        WHERE
            [identifier] == NEW.[identifier];
    END
    ;
    )EOF",


    R"EOF(
    -- mm_bookmarks trigger to prevent removing identifier below 256
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_prevent_reserve_remove_before_delete
    BEFORE DELETE ON
        mm_bookmarks
    WHEN
        OLD.[identifier] < 256
    BEGIN
        SELECT RAISE(ROLLBACK, "[identifier] below 256 can not be deleted");
    END
    ;
    )EOF",


    /*R"EOF(
    -- mm_bookmarks trigger to prevent updating identifier below 256
    CREATE TRIGGER IF NOT EXISTS
        trigger_mm_bookmarks_prevent_reserve_update_before_update
    BEFORE UPDATE ON
        mm_bookmarks
    WHEN
        OLD.[identifier] < 256
    BEGIN
        SELECT RAISE(ROLLBACK, "[identifier] below 256 can not be updated");
    END
    ;
    )EOF",*/
};


constexpr static char const* primary_containers_count = R"EOF(
    SELECT
        COUNT(*)
    FROM
        mm_bookmarks
    WHERE
        [identifier] < 256
        AND
        [type] == 0
    ;
)EOF";


constexpr static char const* insert_primary_containers = R"EOF(
    -- mm_bookmarks primary/reserved containers
    INSERT INTO
        mm_bookmarks
        (
            [identifier],
            [container],
            [type],
            [url],
            [title]
        )
    VALUES
        -- primary
        (0, 0, 0, NULL, 'MM Bookmarks'),
        (1, 0, 0, NULL, 'Main Bookmarks'),
        (2, 0, 0, NULL, 'Other Bookmarks'),
        (3, 0, 0, NULL, 'Removed Bookmarks'),
        -- reserved
        (4, 0, 0, NULL, '[reserved]'),
        (5, 0, 0, NULL, '[reserved]'),
        (6, 0, 0, NULL, '[reserved]'),
        (7, 0, 0, NULL, '[reserved]'),
        (8, 0, 0, NULL, '[reserved]'),
        (9, 0, 0, NULL, '[reserved]'),
        (10, 0, 0, NULL, '[reserved]'),
        (11, 0, 0, NULL, '[reserved]'),
        (12, 0, 0, NULL, '[reserved]'),
        (13, 0, 0, NULL, '[reserved]'),
        (14, 0, 0, NULL, '[reserved]'),
        (15, 0, 0, NULL, '[reserved]'),
        (16, 0, 0, NULL, '[reserved]'),
        (17, 0, 0, NULL, '[reserved]'),
        (18, 0, 0, NULL, '[reserved]'),
        (19, 0, 0, NULL, '[reserved]'),
        (20, 0, 0, NULL, '[reserved]'),
        (21, 0, 0, NULL, '[reserved]'),
        (22, 0, 0, NULL, '[reserved]'),
        (23, 0, 0, NULL, '[reserved]'),
        (24, 0, 0, NULL, '[reserved]'),
        (25, 0, 0, NULL, '[reserved]'),
        (26, 0, 0, NULL, '[reserved]'),
        (27, 0, 0, NULL, '[reserved]'),
        (28, 0, 0, NULL, '[reserved]'),
        (29, 0, 0, NULL, '[reserved]'),
        (30, 0, 0, NULL, '[reserved]'),
        (31, 0, 0, NULL, '[reserved]'),
        (32, 0, 0, NULL, '[reserved]'),
        (33, 0, 0, NULL, '[reserved]'),
        (34, 0, 0, NULL, '[reserved]'),
        (35, 0, 0, NULL, '[reserved]'),
        (36, 0, 0, NULL, '[reserved]'),
        (37, 0, 0, NULL, '[reserved]'),
        (38, 0, 0, NULL, '[reserved]'),
        (39, 0, 0, NULL, '[reserved]'),
        (40, 0, 0, NULL, '[reserved]'),
        (41, 0, 0, NULL, '[reserved]'),
        (42, 0, 0, NULL, '[reserved]'),
        (43, 0, 0, NULL, '[reserved]'),
        (44, 0, 0, NULL, '[reserved]'),
        (45, 0, 0, NULL, '[reserved]'),
        (46, 0, 0, NULL, '[reserved]'),
        (47, 0, 0, NULL, '[reserved]'),
        (48, 0, 0, NULL, '[reserved]'),
        (49, 0, 0, NULL, '[reserved]'),
        (50, 0, 0, NULL, '[reserved]'),
        (51, 0, 0, NULL, '[reserved]'),
        (52, 0, 0, NULL, '[reserved]'),
        (53, 0, 0, NULL, '[reserved]'),
        (54, 0, 0, NULL, '[reserved]'),
        (55, 0, 0, NULL, '[reserved]'),
        (56, 0, 0, NULL, '[reserved]'),
        (57, 0, 0, NULL, '[reserved]'),
        (58, 0, 0, NULL, '[reserved]'),
        (59, 0, 0, NULL, '[reserved]'),
        (60, 0, 0, NULL, '[reserved]'),
        (61, 0, 0, NULL, '[reserved]'),
        (62, 0, 0, NULL, '[reserved]'),
        (63, 0, 0, NULL, '[reserved]'),
        (64, 0, 0, NULL, '[reserved]'),
        (65, 0, 0, NULL, '[reserved]'),
        (66, 0, 0, NULL, '[reserved]'),
        (67, 0, 0, NULL, '[reserved]'),
        (68, 0, 0, NULL, '[reserved]'),
        (69, 0, 0, NULL, '[reserved]'),
        (70, 0, 0, NULL, '[reserved]'),
        (71, 0, 0, NULL, '[reserved]'),
        (72, 0, 0, NULL, '[reserved]'),
        (73, 0, 0, NULL, '[reserved]'),
        (74, 0, 0, NULL, '[reserved]'),
        (75, 0, 0, NULL, '[reserved]'),
        (76, 0, 0, NULL, '[reserved]'),
        (77, 0, 0, NULL, '[reserved]'),
        (78, 0, 0, NULL, '[reserved]'),
        (79, 0, 0, NULL, '[reserved]'),
        (80, 0, 0, NULL, '[reserved]'),
        (81, 0, 0, NULL, '[reserved]'),
        (82, 0, 0, NULL, '[reserved]'),
        (83, 0, 0, NULL, '[reserved]'),
        (84, 0, 0, NULL, '[reserved]'),
        (85, 0, 0, NULL, '[reserved]'),
        (86, 0, 0, NULL, '[reserved]'),
        (87, 0, 0, NULL, '[reserved]'),
        (88, 0, 0, NULL, '[reserved]'),
        (89, 0, 0, NULL, '[reserved]'),
        (90, 0, 0, NULL, '[reserved]'),
        (91, 0, 0, NULL, '[reserved]'),
        (92, 0, 0, NULL, '[reserved]'),
        (93, 0, 0, NULL, '[reserved]'),
        (94, 0, 0, NULL, '[reserved]'),
        (95, 0, 0, NULL, '[reserved]'),
        (96, 0, 0, NULL, '[reserved]'),
        (97, 0, 0, NULL, '[reserved]'),
        (98, 0, 0, NULL, '[reserved]'),
        (99, 0, 0, NULL, '[reserved]'),
        (100, 0, 0, NULL, '[reserved]'),
        (101, 0, 0, NULL, '[reserved]'),
        (102, 0, 0, NULL, '[reserved]'),
        (103, 0, 0, NULL, '[reserved]'),
        (104, 0, 0, NULL, '[reserved]'),
        (105, 0, 0, NULL, '[reserved]'),
        (106, 0, 0, NULL, '[reserved]'),
        (107, 0, 0, NULL, '[reserved]'),
        (108, 0, 0, NULL, '[reserved]'),
        (109, 0, 0, NULL, '[reserved]'),
        (110, 0, 0, NULL, '[reserved]'),
        (111, 0, 0, NULL, '[reserved]'),
        (112, 0, 0, NULL, '[reserved]'),
        (113, 0, 0, NULL, '[reserved]'),
        (114, 0, 0, NULL, '[reserved]'),
        (115, 0, 0, NULL, '[reserved]'),
        (116, 0, 0, NULL, '[reserved]'),
        (117, 0, 0, NULL, '[reserved]'),
        (118, 0, 0, NULL, '[reserved]'),
        (119, 0, 0, NULL, '[reserved]'),
        (120, 0, 0, NULL, '[reserved]'),
        (121, 0, 0, NULL, '[reserved]'),
        (122, 0, 0, NULL, '[reserved]'),
        (123, 0, 0, NULL, '[reserved]'),
        (124, 0, 0, NULL, '[reserved]'),
        (125, 0, 0, NULL, '[reserved]'),
        (126, 0, 0, NULL, '[reserved]'),
        (127, 0, 0, NULL, '[reserved]'),
        (128, 0, 0, NULL, '[reserved]'),
        (129, 0, 0, NULL, '[reserved]'),
        (130, 0, 0, NULL, '[reserved]'),
        (131, 0, 0, NULL, '[reserved]'),
        (132, 0, 0, NULL, '[reserved]'),
        (133, 0, 0, NULL, '[reserved]'),
        (134, 0, 0, NULL, '[reserved]'),
        (135, 0, 0, NULL, '[reserved]'),
        (136, 0, 0, NULL, '[reserved]'),
        (137, 0, 0, NULL, '[reserved]'),
        (138, 0, 0, NULL, '[reserved]'),
        (139, 0, 0, NULL, '[reserved]'),
        (140, 0, 0, NULL, '[reserved]'),
        (141, 0, 0, NULL, '[reserved]'),
        (142, 0, 0, NULL, '[reserved]'),
        (143, 0, 0, NULL, '[reserved]'),
        (144, 0, 0, NULL, '[reserved]'),
        (145, 0, 0, NULL, '[reserved]'),
        (146, 0, 0, NULL, '[reserved]'),
        (147, 0, 0, NULL, '[reserved]'),
        (148, 0, 0, NULL, '[reserved]'),
        (149, 0, 0, NULL, '[reserved]'),
        (150, 0, 0, NULL, '[reserved]'),
        (151, 0, 0, NULL, '[reserved]'),
        (152, 0, 0, NULL, '[reserved]'),
        (153, 0, 0, NULL, '[reserved]'),
        (154, 0, 0, NULL, '[reserved]'),
        (155, 0, 0, NULL, '[reserved]'),
        (156, 0, 0, NULL, '[reserved]'),
        (157, 0, 0, NULL, '[reserved]'),
        (158, 0, 0, NULL, '[reserved]'),
        (159, 0, 0, NULL, '[reserved]'),
        (160, 0, 0, NULL, '[reserved]'),
        (161, 0, 0, NULL, '[reserved]'),
        (162, 0, 0, NULL, '[reserved]'),
        (163, 0, 0, NULL, '[reserved]'),
        (164, 0, 0, NULL, '[reserved]'),
        (165, 0, 0, NULL, '[reserved]'),
        (166, 0, 0, NULL, '[reserved]'),
        (167, 0, 0, NULL, '[reserved]'),
        (168, 0, 0, NULL, '[reserved]'),
        (169, 0, 0, NULL, '[reserved]'),
        (170, 0, 0, NULL, '[reserved]'),
        (171, 0, 0, NULL, '[reserved]'),
        (172, 0, 0, NULL, '[reserved]'),
        (173, 0, 0, NULL, '[reserved]'),
        (174, 0, 0, NULL, '[reserved]'),
        (175, 0, 0, NULL, '[reserved]'),
        (176, 0, 0, NULL, '[reserved]'),
        (177, 0, 0, NULL, '[reserved]'),
        (178, 0, 0, NULL, '[reserved]'),
        (179, 0, 0, NULL, '[reserved]'),
        (180, 0, 0, NULL, '[reserved]'),
        (181, 0, 0, NULL, '[reserved]'),
        (182, 0, 0, NULL, '[reserved]'),
        (183, 0, 0, NULL, '[reserved]'),
        (184, 0, 0, NULL, '[reserved]'),
        (185, 0, 0, NULL, '[reserved]'),
        (186, 0, 0, NULL, '[reserved]'),
        (187, 0, 0, NULL, '[reserved]'),
        (188, 0, 0, NULL, '[reserved]'),
        (189, 0, 0, NULL, '[reserved]'),
        (190, 0, 0, NULL, '[reserved]'),
        (191, 0, 0, NULL, '[reserved]'),
        (192, 0, 0, NULL, '[reserved]'),
        (193, 0, 0, NULL, '[reserved]'),
        (194, 0, 0, NULL, '[reserved]'),
        (195, 0, 0, NULL, '[reserved]'),
        (196, 0, 0, NULL, '[reserved]'),
        (197, 0, 0, NULL, '[reserved]'),
        (198, 0, 0, NULL, '[reserved]'),
        (199, 0, 0, NULL, '[reserved]'),
        (200, 0, 0, NULL, '[reserved]'),
        (201, 0, 0, NULL, '[reserved]'),
        (202, 0, 0, NULL, '[reserved]'),
        (203, 0, 0, NULL, '[reserved]'),
        (204, 0, 0, NULL, '[reserved]'),
        (205, 0, 0, NULL, '[reserved]'),
        (206, 0, 0, NULL, '[reserved]'),
        (207, 0, 0, NULL, '[reserved]'),
        (208, 0, 0, NULL, '[reserved]'),
        (209, 0, 0, NULL, '[reserved]'),
        (210, 0, 0, NULL, '[reserved]'),
        (211, 0, 0, NULL, '[reserved]'),
        (212, 0, 0, NULL, '[reserved]'),
        (213, 0, 0, NULL, '[reserved]'),
        (214, 0, 0, NULL, '[reserved]'),
        (215, 0, 0, NULL, '[reserved]'),
        (216, 0, 0, NULL, '[reserved]'),
        (217, 0, 0, NULL, '[reserved]'),
        (218, 0, 0, NULL, '[reserved]'),
        (219, 0, 0, NULL, '[reserved]'),
        (220, 0, 0, NULL, '[reserved]'),
        (221, 0, 0, NULL, '[reserved]'),
        (222, 0, 0, NULL, '[reserved]'),
        (223, 0, 0, NULL, '[reserved]'),
        (224, 0, 0, NULL, '[reserved]'),
        (225, 0, 0, NULL, '[reserved]'),
        (226, 0, 0, NULL, '[reserved]'),
        (227, 0, 0, NULL, '[reserved]'),
        (228, 0, 0, NULL, '[reserved]'),
        (229, 0, 0, NULL, '[reserved]'),
        (230, 0, 0, NULL, '[reserved]'),
        (231, 0, 0, NULL, '[reserved]'),
        (232, 0, 0, NULL, '[reserved]'),
        (233, 0, 0, NULL, '[reserved]'),
        (234, 0, 0, NULL, '[reserved]'),
        (235, 0, 0, NULL, '[reserved]'),
        (236, 0, 0, NULL, '[reserved]'),
        (237, 0, 0, NULL, '[reserved]'),
        (238, 0, 0, NULL, '[reserved]'),
        (239, 0, 0, NULL, '[reserved]'),
        (240, 0, 0, NULL, '[reserved]'),
        (241, 0, 0, NULL, '[reserved]'),
        (242, 0, 0, NULL, '[reserved]'),
        (243, 0, 0, NULL, '[reserved]'),
        (244, 0, 0, NULL, '[reserved]'),
        (245, 0, 0, NULL, '[reserved]'),
        (246, 0, 0, NULL, '[reserved]'),
        (247, 0, 0, NULL, '[reserved]'),
        (248, 0, 0, NULL, '[reserved]'),
        (249, 0, 0, NULL, '[reserved]'),
        (250, 0, 0, NULL, '[reserved]'),
        (251, 0, 0, NULL, '[reserved]'),
        (252, 0, 0, NULL, '[reserved]'),
        (253, 0, 0, NULL, '[reserved]'),
        (254, 0, 0, NULL, '[reserved]'),
        (255, 0, 0, NULL, '[reserved]')
    ;
)EOF";

/*
constexpr static char const* insert_template = R"EOF(
    -- mm_bookmarks INSERT template
    INSERT INTO
        mm_bookmarks
        (
            [container],
            [type],
            [url],
            [title],
            [note]
        )
    VALUES
    (
        :CONTAINER,
        :TYPE,
        :URL,
        :TITLE,
        :NOTE
    );
)EOF";
*/
/*
constexpr static char const* update_template = R"EOF(
    -- mm_bookmarks UPDATE template
    -- on update FOREIGN KEY constrain will take effect
    -- if parent container is updated, container of all children items will
    be cascaded UPDATE
        mm_bookmarks
    SET
        [container] = :CONTAINER,
        [url] = :URL,
        [title] = :TITLE,
        [note] = :NOTE
    WHERE
        [identifier] == :IDENTIFIER
    ;
)EOF";
*/
/*
constexpr static char const* update_template_2 = R"EOF(
    UPDATE
        mm_bookmarks
    SET
        [container] = :NEWCONTAINER,
        [url] = :NEWURL,
        [title] = :NEWTITLE,
        [note] = :NEWNOTE
    WHERE
        [identifier] == :OLDIDENTIFIER
    ;
)EOF";
*/
/*
constexpr static char const* select_all = R"EOF(
    -- mm_bookmarks select all bookmarks template
    SELECT
        *
    FROM
        mm_bookmarks
    ;
)EOF";
*/
/*
constexpr static char const* select_based_on_identifier = R"EOF(
    -- mm_bookmarks select specific bookmark template
    SELECT
        *
    FROM
        mm_bookmarks
    WHERE
        [identifier] == :IDENTIFIER
    ;
)EOF";
*/
/*
constexpr static char const* select_based_on_type = R"EOF(
    -- mm_bookmarks select specific bookmark template
    SELECT
        *
    FROM
        mm_bookmarks
    WHERE
        [type] == :TYPE
    ;
)EOF";
*/
/*
constexpr static char const* select_based_on_container = R"EOF(
    -- mm_bookmarks select specific bookmark template
    SELECT
        *
    FROM
        mm_bookmarks
    WHERE
        [container] == :CONTAINER
    ;
)EOF";
*/
/*
constexpr static char const*
    select_based_on_container_with_idetifier_pagination_and_limit = R"EOF(
    SELECT
        *
    FROM
        mm_bookmarks
    WHERE
    (
        [container] == :CONTAINER
        AND
        [identifier] > :PAGINATIONSTARTIDENTIFIER
    )
    ORDER BY
        [identifier]
    LIMIT
        :AMOUNT;
)EOF";
*/
/*
constexpr static char const* select_based_on_container_with_limit_and_offset =
    R"EOF(
    SELECT
        *
    FROM
        mm_bookmarks
    WHERE
    (
        [container] == :CONTAINER
    )
    ORDER BY
        [title]
    LIMIT
        :PLIMIT
    OFFSET
        :POFFSET
    ;
)EOF";
*/
/*
constexpr static char const* delete_based_on_identifier = R"EOF(
    -- mm_bookmarks delete bookmark template
    -- on delete FOREIGN KEY constrain will take effect
    -- if parent container is deleted, container of all children items will
    be set to DEFAULT (0)

    DELETE FROM
        mm_bookmarks
    WHERE
        [identifier] == :IDENTIFIER
    ;
)EOF";
*/
} // namespace bookmarks


namespace transfer
{
namespace mmbookmarks
{
namespace import
{
// for {0} ::
// c++17 and below:
//  use custom mechanism to replace {0} with path
//  of external database path
// c++20 and above:
//  use std::format
constexpr static char const* attach = R"EOF(
    -- insert from another table
    -- attach external db
    ATTACH DATABASE
        '{0}'
    AS
        tmp_other_db
    ;
)EOF";


constexpr static char const* detach = R"EOF(
    DETACH DATABASE
        tmp_other_db
    ;
)EOF";


constexpr static std::array<char const*, 3> preparation = {
    R"EOF(
    -- new urls and their containers
    CREATE TABLE IF NOT EXISTS tmp_other_entries
    (
        [identifier]
            INTEGER PRIMARY KEY,
        [container]
            INTEGER NOT NULL
            DEFAULT 0,
        [type]
            INT NOT NULL,
        [url]
            TEXT UNIQUE,
        [title]
            TEXT,
        [note]
            TEXT,
        [created]
            TEXT NOT NULL
            DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
        [modified]
            TEXT NOT NULL
            DEFAULT (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now')),
        FOREIGN KEY
            (
                [container]
            )
        REFERENCES
            tmp_other_entries
            (
                [identifier]
            )
        ON UPDATE
            CASCADE
        ON DELETE
            SET DEFAULT
    )
    ;
    )EOF",


    R"EOF(
    -- containers identifier holder
    CREATE TABLE IF NOT EXISTS
        tmp_other_containers
        (
            [mm_bookmarks_identifier]
                INTEGER UNIQUE NOT NULL,
            [other_identifier]
                INTEGER UNIQUE NOT NULL,
            [mm_bookmarks_container]
                INTEGER,
            [other_container]
                INTEGER
        )
    ;
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
            tmp_other_containers
            (
                [mm_bookmarks_identifier],
                [other_identifier]
            )
        VALUES
        (
            NEW.[identifier],
            NEW.[note]
        );

        -- remove note as it was for other_identifier transfer purpose only
        UPDATE
            mm_bookmarks
        SET
            [note] = NULL
        WHERE
            [identifier] == NEW.[identifier]
        ;
    END;
    )EOF",
};


constexpr static std::array<char const*, 3> cleanup = {
    R"EOF(
    DROP TRIGGER IF EXISTS
        tmp_trgr_mm_bookmarks_to_tmp_other_containers
    ;
    )EOF",


    R"EOF(
    DROP TABLE IF EXISTS
        tmp_other_containers
    ;
    )EOF",


    R"EOF(
    DROP TABLE IF EXISTS
        tmp_other_entries
    ;
    )EOF",
};


constexpr static std::array<char const*, 10> process = {
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
        0,
        0,
        (
            "Imported Bookmarks [mm_bookmarks] ["
            || (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
            || "]"
        ),
        -1
    )
    ;
    )EOF",


    R"EOF(
    -- find new urls and their containers
    WITH CTEParents AS
    (
            SELECT
                tmp_other_db.mm_bookmarks.[identifier],
                tmp_other_db.mm_bookmarks.[container],
                tmp_other_db.mm_bookmarks.[type],
                tmp_other_db.mm_bookmarks.[url],
                tmp_other_db.mm_bookmarks.[title],
                tmp_other_db.mm_bookmarks.[note],
                tmp_other_db.mm_bookmarks.[created],
                tmp_other_db.mm_bookmarks.[modified]
            FROM
                tmp_other_db.mm_bookmarks
            WHERE
                (
                    tmp_other_db.mm_bookmarks.[type] == 1
                    AND
                    NOT EXISTS
                    (
                        SELECT
                            *
                        FROM
                            main.mm_bookmarks
                        WHERE
                            main.mm_bookmarks.[url] ==
tmp_other_db.mm_bookmarks.[url]
                    )
                )
        -- UNION to avoid infinite loop
        UNION
            SELECT
                tmp_other_db.mm_bookmarks.[identifier],
                tmp_other_db.mm_bookmarks.[container],
                tmp_other_db.mm_bookmarks.[type],
                tmp_other_db.mm_bookmarks.[url],
                tmp_other_db.mm_bookmarks.[title],
                tmp_other_db.mm_bookmarks.[note],
                tmp_other_db.mm_bookmarks.[created],
                tmp_other_db.mm_bookmarks.[modified]
            FROM
                tmp_other_db.mm_bookmarks
            JOIN
                CTEParents
            ON
                tmp_other_db.mm_bookmarks.[identifier] = CTEParents.[container]
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
        CTEParents
    ;
    )EOF",


    R"EOF(
    -- insert containers
    INSERT INTO
        mm_bookmarks
        (
            [container],
            [type],
            [title],
            [note]
        )
    SELECT
        0,
        0,
        tmp_other_entries.[title],
        tmp_other_entries.[identifier]
    FROM
        tmp_other_entries
    WHERE
        tmp_other_entries.[type] == 0
    ORDER BY
        tmp_other_entries.[container]
    ;
    )EOF",


    R"EOF(
    -- required for containers only
    DROP TRIGGER IF EXISTS
        tmp_trgr_mm_bookmarks_to_tmp_other_containers
    ;
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
        )
    ;
    )EOF",


    R"EOF(
    -- set base container's container
    UPDATE
        tmp_other_containers
    SET
        [mm_bookmarks_container] = 0
    WHERE
        [other_identifier] == -1
    ;
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
                tmp_other_containers.[other_identifier] == -1
        )
    WHERE
        [other_container] == 0
    ;
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
        [mm_bookmarks_container] IS NULL
    ;
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
    )
    ;
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
            [note]
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
                    tmp_other_containers.[other_identifier] == -1
            ),
            0
        ),
        tmp_other_entries.[type],
        tmp_other_entries.[url],
        tmp_other_entries.[title],
        tmp_other_entries.[note]
    FROM
        tmp_other_entries
    WHERE
        tmp_other_entries.[type] == 1
        AND
        NOT EXISTS
        (
            SELECT
                *
            FROM
                main.mm_bookmarks
            WHERE
                main.mm_bookmarks.[url] == tmp_other_entries.[url]
        )
    ;
    )EOF",
};
} // namespace import
} // namespace mmbookmarks


namespace firefox_places_sqlite
{
namespace import
{
// for {0} ::
// c++17 and below:
//  use custom mechanism to replace {0} with path
//  of external database path
// c++20 and above:
//  use std::format
constexpr static char const* attach = R"EOF(
    ATTACH DATABASE
        '{0}'
    AS
        temp_database_firefox
    ;
)EOF";


constexpr static char const* detach = R"EOF(
    DETACH DATABASE
        temp_database_firefox
    ;
)EOF";


constexpr static std::array<char const*, 2> preparation = {
    R"EOF(
    -- containers identifier holder
    CREATE TABLE IF NOT EXISTS
        temp_table_firefox_folders
        (
            [mm_bookmarks_identifier]
                INTEGER UNIQUE NOT NULL,
            [moz_bookmarks_id]
                INTEGER UNIQUE NOT NULL,
            [moz_bookmarks_title]
                TEXT UNIQUE NOT NULL
                DEFAULT 'unnamed'
        )
    ;
    )EOF",


    R"EOF(
    -- server-client mechanism to gather inserted containers identifier
    -- probably have edge cases, specially in multi connection situation
    CREATE TRIGGER IF NOT EXISTS
        temp_trigger_mm_bookmarks_to_temp_table_firefox_folders
    AFTER INSERT ON
        mm_bookmarks
    BEGIN
        INSERT INTO
            temp_table_firefox_folders
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


constexpr static std::array<char const*, 2> cleanup = {
    R"EOF(
    DROP TRIGGER IF EXISTS
        temp_trigger_mm_bookmarks_to_temp_table_firefox_folders
    ;
    )EOF",


    R"EOF(
    DROP TABLE IF EXISTS
        temp_table_firefox_folders
    ;
    )EOF",
};


constexpr static std::array<char const*, 5> process = {
    R"EOF(
    -- base import container
    INSERT INTO
        mm_bookmarks
        (
            [container],
            [type],
            [title],
            [note]
        )
    VALUES (
        0,
        0,
        (
            "Imported Bookmarks [FireFox] ["
            || (strftime('%Y-%m-%dT%H:%M:%S+00:00', 'now'))
            || "]"
        ),
        0
    )
    ;
    )EOF",


    R"EOF(
    -- all the folders/containers
    INSERT INTO
        mm_bookmarks
        (
            [container],
            [type],
            [title],
            [note]
        )
    SELECT
        0,
        0,
        temp_database_firefox.moz_bookmarks.[title],
        temp_database_firefox.moz_bookmarks.[id]
    FROM
        temp_database_firefox.moz_bookmarks
    WHERE
        temp_database_firefox.moz_bookmarks.[type] == 2
    ;
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
                    temp_table_firefox_folders.[mm_bookmarks_identifier]
                FROM
                    temp_table_firefox_folders
                WHERE
                (
                    temp_table_firefox_folders.[moz_bookmarks_id] ==
                    (
                        SELECT
                            temp_database_firefox.moz_bookmarks.[parent]
                        FROM
                            temp_database_firefox.moz_bookmarks
                        WHERE
                        (
                            temp_database_firefox.moz_bookmarks.[id] ==
                            (
                                SELECT
                                    temp_table_firefox_folders.[moz_bookmarks_id]
                                FROM
                                    temp_table_firefox_folders
                                WHERE
                                (
                                    mm_bookmarks.[identifier]
                                    ==
                                    temp_table_firefox_folders.[mm_bookmarks_identifier]
                                )
                            )
                        )
                    )
                )
            ),
            0
        )
    WHERE
    (
        mm_bookmarks.[identifier] ==
        (
            SELECT
                temp_table_firefox_folders.[mm_bookmarks_identifier]
            FROM
                temp_table_firefox_folders
            WHERE
            (
                mm_bookmarks.[identifier]
                ==
                temp_table_firefox_folders.[mm_bookmarks_identifier]
            )
        )
    )
    ;
    )EOF",


    R"EOF(
    -- required for containers only
    DROP TRIGGER IF EXISTS
        temp_trigger_mm_bookmarks_to_temp_table_firefox_folders
    ;
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
            [note]
        )
    SELECT
        1,
        COALESCE
        (
            (
                SELECT
                    temp_table_firefox_folders.[mm_bookmarks_identifier]
                FROM
                    temp_table_firefox_folders
                WHERE
                (
                    temp_database_firefox.moz_bookmarks.[parent]
                    ==
                    temp_table_firefox_folders.[moz_bookmarks_id]
                )
            ),
            0
        ),
        temp_database_firefox.moz_bookmarks.[title],
        temp_database_firefox.moz_places.[url],
        temp_database_firefox.moz_places.[description]
    FROM
        temp_database_firefox.moz_bookmarks
    LEFT JOIN
        temp_database_firefox.moz_places
    ON
        (
            temp_database_firefox.moz_bookmarks.[fk]
            ==
            temp_database_firefox.moz_places.[id]
        )
        AND
        (
            temp_database_firefox.moz_bookmarks.[type]
            ==
            1
        )
    WHERE
        (
            temp_database_firefox.moz_bookmarks.[type]
            ==
            1
        )
    ;
    )EOF",
};
} // namespace import
} // namespace firefox_places_sqlite
} // namespace transfer
} // namespace sql
} // namespace bookmarks
} // namespace mm
