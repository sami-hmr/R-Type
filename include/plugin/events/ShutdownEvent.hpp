/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Shutdown
*/

#pragma once

struct ShutdownEvent
{
    ShutdownEvent(std::string r, int e)
        : reason(std::move(r))
        , exit_code(e)
    {
    }
    DEFAULT_BYTE_CONSTRUCTOR(ShutdownEvent,
                            ([](std::string const& r, int e)
                                { return (ShutdownEvent) {r, e}; }),
                            parseByteString(),
                            parseByte<int>())

    DEFAULT_SERIALIZE(string_to_byte(this->reason), type_to_byte(this->exit_code))

    std::string reason;
    int exit_code = 0;
};