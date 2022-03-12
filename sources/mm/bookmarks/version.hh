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

namespace mm
{
namespace bookmarks
{
namespace version
{
unsigned int const major = 1;
unsigned int const minor = 0;
unsigned int const patch = 0;

std::string const string = std::to_string(major) + "." + std::to_string(minor) +
                           "." + std::to_string(patch);
} // namespace version
} // namespace bookmarks
} // namespace mm
