#pragma once

#include <functional>
#include <future>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CustomException.hpp"
#include "Parser.hpp"
#include "ParserTypes.hpp"
#include "network/Httplib.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/LoggerEvent.hpp"

class HttpClient
{
public:
  HttpClient() = default;
  HttpClient(const HttpClient&) = delete;
  HttpClient(HttpClient&&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;
  HttpClient& operator=(HttpClient&&) = delete;
  HttpClient(std::string const& api_uri);
  HttpClient(std::string const& host, int port);

  ~HttpClient();

  void init(std::string const& host, int port);

  template<typename F>
  void register_get(F&& handler,
                    void* context,
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
                                 context,
                                 std::forward<F>(handler));
  }

  template<typename F>
  void register_post(F&& handler,
                     void* context,
                     std::string const& endpoint,
                     std::string const& body = "",
                     std::string const& content_type = "application/json",
                     httplib::Headers const& headers = {})
  {
    this->_handlers.emplace_back(std::async(std::launch::async,
                                            &HttpClient::send_post,
                                            this,
                                            endpoint,
                                            body,
                                            content_type,
                                            headers),
                                 context,
                                 std::forward<F>(handler));
  }

  template<typename F>
  void register_delete(F&& handler,
                       void* context,
                       std::string const& endpoint,
                       std::string const& body = "",
                       std::string const& content_type = "application/json")
  {
    this->_handlers.emplace_back(std::async(std::launch::async,
                                            &HttpClient::send_delete,
                                            this,
                                            endpoint,
                                            body,
                                            content_type),
                                 context,
                                 std::forward<F>(handler));
  }

  void handle_responses(bool skipping = false);

private:
  httplib::Result send_get(const std::string& endpoint,
                           httplib::Params const& params,
                           httplib::Headers const& header);

  httplib::Result send_post(const std::string& endpoint,
                            std::string const& body,
                            std::string const& content_type,
                            httplib::Headers const&);

  httplib::Result send_delete(const std::string& endpoint,
                              std::string const& body,
                              std::string const& content_type);

  std::optional<httplib::Client> _client;

  struct Handler
  {
    std::future<httplib::Result> call;
    void* context;
    std::function<void(void*, httplib::Result const&)> handler;
  };

  std::vector<Handler> _handlers;
};

class HttpBadCode : public CustomException
{
public:
  explicit HttpBadCode(std::size_t code, const std ::string& message)
      : CustomException(message)
      , code(code)
  {
  }

  HttpBadCode with_context(std ::string const& key, std ::string const& value)
  {
    _context.insert_or_assign(key, value);
    return *this;
  }

  std::size_t code;
};

CUSTOM_EXCEPTION(ClientNotInitialized);

#define PARSE_HTTP_BODY(body, context, parser, type) /* NOLINT */ \
  ( \
      [&]() -> std::optional<type> \
      { \
        auto parsed = parser()(body); \
        if (parsed.index() == ERR) { \
          CONTEXT_LOGGER( \
              context, \
              "http", \
              LogLevel::ERROR, \
              std::format( \
                  "failed to parse http response: {}, \nresponse: \"{}\"", \
                  std::get<ERR>(parsed).message, \
                  body)); \
          return std::nullopt; \
        } \
        return std::get<SUCCESS>(parsed).value; \
      })()
