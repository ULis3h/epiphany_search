#include "epiphany/database/database.h"
#include "epiphany/server/http_server.h"
#include <iostream>
#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {
  std::string conn_str = (argc > 1) ? argv[1] : "sqlite:epiphany.db";
  const char *env_db = std::getenv("EP_DB");
  if (env_db && std::string(env_db).size() > 0) {
    conn_str = std::string(env_db);
  }
  std::cout << "Using database: " << conn_str << std::endl;

  auto db = epiphany::database::Database::Create(conn_str);
  if (!db) {
    std::cerr << "Failed to connect to database." << std::endl;
    return 1;
  }

  // Initialize Schema
  const std::string init_sql = "CREATE TABLE IF NOT EXISTS items ("
                               "id INTEGER PRIMARY KEY, "
                               "title TEXT, "
                               "price REAL, "
                               "image_url TEXT);";
  if (!db->Execute(init_sql)) {
    std::cerr << "Failed to initialize items table." << std::endl;
  }
  db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_items_title ON items(title);");

  // Seed Data
  db->Execute("INSERT OR IGNORE INTO items (title, price, image_url) VALUES (?, ?, ?);",
              {"iPhone 15 Pro", "7999.0",
               "https://img14.360buyimg.com/n1/jfs/t1/211520/38/35532/87961/6530a5c0Fb6887550/22877a5dd675d04f.jpg"});
  db->Execute("INSERT OR IGNORE INTO items (title, price, image_url) VALUES (?, ?, ?);",
              {"MacBook Pro M3", "12999.0",
               "https://img12.360buyimg.com/n1/jfs/t1/231143/33/2763/64388/6548a335F36cd4f97/813589b94000305a.jpg"});
  db->Execute("INSERT OR IGNORE INTO items (title, price, image_url) VALUES (?, ?, ?);",
              {"Xiaomi 14", "3999.0",
               "https://img10.360buyimg.com/n1/jfs/t1/220551/15/34764/95349/653b7089F53d4044a/62d8542cd5950d88.jpg"});
  db->Execute("INSERT OR IGNORE INTO items (title, price, image_url) VALUES (?, ?, ?);",
              {"Sony WH-1000XM5", "2499.0",
               "https://img13.360buyimg.com/n1/jfs/t1/185250/20/24220/55497/62907403E502bd664/1d00c363914757e2.jpg"});

  int port = 8080;
  const char *env_port = std::getenv("EP_PORT");
  if (env_port) {
    try {
      port = std::stoi(env_port);
    } catch (...) {}
  }
  std::string web_root = "epiphany/web";
  const char *env_web = std::getenv("EP_WEB_ROOT");
  if (env_web && std::string(env_web).size() > 0) {
    web_root = std::string(env_web);
  }
  epiphany::server::HttpServer server(port, std::move(db), web_root);
  server.Start();

  return 0;
}
