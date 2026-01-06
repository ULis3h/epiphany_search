#include "epiphany/observability/metrics.h"
#include <sstream>
#include <array>
namespace epiphany {
namespace observability {
std::atomic<long> Metrics::requests{0};
std::atomic<long> Metrics::api_search{0};
std::atomic<long> Metrics::api_search_v2{0};
std::atomic<long> Metrics::health{0};
std::atomic<long> Metrics::errors{0};
std::atomic<long> Metrics::total_latency_ms{0};
std::atomic<long> Metrics::last_latency_ms{0};
std::array<std::atomic<long>, 10> Metrics::latency_buckets{
    std::atomic<long>{0}, std::atomic<long>{0}, std::atomic<long>{0},
    std::atomic<long>{0}, std::atomic<long>{0}, std::atomic<long>{0},
    std::atomic<long>{0}, std::atomic<long>{0}, std::atomic<long>{0},
    std::atomic<long>{0}};
static const long bucket_edges[10] = {1, 5, 10, 50, 100, 200, 500, 1000, 2000, 1LL << 60};
void Metrics::RecordRequest() { requests.fetch_add(1); }
void Metrics::RecordLatency(long ms) {
  last_latency_ms.store(ms);
  total_latency_ms.fetch_add(ms);
  for (int i = 0; i < 10; ++i) {
    if (ms <= bucket_edges[i]) {
      latency_buckets[i].fetch_add(1);
      break;
    }
  }
}
static long Percentile(double p) {
  long total = 0;
  for (int i = 0; i < 10; ++i) total += Metrics::latency_buckets[i].load();
  if (total == 0) return 0;
  long target = static_cast<long>(p * total);
  long acc = 0;
  for (int i = 0; i < 10; ++i) {
    acc += Metrics::latency_buckets[i].load();
    if (acc >= target) return bucket_edges[i];
  }
  return bucket_edges[9];
}
std::string Metrics::ToJson() {
  long req = requests.load();
  long avg = req > 0 ? total_latency_ms.load() / req : 0;
  long p95 = Percentile(0.95);
  long p99 = Percentile(0.99);
  std::ostringstream oss;
  oss << "{\"requests\":" << req << ",\"api_search\":" << api_search.load()
      << ",\"api_search_v2\":" << api_search_v2.load()
      << ",\"health\":" << health.load() << ",\"errors\":" << errors.load()
      << ",\"last_latency_ms\":" << last_latency_ms.load()
      << ",\"avg_latency_ms\":" << avg
      << ",\"p95_ms\":" << p95 << ",\"p99_ms\":" << p99 << "}";
  return oss.str();
}
} // namespace observability
} // namespace epiphany
