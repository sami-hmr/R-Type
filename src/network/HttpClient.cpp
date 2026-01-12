#include <algorithm>
#include <chrono>
#include <future>
#include <vector>

#include "network/HttpClient.hpp"

#include <resolv.h>

#include "network/Httplib.hpp"

HttpClient::HttpClient(std::string const& api_uri)
    : _client(api_uri) {};

HttpClient::HttpClient(std::string const& host, int port)
    : _client(host, port) {};

HttpClient::~HttpClient()
{
  while (!this->_handlers.empty()) {
    this->handle_responses(true);
  }
}

void HttpClient::handle_responses(bool skipping)
{
  std::vector<std::size_t> handeled_responses;

  for (std::size_t i = 0; i < this->_handlers.size(); i++) {
    auto status = _handlers[i].call.wait_for(std::chrono::seconds(0));
    if (status != std::future_status::ready) {
      continue;
    }
    handeled_responses.push_back(i);
    auto result = _handlers[i].call.get();

    if (result.error() != httplib::Error::Success) {
      std::cout << "error with http request, code: " << result.error() << "\n";
      continue;
    }
    if (!skipping) {
      _handlers[i].handler(_handlers[i].context, result);
    }
  }

  std::reverse(handeled_responses.begin(), handeled_responses.end());
  for (auto i : handeled_responses) {
    this->_handlers.erase(this->_handlers.begin() + i);
  }
}

httplib::Result HttpClient::send_get(const std::string& endpoint,
                                     httplib::Params const& params,
                                     httplib::Headers const& header)
{
  return this->_client.Get(endpoint, params, header);
}

httplib::Result HttpClient::send_post(const std::string& endpoint,
                                      std::string const& body,
                                      std::string const& content_type, httplib::Headers const &headers)
{
  return this->_client.Post(endpoint, headers, body, content_type);
}

httplib::Result HttpClient::send_delete(const std::string& endpoint,
                                        std::string const& body,
                                        std::string const& content_type)
{
  return this->_client.Delete(endpoint, body, content_type);
}
