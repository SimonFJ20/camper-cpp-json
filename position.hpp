#pragma once

#include <concepts>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Position {
    size_t index;
    size_t line;
    size_t column;

    constexpr auto value(std::string_view text, size_t length) const noexcept -> std::string
    {
        return std::string { text.substr(this->index, length) };
    }
};

struct Error {
    Position pos;
    std::string message;

    constexpr auto to_string() const noexcept -> std::string
    {
        return std::format("error: {}, at {}:{}", this->message, this->pos.line, this->pos.column);
    }
};

struct ErrorCollector {
    constexpr void add(Error error) { errors.push_back(error); }
    constexpr auto contains_error() const noexcept -> bool { return not this->errors.empty(); }

    std::vector<Error> errors = {};
};
