#pragma once
#include "epiphany/searcher/searcher.h"
#include <chrono>
#include <memory>
#include <random>
#include <sstream>
#include <string>
namespace epiphany {
namespace qrs {
class QRS {
public:
  explicit QRS(std::shared_ptr<epiphany::database::Database> db)
      : searcher_(std::make_shared<epiphany::searcher::Searcher>(db)) {}
  std::string Search(const std::string &q, int limit, int offset) {
    return searcher_->Search(q, limit, offset).first;
  }
  std::string SearchV2(const std::string &q, int limit, int offset) {
    auto t0 = std::chrono::steady_clock::now();
    auto result = searcher_->Search(q, limit, offset);
    auto t1 = std::chrono::steady_clock::now();
    auto search_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::ostringstream oss;
    oss << "{\"trace_id\":\"" << GenerateTraceId() << "\",\"limit\":" << limit
        << ",\"offset\":" << offset << ",\"total\":" << result.second
        << ",\"elapsed_ms\":" << search_ms
        << ",\"parse_ms\":0,\"route_ms\":0,\"search_ms\":" << search_ms
        << ",\"aggregate_ms\":0,\"items\":" << result.first << "}";
    return oss.str();
  }
private:
  std::string GenerateTraceId() {
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(now);
    std::ostringstream oss;
    oss << std::hex << rng();
    return oss.str();
  }
  std::shared_ptr<epiphany::searcher::Searcher> searcher_;
};
} // namespace qrs
} // namespace epiphany
