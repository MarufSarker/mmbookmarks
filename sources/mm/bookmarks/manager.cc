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


#include "manager.hh"
#include "sql.hh"
#include "utilities.hh"
#include <mm/sqlite/utilities.hh>
#include <mm/sqlite/column.hh>
#include <mm/sqlite/row.hh>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace mm
{
namespace bookmarks
{
manager::manager() = default;


manager::~manager() { close(); }


manager::manager(std::string const& directory)
    : manager {directory, m_default_filename}
{
}


manager::manager(std::string const& directory, std::string const& filename)
{
    open(directory, filename);
}


void manager::open(std::string const& directory)
{
    open(directory, m_default_filename);
}


void manager::open(std::string const& directory, std::string const& filename)
{
    if (opened())
        throw std::runtime_error {"Database already opened."};

    constexpr static char const* separator =
#ifdef _WIN32
        "\\";
#else
        "/";
#endif

    m_filepath = directory + std::string {separator} + filename;
    m_database.open(m_filepath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    prepare_databases();
}


void manager::close()
{
    m_database.close();
    m_filepath.clear();
}


bool manager::opened() const { return m_database.opened(); }


void manager::prepare_databases()
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    for (auto const& v : sql::versions::create)
        m_database.execute(v);

    for (auto const& v : sql::bookmarks::create)
        m_database.execute(v);
}


void manager::vacuum_databases()
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    m_database.execute(sql::sqlite::vacuum);
}


void manager::insert_bookmarks(std::vector<bookmark> const& bookmarks)
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    sqlite::row _row {};

    std::string sql {};
    sql += "INSERT INTO mm_bookmarks ";
    sql += "([container], [type], [url], [title], [note]) VALUES ";

    for (size_t i = 0; i < bookmarks.size(); ++i)
    {
        bookmark bm = bookmarks.at(i);

        if (bm.container.empty())
            bm.container = sql::bookmarks::helpers::defaults::container;

        if (bm.type != sql::bookmarks::helpers::type::container &&
            bm.type != sql::bookmarks::helpers::type::url)
            throw std::runtime_error {"Invalid bookmark type."};

        sqlite::row tmp_row = bm.to_row(true, std::to_string(i));

        for (auto r : tmp_row.columns())
            _row.append(r.second.parameter(), r.second);

        std::string _sql {};

        _sql += "(";
        _sql += ":" + tmp_row.columns().at("container").parameter() + ", ";
        _sql += ":" + tmp_row.columns().at("type").parameter() + ", ";
        _sql += ":" + tmp_row.columns().at("url").parameter() + ", ";
        _sql += ":" + tmp_row.columns().at("title").parameter() + ", ";
        _sql += ":" + tmp_row.columns().at("note").parameter() + "";
        _sql += ")";
        _sql += ((i + 1) < bookmarks.size()) ? ", " : "";

        sql += _sql;
    }

    sql += ";";

    m_database.execute(sql, _row);
}


void manager::update_bookmarks(std::vector<bookmark> const& bookmarks)
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    if (bookmarks.empty())
        return;

    for (auto const& v : bookmarks)
    {
        sqlite::row              _row {};
        std::vector<std::string> tmps {};

        std::string sql = "UPDATE mm_bookmarks SET ";

        static auto _add =
            [&](std::string const&       name,
                std::string const&       param,
                std::string const&       value,
                sqlite::data_type const& type = sqlite::data_type::TEXT)
        {
            if (value.empty())
                return;
            tmps.push_back("[" + name + "] = :" + param);
            _row.append(name, sqlite::column {value, type, param});
        };

        _add("container", "NEWCONTAINER", v.container);
        _add("url", "NEWURL", v.url);
        _add("title", "NEWTITLE", v.title);
        _add("note", "NEWNOTE", v.note);

        if (tmps.empty())
            continue;

        for (size_t i = 0; i < tmps.size(); ++i)
            sql += ((i > 0) ? ", " : "") + tmps.at(i);

        sql += " WHERE [identifier] == :OLDIDENTIFIER;";
        _row.append("OLDIDENTIFIER",
                    sqlite::column {v.identifier, "OLDIDENTIFIER"});

        m_database.execute(sql, _row);
    }
}


void manager::delete_bookmarks(std::vector<std::string> const& identifiers)
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    if (identifiers.empty())
        return;

    comparison comp {similarity_type::EQUAL, "identifier", identifiers.at(0)};

    for (size_t i = 1; i < identifiers.size(); ++i)
    {
        comp.append(logical_type::OR,
                    comparison {similarity_type::EQUAL,
                                "identifier",
                                identifiers.at(i)});
    }

    std::pair<std::string, sqlite::row> comp_data = comp.statement_and_row();

    std::string sql = {};

    sql += "DELETE FROM mm_bookmarks WHERE ";
    sql += comp_data.first + ";";

    m_database.execute(sql, comp_data.second);
}


std::vector<bookmark> manager::select_bookmarks(
    comparison const&                                comparison_,
    std::vector<std::pair<std::string, bool>> const& order_by_and_asc,
    unsigned int const&                              limit,
    unsigned int const&                              offset)
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    std::pair<std::string, sqlite::row> comp = comparison_.statement_and_row();

    std::string order_by {};

    for (auto v : order_by_and_asc)
    {
        bookmark::valid_key(v.first);
        if (!order_by.empty())
            order_by += ", ";
        order_by += "[" + v.first + "] " + (v.second ? "ASC" : "DESC");
    }

    comp.second.append("MLIMIT",
                       sqlite::column {static_cast<int>(limit), "MLIMIT"});
    comp.second.append("MOFFSET",
                       sqlite::column {static_cast<int>(offset), "MOFFSET"});

    std::string sql = {};

    sql += "SELECT * FROM mm_bookmarks";
    sql += " WHERE " + comp.first;
    sql += " ORDER BY " + order_by;
    sql += " LIMIT :MLIMIT OFFSET :MOFFSET;";

    std::vector<bookmark>    result {};
    std::vector<sqlite::row> rows = m_database.execute(sql, comp.second);

    for (auto const& v : rows)
        result.push_back(bookmark {v});

    return result;
}


size_t manager::count_bookmarks(comparison const& comparison_)
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};

    std::pair<std::string, sqlite::row> comp = comparison_.statement_and_row();

    std::string sql = {};

    sql += "SELECT COUNT(*) FROM mm_bookmarks";
    sql += " WHERE " + comp.first;

    std::vector<sqlite::row> rows = m_database.execute(sql, comp.second);

    return static_cast<size_t>(
        sqlite::to_int(rows.at(0).columns().at("COUNT(*)").value()));
}


void manager::import_from(source_type const& type, std::string const& path)
{
    if (!opened())
        throw std::runtime_error {"Database need to be opened."};
    if (path.empty())
        throw std::runtime_error {"External source path is required."};

    std::vector<std::string> cleanup;
    std::string              detach;
    std::string              attach;
    std::vector<std::string> preparation;
    std::vector<std::string> process;

    switch (type)
    {
    case source_type::MMBOOKMARKS:
    {
        namespace ns = sql::imports::mm_bookmarks;
        cleanup      = {ns::cleanup.begin(), ns::cleanup.end()};
        detach       = ns::detach;
        attach       = ns::attach;
        preparation  = {ns::preparation.begin(), ns::preparation.end()};
        process      = {ns::process.begin(), ns::process.end()};
        break;
    }
    case source_type::FIREFOX_SQLITE:
    {
        namespace ns = sql::imports::firefox_places_sqlite;
        cleanup      = {ns::cleanup.begin(), ns::cleanup.end()};
        detach       = ns::detach;
        attach       = ns::attach;
        preparation  = {ns::preparation.begin(), ns::preparation.end()};
        process      = {ns::process.begin(), ns::process.end()};
        break;
    }
    default:
    {
        throw std::runtime_error {"Invalid external source type."};
    }
    }


    static auto _import_cleanup = [](sqlite::database&               database_,
                                     std::vector<std::string> const& cleanups_,
                                     std::string const&              detach_,
                                     bool const& already_attached)
    {
        // cleanup
        for (auto const& v : cleanups_)
            database_.execute(v);

        // error (acceptable) if not already attached
        try
        {
            database_.execute(detach_);
        }
        catch (std::exception const& e)
        {
            if (already_attached)
                std::rethrow_exception(std::current_exception());
            else
                std::cerr << "| Cleanup Error : " << e.what() << std::endl;
        }
    };


    static auto _import_prepare_and_process =
        [](sqlite::database&               database_,
           std::string const&              attach_,
           std::string const&              path_,
           std::vector<std::string> const& preparation_,
           std::vector<std::string> const& process_)
    {
        try
        {
            // prepare

            std::string const attach_sql =
                replace_substr(attach_, "{0}", escape_characters(path_));

            database_.execute(attach_sql);

            for (auto const& v : preparation_)
                database_.execute(v);

            // process

            for (auto const& v : process_)
                database_.execute(v);
        }
        catch (std::exception const& e)
        {
            std::cerr << "| Import Process Error : " << e.what() << std::endl;
        }
    };


    _import_cleanup(m_database, cleanup, detach, false);

    _import_prepare_and_process(m_database, attach, path, preparation, process);

    _import_cleanup(m_database, cleanup, detach, true);
}


void manager::logging(bool const& enable) { m_database.logging(enable); }


bool manager::logging() const { return m_database.logging(); }
} // namespace bookmarks
} // namespace mm
