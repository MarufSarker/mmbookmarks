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
#include <utility>
#include <memory>
#include "enums.hh"
#include <mm/sqlite/enums.hh>
#include <mm/sqlite/column.hh>
#include <mm/sqlite/row.hh>

namespace mm
{
namespace bookmarks
{
class comparison
{
public:
    comparison();

    comparison(similarity_type const& type,
               std::string const&     key,
               int const&             value);
    comparison(similarity_type const& type,
               std::string const&     key,
               double const&          value);
    comparison(similarity_type const& type,
               std::string const&     key,
               std::string const&     value);

    void set(similarity_type const& type,
             std::string const&     key,
             int const&             value);
    void set(similarity_type const& type,
             std::string const&     key,
             double const&          value);
    void set(similarity_type const& type,
             std::string const&     key,
             std::string const&     value);
    void set(similarity_type const&   type,
             std::string const&       key,
             std::string const&       value,
             sqlite::data_type const& data_type_);

    void append(logical_type const& type, comparison const& other);

    std::pair<std::string, sqlite::row>
        statement_and_row(unsigned int const& __internal_postfix = 0) const;


private:
    similarity_type m_type   = similarity_type::NONE;
    std::string     m_key    = {};
    sqlite::column  m_column = {};
    std::vector<std::pair<logical_type, comparison>> m_other_comparisons = {};
};
} // namespace bookmarks
} // namespace mm
