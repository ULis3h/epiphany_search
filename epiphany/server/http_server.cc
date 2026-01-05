#include "epiphany/server/http_server.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace epiphany {
namespace server {

HttpServer::HttpServer(int port,
                       std::shared_ptr<epiphany::database::Database> db)
    : port_(port), db_(db) {}

void HttpServer::Start() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("Socket creation failed");
    return;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    return;
  }

  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    return;
  }

  std::cout << "Server listening on port " << port_ << std::endl;

  while (true) {
    int new_socket = accept(server_fd, nullptr, nullptr);
    if (new_socket >= 0) {
      HandleClient(new_socket);
      close(new_socket);
    }
  }
}

void HttpServer::HandleClient(int client_socket) {
  char buffer[1024] = {0};
  read(client_socket, buffer, 1024);
  std::string request(buffer);

  std::string response = ProcessRequest(request);
  send(client_socket, response.c_str(), response.size(), 0);
}

std::string HttpServer::ProcessRequest(const std::string &request) {
  std::istringstream iss(request);
  std::string method, path, protocol;
  iss >> method >> path >> protocol;

  if (method != "GET") {
    return "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
  }

  if (path == "/") {
    path = "/index.html";
  }

  if (path.find("/api/search") == 0) {
    // Simple query params parsing
    size_t query_pos = path.find("q=");
    if (query_pos != std::string::npos) {
      std::string query = path.substr(query_pos + 2);
      // Decoding basics (replace %20 with space)
      size_t pos;
      while ((pos = query.find("%20")) != std::string::npos) {
        query.replace(pos, 3, " ");
      }
      std::string json = db_->Search(query);
      return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json;
    }
    return "HTTP/1.1 400 Bad Request\r\n\r\n";
  }

  // Static files
  std::string content = ReadFile("epiphany/web" + path);
  if (!content.empty()) {
    return "HTTP/1.1 200 OK\r\nContent-Type: " + GetMimeType(path) +
           "\r\n\r\n" + content;
  }

  return "HTTP/1.1 404 Not Found\r\n\r\nNot Found";
}

std::string HttpServer::ReadFile(const std::string &path) {
  std::ifstream f(path);
  if (f.good()) {
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
  }
  return "";
}

std::string HttpServer::GetMimeType(const std::string &path) {
  if (path.find(".html") != std::string::npos)
    return "text/html";
  if (path.find(".css") != std::string::npos)
    return "text/css";
  if (path.find(".js") != std::string::npos)
    return "application/javascript";
  return "text/plain";
}

} // namespace server
} // namespace epiphany
