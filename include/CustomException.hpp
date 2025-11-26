/*
** EPITECH PROJECT, 2025
** raytracer
** File description:
** customException base class
*/

#ifndef CUSTOM_EXCEPTION_HPP
#define CUSTOM_EXCEPTION_HPP

#include <exception>
#include <string>
#include <utility>

class CustomException : public std::exception
{
public:
  explicit CustomException(std::string const& message)
      : _message(message)
  {
  }

  explicit CustomException(std::string&& message)
      : _message(std::move(message))
  {
  }

  const char* what() const noexcept override { return _message.c_str(); }

private:
  std::string _message;
};

#define CUSTOM_EXCEPTION(name) \
    class name : public CustomException{ public: explicit name(const std::string& message) : CustomException(message){} };

#endif
