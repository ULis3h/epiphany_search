#pragma once
#include "epiphany/database/database.h"
#include <functional>
#include <memory>
#include <string>

namespace epiphany {
namespace server {

class HttpServer {
public:
  HttpServer(int port, std::shared_ptr<epiphany::database::Database> db);
  void Start();

private:
  void HandleClient(int client_socket);
  std::string ProcessRequest(const std::string &request);
  std::string GetMimeType(const std::string &path);
  std::string ReadFile(const std::string &path);

  int port_;
  std::shared_ptr<epiphany::database::Database> db_;
};

} // namespace server
} // namespace epiphany
