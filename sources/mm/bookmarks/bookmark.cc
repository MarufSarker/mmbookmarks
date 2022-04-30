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


#include "bookmark.hh"
#include "utilities.hh"
#include <mm/sqlite/utilities.hh>
#include <stdexcept>

namespace mm
{
namespace bookmarks
{
bookmark::bookmark() = default;


bookmark::~bookmark() = default;


bookmark::bookmark(sqlite::row const& row_)
{
    for (auto v : row_.columns())
    {
        if (v.first == "identifier")
            identifier = v.second.value();
        else if (v.first == "container")
            container = v.second.value();
        else if (v.first == "type")
            type = v.second.value();
        else if (v.first == "url")
            url = v.second.value();
        else if (v.first == "title")
            title = v.second.value();
        else if (v.first == "note")
            note = v.second.value();
        else if (v.first == "created")
            created = v.second.value();
        else if (v.first == "modified")
            modified = v.second.value();
    }
}


sqlite::row bookmark::to_row(bool const&        assign_parameters,
                             std::string const& postfix) const
{
    sqlite::row result {};

    static auto _col = [&](std::string const&       name_,
                           std::string const&       value_,
                           sqlite::data_type const& type_,
                           bool const&              allow_empty = false)
    {
        if (!allow_empty && value_.empty())
            return;
        result.append(
            name_,
            sqlite::column {
                value_,
                type_,
                (assign_parameters ? form_parameter(name_, postfix) : "")});
    };

    _col("identifier", identifier, sqlite::data_type::TEXT);
    _col("container", container, sqlite::data_type::TEXT);
    _col("type", type, sqlite::data_type::TEXT);
    _col("url", url, sqlite::data_type::TEXT, true);
    _col("title", title, sqlite::data_type::TEXT, true);
    _col("note", note, sqlite::data_type::TEXT, true);
    _col("created", created, sqlite::data_type::TEXT);
    _col("modified", modified, sqlite::data_type::TEXT);

    return result;
}


void bookmark::valid_key(std::string const& key)
{
    if (!(key == "identifier" || key == "container" || key == "type" ||
          key == "url" || key == "title" || key == "note" || key == "created" ||
          key == "modified"))
        throw std::runtime_error {"Invalid key for bookmark."};
}
} // namespace bookmarks
} // namespace mm
