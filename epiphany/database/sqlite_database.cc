#include "epiphany/database/database.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <string>

namespace epiphany {
namespace database {

class SqliteDatabase : public Database {
public:
  SqliteDatabase(sqlite3 *db) : db_(db) {}

  ~SqliteDatabase() override {
    if (db_) {
      sqlite3_close(db_);
    }
  }

  bool Execute(const std::string &query) override {
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db_, query.c_str(), 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << (err_msg ? err_msg : "Unknown error")
                << std::endl;
      sqlite3_free(err_msg);
      return false;
    }
    return true;
  }

  std::string Search(const std::string &query, int limit, int offset) override {
    auto start = std::chrono::high_resolution_clock::now();

    auto escape_json = [](const std::string &s) {
      std::string out;
      out.reserve(s.size());
      for (char c : s) {
        switch (c) {
        case '\"':
          out += "\\\"";
          break;
        case '\\':
          out += "\\\\";
          break;
        case '\b':
          out += "\\b";
          break;
        case '\f':
          out += "\\f";
          break;
        case '\n':
          out += "\\n";
          break;
        case '\r':
          out += "\\r";
          break;
        case '\t':
          out += "\\t";
          break;
        default:
          if (static_cast<unsigned char>(c) < 0x20) {
            char buf[7];
            std::snprintf(buf, sizeof(buf), "\\u%04x",
                          static_cast<unsigned char>(c));
            out += buf;
          } else {
            out += c;
          }
        }
      }
      return out;
    };

    if (limit <= 0)
      limit = 10;
    if (limit > 100)
      limit = 100;
    if (offset < 0)
      offset = 0;

    const char *sql = "SELECT title, price, image_url FROM items WHERE title "
                      "LIKE ? LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, 0) != SQLITE_OK) {
      return "{\"items\": [], \"latency_ms\": 0}";
    }

    std::string like_query = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, like_query.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, limit);
    sqlite3_bind_int(stmt, 3, offset);

    std::string json_items = "[";
    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      if (!first)
        json_items += ",";
      first = false;
      const char *t = (const char *)sqlite3_column_text(stmt, 0);
      std::string title = t ? t : "";
      double price = sqlite3_column_double(stmt, 1);
      const char *img_ptr = (const char *)sqlite3_column_text(stmt, 2);
      std::string img = img_ptr ? img_ptr : "";

      json_items += "{\"title\":\"" + escape_json(title) +
                    "\", \"price\":" + std::to_string(price) +
                    ", \"image_url\":\"" + escape_json(img) + "\"}";
    }
    sqlite3_finalize(stmt);
    json_items += "]";

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::stringstream ss;
    ss << "{\"items\": " << json_items << ", \"latency_ms\": " << std::fixed
       << std::setprecision(3) << elapsed.count() << "}";
    return ss.str();
  }

  int Count(const std::string &query) override {
    const char *sql = "SELECT COUNT(*) FROM items WHERE title LIKE ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, 0) != SQLITE_OK) {
      return 0;
    }
    std::string like = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, like.c_str(), -1, SQLITE_TRANSIENT);
    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return total;
  }

private:
  sqlite3 *db_;
};

std::unique_ptr<Database>
Database::Create(const std::string &connection_string) {
  if (connection_string.rfind("sqlite:", 0) != 0) {
    std::cerr << "Invalid connection string. Must start with 'sqlite:'"
              << std::endl;
    return nullptr;
  }

  std::string filename = connection_string.substr(7);
  sqlite3 *handle;
  int rc = sqlite3_open(filename.c_str(), &handle);

  if (rc != SQLITE_OK) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(handle)
              << std::endl;
    sqlite3_close(handle);
    return nullptr;
  }

  return std::make_unique<SqliteDatabase>(handle);
}

} // namespace database
} // namespace epiphany
