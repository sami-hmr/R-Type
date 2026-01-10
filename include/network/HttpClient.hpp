#pragma once

#include <functional>
#include <future>
#include <string>
#include <vector>

#include "network/Httplib.hpp"

class HttpClient
{
public:
  HttpClient(std::string const& api_uri);

  void register_request(std::function<void(httplib::Result)>&&,
                        std::string const& endpoint,
                        httplib::Params const& params = {},
                        httplib::Headers const& header = {});
  void handle_responses();

private:
  httplib::Result send_request(std::string const& endpoint,
                               httplib::Params const& params,
                               httplib::Headers const& header);

  httplib::Client _client;

  struct Handler
  {
    std::future<httplib::Result> call;
    std::function<void(httplib::Result)> handler;
  };

  std::vector<Handler> _handlers;
};
