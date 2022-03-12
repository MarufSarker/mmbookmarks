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
#include <mm/sqlite/row.hh>

namespace mm
{
namespace bookmarks
{
class bookmark
{
public:
    std::string identifier = {};
    std::string container  = {};
    std::string type       = {};
    std::string url        = {};
    std::string title      = {};
    std::string note       = {};
    std::string created    = {};
    std::string modified   = {};

    bookmark();
    ~bookmark();

    bookmark(sqlite::row const& row_);

    sqlite::row to_row(bool const&        assign_parameters = false,
                       std::string const& postfix           = "") const;

    static void valid_key(std::string const& key);
};
} // namespace bookmarks
} // namespace mm
