#include "epiphany/database/database.h"
#include "epiphany/server/http_server.h"
#include <iostream>

int main(int argc, char *argv[]) {
  // Default to "sqlite:epiphany.db" if no arg provided
  std::string conn_str = (argc > 1) ? argv[1] : "sqlite:epiphany.db";
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

  // Seed Data
  db->Execute("INSERT INTO items (title, price, image_url) SELECT 'iPhone 15 "
              "Pro', 7999.0, "
              "'https://img14.360buyimg.com/n1/jfs/t1/211520/38/35532/87961/"
              "6530a5c0Fb6887550/22877a5dd675d04f.jpg' WHERE NOT EXISTS "
              "(SELECT 1 FROM items WHERE title='iPhone 15 Pro');");
  db->Execute("INSERT INTO items (title, price, image_url) SELECT 'MacBook Pro "
              "M3', 12999.0, "
              "'https://img12.360buyimg.com/n1/jfs/t1/231143/33/2763/64388/"
              "6548a335F36cd4f97/813589b94000305a.jpg' WHERE NOT EXISTS "
              "(SELECT 1 FROM items WHERE title='MacBook Pro M3');");
  db->Execute(
      "INSERT INTO items (title, price, image_url) SELECT 'Xiaomi 14', 3999.0, "
      "'https://img10.360buyimg.com/n1/jfs/t1/220551/15/34764/95349/"
      "653b7089F53d4044a/62d8542cd5950d88.jpg' WHERE NOT EXISTS (SELECT 1 FROM "
      "items WHERE title='Xiaomi 14');");
  db->Execute("INSERT INTO items (title, price, image_url) SELECT 'Sony "
              "WH-1000XM5', 2499.0, "
              "'https://img13.360buyimg.com/n1/jfs/t1/185250/20/24220/55497/"
              "62907403E502bd664/1d00c363914757e2.jpg' WHERE NOT EXISTS "
              "(SELECT 1 FROM items WHERE title='Sony WH-1000XM5');");

  epiphany::server::HttpServer server(8080, std::move(db));
  server.Start();

  return 0;
}
