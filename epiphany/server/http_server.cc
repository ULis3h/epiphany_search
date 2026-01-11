#include "epiphany/server/http_server.h"
#include "epiphany/observability/metrics.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace epiphany {
namespace server {

HttpServer::HttpServer(int port,
                       std::shared_ptr<epiphany::database::Database> db,
                       const std::string &web_root)
    : port_(port), qrs_(std::make_shared<epiphany::qrs::QRS>(db)),
      web_root_(web_root) {}

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
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_socket =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (new_socket >= 0) {
      char client_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
      int client_port = ntohs(client_addr.sin_port);
      HandleClient(new_socket, std::string(client_ip), client_port);
      close(new_socket);
    }
  }
}

// Helper function to parse HTTP headers
std::map<std::string, std::string> ParseHeaders(const std::string &request) {
  std::map<std::string, std::string> headers;
  std::istringstream iss(request);
  std::string line;
  // Skip first line (request line)
  std::getline(iss, line);
  while (std::getline(iss, line)) {
    if (line.empty() || line == "\r")
      break;
    // Remove trailing \r if present
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string key = line.substr(0, colon);
      std::string value = line.substr(colon + 1);
      // Trim leading whitespace from value
      size_t start = value.find_first_not_of(" \t");
      if (start != std::string::npos) {
        value = value.substr(start);
      }
      headers[key] = value;
    }
  }
  return headers;
}

void HttpServer::HandleClient(int client_socket, const std::string &client_ip,
                              int client_port) {
  char buffer[4096] = {0};
  read(client_socket, buffer, 4096);
  std::string request(buffer);
  {
    std::istringstream li(request);
    std::string m, p;
    li >> m >> p;
    std::cout << "[req] " << m << " " << p << " from " << client_ip << ":"
              << client_port << std::endl;
  }

  auto t0 = std::chrono::steady_clock::now();
  auto response = ProcessRequest(request, client_ip, client_port);
  auto t1 = std::chrono::steady_clock::now();
  auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  epiphany::observability::Metrics::RecordRequest();
  epiphany::observability::Metrics::RecordLatency(ms);
  send(client_socket, response.c_str(), response.size(), 0);
}

std::string HttpServer::ProcessRequest(const std::string &request,
                                       const std::string &client_ip,
                                       int client_port) {
  std::istringstream iss(request);
  std::string method, path, protocol;
  iss >> method >> path >> protocol;

  // Parse headers for client info
  auto headers = ParseHeaders(request);

  if (method != "GET") {
    epiphany::observability::Metrics::errors.fetch_add(1);
    return "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: "
           "application/json\r\n\r\n{\"error\":\"method not allowed\"}";
  }

  if (path == "/") {
    path = "/index.html";
  }

  if (path == "/health") {
    epiphany::observability::Metrics::health.fetch_add(1);
    return "HTTP/1.1 200 OK\r\nContent-Type: "
           "application/json\r\n\r\n{\"status\":\"ok\"}";
  }

  if (path.find("/metrics") == 0) {
    std::string json = epiphany::observability::Metrics::ToJson();
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json;
  }

  // Client info endpoint
  if (path.find("/api/client_info") == 0) {
    std::ostringstream json;
    json << "{";
    json << "\"ip\":\"" << client_ip << "\",";
    json << "\"port\":" << client_port << ",";
    json << "\"user_agent\":\""
         << (headers.count("User-Agent") ? headers["User-Agent"] : "") << "\",";
    json << "\"accept_language\":\""
         << (headers.count("Accept-Language") ? headers["Accept-Language"] : "")
         << "\",";
    json << "\"host\":\"" << (headers.count("Host") ? headers["Host"] : "")
         << "\",";
    json << "\"connection\":\""
         << (headers.count("Connection") ? headers["Connection"] : "") << "\",";
    json << "\"accept\":\""
         << (headers.count("Accept") ? headers["Accept"] : "") << "\",";
    json << "\"headers\":{";
    bool first = true;
    for (const auto &h : headers) {
      if (!first)
        json << ",";
      // Escape double quotes in header values
      std::string escaped_value;
      for (char c : h.second) {
        if (c == '"')
          escaped_value += "\\\"";
        else if (c == '\\')
          escaped_value += "\\\\";
        else
          escaped_value += c;
      }
      json << "\"" << h.first << "\":\"" << escaped_value << "\"";
      first = false;
    }
    json << "}";
    json << "}";
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" +
           json.str();
  }

  if (path.find("/api/search_v2") == 0) {
    epiphany::observability::Metrics::api_search_v2.fetch_add(1);
    size_t qm = path.find('?');
    std::string qs = (qm != std::string::npos) ? path.substr(qm + 1) : "";
    auto decode = [](const std::string &in) {
      std::string out;
      out.reserve(in.size());
      for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
          char hex[3] = {in[i + 1], in[i + 2], 0};
          int v = std::strtol(hex, nullptr, 16);
          out.push_back(static_cast<char>(v));
          i += 2;
        } else if (in[i] == '+') {
          out.push_back(' ');
        } else {
          out.push_back(in[i]);
        }
      }
      return out;
    };
    std::string q, limit_s, offset_s;
    std::istringstream qss(qs);
    std::string kv;
    while (std::getline(qss, kv, '&')) {
      size_t eq = kv.find('=');
      if (eq == std::string::npos)
        continue;
      std::string k = kv.substr(0, eq);
      std::string v = decode(kv.substr(eq + 1));
      if (k == "q")
        q = v;
      else if (k == "limit")
        limit_s = v;
      else if (k == "offset")
        offset_s = v;
    }
    if (q.empty()) {
      return "HTTP/1.1 400 Bad Request\r\nContent-Type: "
             "application/json\r\n\r\n{\"error\":\"missing q\"}";
    }
    int limit = 10, offset = 0;
    try {
      if (!limit_s.empty())
        limit = std::stoi(limit_s);
      if (!offset_s.empty())
        offset = std::stoi(offset_s);
    } catch (...) {
      return "HTTP/1.1 400 Bad Request\r\nContent-Type: "
             "application/json\r\n\r\n{\"error\":\"invalid limit or offset\"}";
    }
    std::string json = qrs_->SearchV2(q, limit, offset);
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json;
  }

  if (path.find("/api/search") == 0) {
    epiphany::observability::Metrics::api_search.fetch_add(1);
    size_t qm = path.find('?');
    std::string qs = (qm != std::string::npos) ? path.substr(qm + 1) : "";
    auto decode = [](const std::string &in) {
      std::string out;
      out.reserve(in.size());
      for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
          char hex[3] = {in[i + 1], in[i + 2], 0};
          int v = std::strtol(hex, nullptr, 16);
          out.push_back(static_cast<char>(v));
          i += 2;
        } else if (in[i] == '+') {
          out.push_back(' ');
        } else {
          out.push_back(in[i]);
        }
      }
      return out;
    };
    std::string q, limit_s, offset_s;
    std::istringstream qss(qs);
    std::string kv;
    while (std::getline(qss, kv, '&')) {
      size_t eq = kv.find('=');
      if (eq == std::string::npos)
        continue;
      std::string k = kv.substr(0, eq);
      std::string v = decode(kv.substr(eq + 1));
      if (k == "q")
        q = v;
      else if (k == "limit")
        limit_s = v;
      else if (k == "offset")
        offset_s = v;
    }
    if (q.empty()) {
      return "HTTP/1.1 400 Bad Request\r\nContent-Type: "
             "application/json\r\n\r\n{\"error\":\"missing q\"}";
    }
    int limit = 10, offset = 0;
    try {
      if (!limit_s.empty())
        limit = std::stoi(limit_s);
      if (!offset_s.empty())
        offset = std::stoi(offset_s);
    } catch (...) {
      return "HTTP/1.1 400 Bad Request\r\nContent-Type: "
             "application/json\r\n\r\n{\"error\":\"invalid limit or offset\"}";
    }
    std::string json = qrs_->Search(q, limit, offset);
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json;
  }

  if (path.find("..") != std::string::npos) {
    epiphany::observability::Metrics::errors.fetch_add(1);
    return "HTTP/1.1 400 Bad Request\r\nContent-Type: "
           "application/json\r\n\r\n{\"error\":\"invalid path\"}";
  }
  std::string content = ReadFile(web_root_ + path);
  if (!content.empty()) {
    return "HTTP/1.1 200 OK\r\nContent-Type: " + GetMimeType(path) +
           "\r\n\r\n" + content;
  }

  return "HTTP/1.1 404 Not Found\r\nContent-Type: "
         "application/json\r\n\r\n{\"error\":\"not found\"}";
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
