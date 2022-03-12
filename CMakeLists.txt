# mmbookmarks
# Copyright (C) 2022  Maruf Sarker
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# [ Project

project(mmbookmarks)

if(NOT CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 11)
endif()

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ] Project

# [ Options

set(MM_SQLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mmsqlite"
    CACHE PATH "mmsqlite sources directory")

# ] Options

# [ Files

file(GLOB_RECURSE MM_HEADERS
    "sources/mm/bookmarks/*.hh"
)

file(GLOB_RECURSE MM_SOURCES
    "sources/mm/bookmarks/*.cc"
)

# ] Files

# [ mmsqlite

add_subdirectory("${MM_SQLITE_DIR}" mmsqlite)

# ] mmsqlite

# [ Library

add_library(${PROJECT_NAME}
    ${MM_SOURCES}
    ${MM_HEADERS}
)

# ] Library

# [ Compiler Options

set(MM_GNU_CXX_COMMON_FLAGS
    -s
    -fPIC
    -ftime-report
    -fdiagnostics-color=auto
    -Wall
    -Wextra
    -Wunused
    -Wshadow
    -Wpedantic
    -Wformat=2
    -Wconversion
    -Wcast-align
    -Wlogical-op
    -Wuseless-cast
    -Wold-style-cast
    -Wduplicated-cond
    -Wsign-conversion
    -Wdouble-promotion
    -Wnon-virtual-dtor
    -Wnull-dereference
    -Woverloaded-virtual
    -Wduplicated-branches
)

set(MM_GNU_CXX_COMPILE_FLAGS_RELEASE
    -O3
    -DNDEBUG
    ${MM_GNU_CXX_COMMON_FLAGS}
)

set(MM_GNU_CXX_COMPILE_FLAGS_DEBUG
    -O0
    -g3
    -ggdb3
    ${MM_GNU_CXX_COMMON_FLAGS}
)

# ] Compiler Options

# [ Target Options

target_link_libraries(${PROJECT_NAME}
    mmsqlite
    ${CMAKE_DL_LIBS}
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    "${MM_SQLITE_DIR}/sources"
)

target_compile_options(${PROJECT_NAME}
PUBLIC
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:
        ${MM_GNU_CXX_COMPILE_FLAGS_DEBUG}>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:
        ${MM_GNU_CXX_COMPILE_FLAGS_DEBUG}>

    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:
        ${MM_GNU_CXX_COMPILE_FLAGS_RELEASE}>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:
        ${MM_GNU_CXX_COMPILE_FLAGS_RELEASE}>
)

# ] Target Options
