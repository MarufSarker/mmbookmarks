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

#include <string>
#include <vector>
#include "bookmark.hh"
#include "comparison.hh"
#include <mm/sqlite/database.hh>

namespace mm
{
namespace bookmarks
{
class manager
{
public:
    manager();
    ~manager();

    manager(std::string const& directory);
    manager(std::string const& directory, std::string const& filename);

    void open(std::string const& directory);
    void open(std::string const& directory, std::string const& filename);
    void close();
    bool opened() const;

    void prepare_databases();
    void vacuum_databases();

    void insert_bookmarks(std::vector<bookmark> const& bookmarks);
    void update_bookmarks(std::vector<bookmark> const& bookmarks);
    void delete_bookmarks(std::vector<std::string> const& identifiers);
    std::vector<bookmark> select_bookmarks(
        comparison const&                                comparison_,
        std::vector<std::pair<std::string, bool>> const& order_by_and_asc,
        unsigned int const&                              limit,
        unsigned int const&                              offset);
    size_t count_bookmarks(comparison const& comparison_);

    void import_from(source_type const& type, std::string const& path);

    void logging(bool const& enable);
    bool logging() const;


private:
    constexpr static char const* m_default_filename = "mm_bookmarks.db";

    std::string      m_filepath = {};
    sqlite::database m_database = {};
};
} // namespace bookmarks
} // namespace mm
