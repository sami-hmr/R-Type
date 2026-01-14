#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <future>
#include <optional>
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
    try {
      this->handle_responses(true);
    } catch (HttpBadCode) {
    }  // NOLINT
  }
}

void HttpClient::handle_responses(bool skipping)
{
  std::optional<std::size_t> to_handle;

  for (std::size_t i = 0; i < this->_handlers.size(); i++) {
    auto status = _handlers[i].call.wait_for(std::chrono::seconds(0));
    if (status == std::future_status::ready) {
      to_handle = i;
      break;
    }
  }

  if (!to_handle) {
    return;
  }

  auto result = _handlers[*to_handle].call.get();
  if (result.error() != httplib::Error::Success) {
    std::cout << "error with http request, code: " << result.error() << "\n";
    return;
  }
  if (result->status < 200 || result->status > 299) {
    this->_handlers.erase(this->_handlers.begin() + *to_handle);
    throw HttpBadCode(result->status, result->body);
  }
  if (!skipping) {
    _handlers[*to_handle].handler(_handlers[*to_handle].context, result);
  }
  this->_handlers.erase(this->_handlers.begin() + *to_handle);
}

httplib::Result HttpClient::send_get(const std::string& endpoint,
                                     httplib::Params const& params,
                                     httplib::Headers const& header)
{
  return this->_client.Get(endpoint, params, header);
}

httplib::Result HttpClient::send_post(const std::string& endpoint,
                                      std::string const& body,
                                      std::string const& content_type,
                                      httplib::Headers const& headers)
{
  return this->_client.Post(endpoint, headers, body, content_type);
}

httplib::Result HttpClient::send_delete(const std::string& endpoint,
                                        std::string const& body,
                                        std::string const& content_type)
{
  return this->_client.Delete(endpoint, body, content_type);
}
