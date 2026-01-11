#pragma once
#include <memory>
#include <string>
#include <vector>

namespace epiphany {
namespace database {

class Database {
public:
  virtual ~Database() = default;

  // Executes a query (e.g., INSERT, CREATE TABLE).
  // Returns true on success, false on failure.
  virtual bool Execute(const std::string &query) = 0;
  virtual bool Execute(const std::string &query, const std::vector<std::string> &params) = 0;

  // Search for items/products.
  virtual std::string Search(const std::string &query, int limit, int offset) = 0;
  virtual int Count(const std::string &query) = 0;
  struct PriceAggregates {
    double avg{0.0};
    double min{0.0};
    double max{0.0};
  };
  virtual PriceAggregates PriceStats(const std::string &query) = 0;

  // Factory method to create a database instance.
  // connection_string example: "sqlite:test.db"
  static std::unique_ptr<Database> Create(const std::string &connection_string);
};

} // namespace database
} // namespace epiphany
