#include "epiphany/database/database.h"
#include "epiphany/server/http_server.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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
  db->Execute(
      "CREATE UNIQUE INDEX IF NOT EXISTS idx_items_title ON items(title);");

  // Seed Data - Generate 1000 products
  const std::vector<std::string> categories = {
      "手机", "笔记本电脑", "平板电脑", "耳机",   "智能手表",
      "相机", "电视",       "冰箱",     "洗衣机", "空调",
      "键盘", "鼠标",       "显示器",   "音箱",   "路由器"};
  const std::vector<std::string> brands = {
      "Apple",    "Samsung", "Xiaomi", "Huawei",  "Sony",
      "Dell",     "Lenovo",  "ASUS",   "LG",      "Panasonic",
      "Logitech", "Bose",    "JBL",    "TP-Link", "Dyson"};
  const std::vector<std::string> adjectives = {
      "Pro", "Max",  "Ultra", "Plus",    "Lite",
      "Air", "Mini", "Elite", "Premium", "Standard"};

  // Real product images mapped by category (using Unsplash CDN)
  const std::vector<std::vector<std::string>> category_images = {
      // 手机 (smartphones)
      {"https://images.unsplash.com/"
       "photo-1511707171634-5f897ff02aa9?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1592899677977-9c10ca588bbd?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1605236453806-6ff36851218e?w=300&h=300&fit=crop"},
      // 笔记本电脑 (laptops)
      {"https://images.unsplash.com/"
       "photo-1496181133206-80ce9b88a853?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1525547719571-a2d4ac8945e2?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1588872657578-7efd1f1555ed?w=300&h=300&fit=crop"},
      // 平板电脑 (tablets)
      {"https://images.unsplash.com/"
       "photo-1544244015-0df4b3ffc6b0?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1585790050230-5dd28404ccb9?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1561154464-82e9adf32764?w=300&h=300&fit=crop"},
      // 耳机 (headphones)
      {"https://images.unsplash.com/"
       "photo-1505740420928-5e560c06d30e?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1583394838336-acd977736f90?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1484704849700-f032a568e944?w=300&h=300&fit=crop"},
      // 智能手表 (smartwatches)
      {"https://images.unsplash.com/"
       "photo-1523275335684-37898b6baf30?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1546868871-7041f2a55e12?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1579586337278-3befd40fd17a?w=300&h=300&fit=crop"},
      // 相机 (cameras)
      {"https://images.unsplash.com/"
       "photo-1516035069371-29a1b244cc32?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1502920917128-1aa500764cbd?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1510127034890-ba27508e9f1c?w=300&h=300&fit=crop"},
      // 电视 (TVs)
      {"https://images.unsplash.com/"
       "photo-1593359677879-a4bb92f829d1?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1567690187548-f07b1d7bf5a9?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1461151304267-38535e780c79?w=300&h=300&fit=crop"},
      // 冰箱 (refrigerators)
      {"https://images.unsplash.com/"
       "photo-1571175443880-49e1d25b2bc5?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1584568694244-14fbdf83bd30?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1536353284924-9220c464e262?w=300&h=300&fit=crop"},
      // 洗衣机 (washing machines)
      {"https://images.unsplash.com/"
       "photo-1626806787461-102c1bfaaea1?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1604335399105-a0c585fd81a1?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1610557892470-55d9e80c0571?w=300&h=300&fit=crop"},
      // 空调 (air conditioners)
      {"https://images.unsplash.com/"
       "photo-1585771724684-38269d6639fd?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1631567937959-6a82a7c8bb00?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1625961332771-3f40b0e2bdcf?w=300&h=300&fit=crop"},
      // 键盘 (keyboards)
      {"https://images.unsplash.com/"
       "photo-1587829741301-dc798b83add3?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1618384887929-16ec33fab9ef?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1595225476474-87563907a212?w=300&h=300&fit=crop"},
      // 鼠标 (mice)
      {"https://images.unsplash.com/"
       "photo-1527864550417-7fd91fc51a46?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1615663245857-ac93bb7c39e7?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1613141411244-0e4ac259d217?w=300&h=300&fit=crop"},
      // 显示器 (monitors)
      {"https://images.unsplash.com/"
       "photo-1527443224154-c4a3942d3acf?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1585792180666-f7347c490ee2?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1616763355548-1b606f439f86?w=300&h=300&fit=crop"},
      // 音箱 (speakers)
      {"https://images.unsplash.com/"
       "photo-1545454675-3531b543be5d?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1608043152269-423dbba4e7e1?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1507003211169-0a1dd7228f2d?w=300&h=300&fit=crop"},
      // 路由器 (routers)
      {"https://images.unsplash.com/"
       "photo-1606904825846-647eb07f5be2?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1544197150-b99a580bb7a8?w=300&h=300&fit=crop",
       "https://images.unsplash.com/"
       "photo-1558494949-ef010cbdcc31?w=300&h=300&fit=crop"}};

  std::srand(42); // Fixed seed for reproducibility
  for (int i = 1; i <= 1000; ++i) {
    const std::string &brand = brands[i % brands.size()];
    int category_idx = i % categories.size();
    const std::string &category = categories[category_idx];
    const std::string &adj = adjectives[i % adjectives.size()];

    // Select image from category-specific images
    const auto &imgs = category_images[category_idx];
    std::string img = imgs[(i / categories.size()) % imgs.size()];

    std::string title =
        brand + " " + category + " " + adj + " " + std::to_string(i);
    double base_price = 500.0 + (i % 50) * 200.0 + (std::rand() % 1000);
    std::string price = std::to_string(static_cast<int>(base_price)) + ".0";

    db->Execute("INSERT OR IGNORE INTO items (title, price, image_url) VALUES "
                "(?, ?, ?);",
                {title, price, img});
  }
  std::cout << "Seeded 1000 products." << std::endl;

  int port = 8080;
  const char *env_port = std::getenv("EP_PORT");
  if (env_port) {
    try {
      port = std::stoi(env_port);
    } catch (...) {
    }
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
