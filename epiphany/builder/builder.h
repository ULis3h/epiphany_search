#pragma once
#include <string>
namespace epiphany {
namespace builder {
class Builder {
public:
  bool BuildOffline(const std::string &input_path, const std::string &output_path);
  bool BuildRealtime(const std::string &topic, const std::string &output_path);
};
} // namespace builder
} // namespace epiphany
