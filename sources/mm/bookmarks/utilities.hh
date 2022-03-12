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

namespace mm
{
namespace bookmarks
{
std::string uppercase(std::string const& str);


std::string form_parameter(std::string const& name, std::string const& postfix);


std::string escape_characters(
    std::string const&              str,
    std::vector<std::string> const& characters = {"'", "\"", ";"});


std::string replace_substr(std::string const& base_str,
                           std::string const& to_be_replaced,
                           std::string const& replace_with);

} // namespace bookmarks
} // namespace mm
