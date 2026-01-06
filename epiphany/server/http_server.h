#pragma once
#include "epiphany/database/database.h"
#include "epiphany/qrs/qrs.h"
#include <functional>
#include <memory>
#include <string>

namespace epiphany {
namespace server {

class HttpServer {
public:
  HttpServer(int port, std::shared_ptr<epiphany::database::Database> db, const std::string &web_root);
  void Start();

private:
  void HandleClient(int client_socket);
  std::string ProcessRequest(const std::string &request);
  std::string GetMimeType(const std::string &path);
  std::string ReadFile(const std::string &path);

  int port_;
  std::shared_ptr<epiphany::qrs::QRS> qrs_;
  std::string web_root_;
};

} // namespace server
} // namespace epiphany
