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

class CustomException : public std::exception {
   public:
    CustomException(std::string message) : message_(message) {
    }
    const char *what() const noexcept override {
        return message_.c_str();
    }

   private:
    std::string message_;
};

#endif
