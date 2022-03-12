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


#include "utilities.hh"
#include <mm/sqlite/utilities.hh>
#include <algorithm>
#include <stdexcept>
#include <cctype>

namespace mm
{
namespace bookmarks
{
std::string uppercase(std::string const& str)
{
    if (str.empty())
        throw std::runtime_error {
            "Can not transform empty string to uppercase."};
    std::string result = str;
    std::transform(result.begin(),
                   result.end(),
                   result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}


std::string form_parameter(std::string const& name, std::string const& postfix)
{
    std::string result = uppercase(name) + postfix;
    sqlite::valid_sqlite_identifier(result);
    return result;
}


std::string escape_characters(std::string const&              str,
                              std::vector<std::string> const& characters)
{
    std::string tmp_str {};

    for (auto const& v : str)
    {
        std::string s {v};
        bool found = (std::find(characters.cbegin(), characters.cend(), s) !=
                      characters.end());
        tmp_str += found ? ("\\" + s) : s;
    }

    return tmp_str;
}


std::string replace_substr(std::string const& base_str,
                           std::string const& to_be_replaced,
                           std::string const& replace_with)
{
    size_t      index   = 0;
    std::string tmp_str = base_str;

    while (true)
    {
        index = tmp_str.find(to_be_replaced, index);

        if (index == std::string::npos)
            break;

        tmp_str.replace(index, to_be_replaced.size(), replace_with);
    }

    return tmp_str;
}
} // namespace bookmarks
} // namespace mm
