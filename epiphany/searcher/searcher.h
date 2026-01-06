#pragma once
#include "epiphany/database/database.h"
#include <memory>
#include <string>
#include <utility>
namespace epiphany {
namespace searcher {
class Searcher {
public:
  explicit Searcher(std::shared_ptr<epiphany::database::Database> db) : db_(db) {}
  std::pair<std::string,int> Search(const std::string &q, int limit, int offset) {
    int total = db_->Count(q);
    std::string items = db_->Search(q, limit, offset);
    return {items, total};
  }
private:
  std::shared_ptr<epiphany::database::Database> db_;
};
} // namespace searcher
} // namespace epiphany
