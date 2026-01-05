#pragma once
#include <memory>
#include <string>

namespace epiphany {
namespace database {

class Database {
public:
  virtual ~Database() = default;

  // Executes a query (e.g., INSERT, CREATE TABLE).
  // Returns true on success, false on failure.
  virtual bool Execute(const std::string &query) = 0;

  // Search for items/products.
  virtual std::string Search(const std::string &query) = 0;

  // Factory method to create a database instance.
  // connection_string example: "sqlite:test.db"
  static std::unique_ptr<Database> Create(const std::string &connection_string);
};

} // namespace database
} // namespace epiphany
