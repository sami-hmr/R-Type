#pragma once
#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct HttpBadCodeEvent
{
  std::size_t code;
  std::string message;

  HttpBadCodeEvent(std::size_t code, std::string message)
      : code(code)
      , message(std::move(message))
  {
  }

  HttpBadCodeEvent(Registry& r,
                   JsonObject const& e,
                   std::optional<Ecs::Entity> entity)
      : code(get_value_copy<int>(r, e, "code", entity).value())
      , message(get_value_copy<std::string>(r, e, "message", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(HttpBadCodeEvent,
                           ([](std::size_t code, std::string const& m)
                            { return HttpBadCodeEvent(code, m); }),
                           parseByte<std::size_t>(),
                           parseByteString())

  DEFAULT_SERIALIZE(type_to_byte(code), string_to_byte(message))

  CHANGE_ENTITY_DEFAULT
};

struct FetchAvailableServers
{
  FetchAvailableServers() = default;

  EMPTY_BYTE_CONSTRUCTOR(FetchAvailableServers)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  FetchAvailableServers(Registry& /* */,
                        JsonObject const& /* */,
                        std::optional<Ecs::Entity>)
  {
  }

  HOOKABLE(FetchAvailableServers)
};

struct FetchAvailableServersSuccessfull
{
  FetchAvailableServersSuccessfull() = default;

  EMPTY_BYTE_CONSTRUCTOR(FetchAvailableServersSuccessfull)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  FetchAvailableServersSuccessfull(Registry& /* */,
                                   JsonObject const& /* */,
                                   std::optional<Ecs::Entity>)
  {
  }

  HOOKABLE(FetchAvailableServersSuccessfull)
};

struct ExposeServer
{
  std::string host;

  ExposeServer(std::string host)
      : host(std::move(host))
  {
  }

  ExposeServer(Registry& r,
               JsonObject const& e,
               std::optional<Ecs::Entity> entity)
      : host(get_value_copy<std::string>(r, e, "host", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ExposeServer,
                           ([](std::string const& h)
                            { return ExposeServer(h); }),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(host))

  CHANGE_ENTITY_DEFAULT
};

struct Register
{
  std::string identifier;
  std::string password;

  Register(std::string identifier, std::string password)
      : identifier(std::move(identifier))
      , password(std::move(password))
  {
  }

  Register(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : identifier(
            get_value_copy<std::string>(r, e, "identifier", entity).value())
      , password(get_value_copy<std::string>(r, e, "password", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Register,
                           ([](std::string const& i, std::string const& p)
                            { return Register(i, p); }),
                           parseByteString(),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(identifier), string_to_byte(password))

  CHANGE_ENTITY_DEFAULT
};

struct LoginSuccessfull
{
  int user;

  LoginSuccessfull(int u)
      : user(u)
  {
  }

  LoginSuccessfull(Registry& r,
                   JsonObject const& e,
                   std::optional<Ecs::Entity> entity)
      : user(get_value_copy<int>(r, e, "user", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(LoginSuccessfull,
                           ([](int u) { return LoginSuccessfull(u); }),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(user))

  CHANGE_ENTITY_DEFAULT
};

struct Login
{
  std::string identifier;
  std::string password;

  Login(std::string identifier, std::string password)
      : identifier(std::move(identifier))
      , password(std::move(password))
  {
  }

  Login(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : identifier(
            get_value_copy<std::string>(r, e, "identifier", entity).value())
      , password(get_value_copy<std::string>(r, e, "password", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Login,
                           ([](std::string const& i, std::string const& p)
                            { return Login(i, p); }),
                           parseByteString(),
                           parseByteString())

  DEFAULT_SERIALIZE(string_to_byte(identifier), string_to_byte(password))

  CHANGE_ENTITY_DEFAULT
};

struct Logout
{
  Logout() = default;

  EMPTY_BYTE_CONSTRUCTOR(Logout)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  Logout(Registry& /* */, JsonObject const& /* */, std::optional<Ecs::Entity>)
  {
  }

  HOOKABLE(Logout)
};

struct FailLogin
{
  FailLogin() = default;

  EMPTY_BYTE_CONSTRUCTOR(FailLogin)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  FailLogin(Registry& /* */,
            JsonObject const& /* */,
            std::optional<Ecs::Entity>)
  {
  }

  HOOKABLE(FailLogin)
};

struct Save
{
  Save() = default;

  EMPTY_BYTE_CONSTRUCTOR(Save)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  Save(Registry& /* */, JsonObject const& /* */, std::optional<Ecs::Entity>) {}

  HOOKABLE(Save)
};

struct SavePlayer
{
  int user;

  SavePlayer(int u)
      : user(u)
  {
  }

  SavePlayer(Registry& r,
             JsonObject const& e,
             std::optional<Ecs::Entity> entity)
      : user(get_value_copy<int>(r, e, "user", entity).value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(SavePlayer,
                           ([](int u) { return SavePlayer(u); }),
                           parseByte<int>())

  DEFAULT_SERIALIZE(type_to_byte(user))

  CHANGE_ENTITY_DEFAULT
};
