#pragma once
#include <atomic>
#include <array>
#include <string>
namespace epiphany {
namespace observability {
struct Metrics {
  static std::atomic<long> requests;
  static std::atomic<long> api_search;
  static std::atomic<long> api_search_v2;
  static std::atomic<long> health;
  static std::atomic<long> errors;
  static std::atomic<long> total_latency_ms;
  static std::atomic<long> last_latency_ms;
  static std::array<std::atomic<long>, 10> latency_buckets;
  static std::string ToJson();
  static void RecordRequest();
  static void RecordLatency(long ms);
};
} // namespace observability
} // namespace epiphany
