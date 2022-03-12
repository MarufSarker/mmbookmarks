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


#include "comparison.hh"
#include "utilities.hh"
#include <mm/sqlite/utilities.hh>
#include <stdexcept>

namespace mm
{
namespace bookmarks
{
comparison::comparison() = default;


comparison::comparison(similarity_type const& type,
                       std::string const&     key,
                       int const&             value)
{
    set(type, key, value);
}


comparison::comparison(similarity_type const& type,
                       std::string const&     key,
                       double const&          value)
{
    set(type, key, value);
}


comparison::comparison(similarity_type const& type,
                       std::string const&     key,
                       std::string const&     value)
{
    set(type, key, value);
}


void comparison::set(similarity_type const& type,
                     std::string const&     key,
                     int const&             value)
{
    set(type, key, std::to_string(value), sqlite::data_type::INTEGER);
}


void comparison::set(similarity_type const& type,
                     std::string const&     key,
                     double const&          value)
{
    set(type, key, std::to_string(value), sqlite::data_type::REAL);
}


void comparison::set(similarity_type const& type,
                     std::string const&     key,
                     std::string const&     value)
{
    set(type, key, value, sqlite::data_type::TEXT);
}


void comparison::set(similarity_type const&   type,
                     std::string const&       key,
                     std::string const&       value,
                     sqlite::data_type const& data_type_)
{
    sqlite::valid_sqlite_identifier(key);

    m_type   = type;
    m_key    = key;
    m_column = sqlite::column {value, data_type_};
}


void comparison::append(logical_type const& type, comparison const& other)
{
    m_other_comparisons.push_back(
        std::pair<logical_type, comparison> {type, other});
}


std::pair<std::string, sqlite::row>
    comparison::statement_and_row(unsigned int const& __internal_postfix) const
{
    unsigned int internal_postfix = __internal_postfix;

    std::pair<std::string, sqlite::row> result {};

    std::string       value = m_column.value();
    sqlite::data_type type  = m_column.type();

    if (m_type == similarity_type::LIKE)
    {
        value = "%" + value + "%";
        type  = sqlite::data_type::TEXT;
    }

    sqlite::column col {
        value, type, form_parameter(m_key, std::to_string(internal_postfix))};

    result.first =
        "[" + m_key + "]" + enum_string(m_type) + ":" + col.parameter();

    result.second.append(col.parameter(), col);

    for (auto const& v : m_other_comparisons)
    {
        internal_postfix += 1;

        std::pair<std::string, sqlite::row> const osr =
            v.second.statement_and_row(internal_postfix);

        for (auto const& osrv : osr.second.columns())
            result.second.append(osrv.first, osrv.second);

        result.first += enum_string(v.first) + osr.first;
    }

    if (result.first.empty())
        throw std::runtime_error {"Empty comparison statement."};

    result.first = "(" + result.first + ")";

    return result;
}
} // namespace bookmarks
} // namespace mm
