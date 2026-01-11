#pragma once

#include <cstdint>
#include <functional>
#include <future>
#include <string>
#include <utility>
#include <vector>

#include "network/Httplib.hpp"

class HttpClient
{
public:
  HttpClient(std::string const& api_uri);
  HttpClient(std::string const& host, int port);

  template<typename F>
  void register_get(F&& handler,
                    std::string const& endpoint,
                    httplib::Params const& params = {},
                    httplib::Headers const& header = {})
  {
    this->_handlers.emplace_back(std::async(std::launch::async,
                                            &HttpClient::send_get,
                                            this,
                                            endpoint,
                                            params,
                                            header),
                                 std::forward<F>(handler));
  }

  template<typename F>
  void register_post(F&& handler,
                     std::string const& endpoint,
                     std::string const& body = "",
                     std::string const& content_type = "application/json")
  {
    this->_handlers.emplace_back(std::async(std::launch::async,
                                            &HttpClient::send_post,
                                            this,
                                            endpoint,
                                            body,
                                            content_type),
                                 std::forward<F>(handler));
  }

  void handle_responses(void*);

private:
  httplib::Result send_get(const std::string& endpoint,
                           httplib::Params const& params,
                           httplib::Headers const& header);

  httplib::Result send_post(const std::string& endpoint,
                            std::string const& body,
                            std::string const& content_type);

  httplib::Client _client;

  struct Handler
  {
    std::future<httplib::Result> call;
    std::function<void(void*, httplib::Result const&)> handler;
  };

  std::vector<Handler> _handlers;
};
