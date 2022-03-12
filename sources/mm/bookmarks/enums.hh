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
#include <stdexcept>

namespace mm
{
namespace bookmarks
{
enum class similarity_type
{
    NONE               = 0,
    EQUAL              = 1,
    NOT_EQUAL          = 2,
    LESS_THAN          = 3,
    LESS_THAN_OR_EQUAL = 4,
    MORE_THAN          = 5,
    MORE_THAN_OR_EQUAL = 6,
    LIKE               = 7,
};


inline std::string enum_string(similarity_type const& type)
{
    switch (type)
    {
    case similarity_type::EQUAL:
        return " == ";
    case similarity_type::NOT_EQUAL:
        return " != ";
    case similarity_type::LESS_THAN:
        return " < ";
    case similarity_type::LESS_THAN_OR_EQUAL:
        return " <= ";
    case similarity_type::MORE_THAN:
        return " > ";
    case similarity_type::MORE_THAN_OR_EQUAL:
        return " >= ";
    case similarity_type::LIKE:
        return " LIKE ";
    default:
        throw std::runtime_error {"Invalid similarity check."};
    }
}


enum class logical_type
{
    NONE = 0,
    AND  = 1,
    OR   = 2,
};


inline std::string enum_string(logical_type const& type)
{
    switch (type)
    {
    case logical_type::AND:
        return " AND ";
    case logical_type::OR:
        return " OR ";
    default:
        throw std::runtime_error {"Invalid logical check."};
    }
}


enum class source_type
{
    NONE           = 0,
    MMBOOKMARKS    = 1,
    FIREFOX_SQLITE = 2,
};
} // namespace bookmarks
} // namespace mm
