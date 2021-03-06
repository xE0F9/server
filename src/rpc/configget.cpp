#include "configget.hpp"

#include <boost/log/trivial.hpp>

#include "../data/datareader.hpp"

using json = nlohmann::json;
using pt::Server::RPC::ConfigGetCommand;

ConfigGetCommand::ConfigGetCommand(sqlite3* db)
    : m_db(db)
{
}

json ConfigGetCommand::Execute(json& params)
{
    if (params.is_array())
    {
        json result = json::object();

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(
            m_db,
            "SELECT value FROM config WHERE key = $1",
            -1,
            &stmt,
            nullptr);

        for (auto const& val : params)
        {
            std::string key = val.get<std::string>();
            sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

            switch (sqlite3_step(stmt))
            {
            case SQLITE_DONE:
                // nothing in the database
                result[key] = {};
                break;
            case SQLITE_ROW:
                auto val = pt_sqlite3_column_std_string(stmt, 0);
                result[key] = json::parse(val.value());
                break;
            }

            sqlite3_reset(stmt);
        }

        sqlite3_finalize(stmt);

        return Ok(result);
    }

    return Error(-1, "no config keys specified");
}
