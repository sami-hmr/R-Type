#include <algorithm>
#include <chrono>
#include <future>
#include <vector>

#include "network/HttpClient.hpp"

#include "network/Httplib.hpp"

HttpClient::HttpClient(std::string const& api_uri)
    : _client(api_uri) {};

httplib::Result HttpClient::send_request(std::string const& endpoint,
                                         httplib::Params const& params,
                                         httplib::Headers const& header)
{
  return this->_client.Get(endpoint, params, header);
}

void HttpClient::register_request(
    std::function<void(httplib::Result)>&& handler,
    std::string const& endpoint,
    httplib::Params const& params,
    httplib::Headers const& header)
{
  this->_handlers.emplace_back(std::async(std::launch::async,
                                          &HttpClient::send_request,
                                          this,
                                          endpoint,
                                          params,
                                          header),
                               std::move(handler));
  ;
}

void HttpClient::handle_responses()
{
  std::vector<std::size_t> handeled_responses;

  for (std::size_t i = 0; i < this->_handlers.size(); i++) {
    auto &it = this->_handlers[i];
    auto status = it.call.wait_for(std::chrono::seconds(0));
    if (status != std::future_status::ready) {
      continue;
    }
    it.handler(it.call.get());
    handeled_responses.push_back(i);
  }

  std::reverse(handeled_responses.begin(), handeled_responses.end());
  for (auto i : handeled_responses) {
      this->_handlers.erase(this->_handlers.begin() + i);
  }
}
