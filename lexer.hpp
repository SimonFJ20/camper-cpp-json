#pragma once

#include "position.hpp"
#include <cstddef>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>

enum class TokenType {
    Eof,
    Error,
    Id,

    Int,
    Decimal,
    String,
    True,
    False,
    Null,

    RBrace,
    LBrace,
    RBracket,
    LBracket,
    Colon,
    Comma,
};

auto token_type_to_string(TokenType type) noexcept -> std::string_view;
auto token_type_value(TokenType type) noexcept -> std::string_view;

struct Token {
    TokenType type;
    Position pos;
    size_t length;

    constexpr auto value(std::string_view text) const noexcept -> std::string
    {
        return pos.value(text, this->length);
    }

    constexpr auto to_string(std::string_view text) const noexcept -> std::string
    {
        return std::format("Token({}, \"{}\")",
            token_type_to_string(this->type),
            this->pos.value(text, this->length));
    }
};

auto keyword_token_type(std::string& value) noexcept -> TokenType;

class Lexer {
public:
    Lexer(std::string_view text, ErrorCollector* errors)
        : text(text)
        , errors(errors)
    {
    }

    auto next() noexcept -> Token;

private:
    auto lex_number() noexcept -> Token;
    auto lex_string() noexcept -> Token;
    auto lex_id() noexcept -> Token;
    auto skip_comment() noexcept -> Token;
    auto skip_whitespace() noexcept -> Token;

    constexpr void step() noexcept
    {
        if (current() == '\n') {
            this->line++;
            this->column = 1;
        }
        else {
            this->column++;
        }
        this->index++;
    }

    constexpr auto token(TokenType type, Position pos) const noexcept -> Token
    {
        return { type, pos, index - pos.index };
    }

    constexpr auto position() const noexcept -> Position
    {
        return { this->index, this->line, this->column };
    }

    constexpr auto current_is(char value) const noexcept -> bool
    {
        return not done() and current() == value;
    }

    constexpr auto done() const noexcept -> bool { return this->index >= this->text.length(); }

    constexpr auto current() const noexcept -> char { return this->text.at(this->index); }

    std::string_view text;
    ErrorCollector* errors;

    size_t index = 0;
    size_t line = 1;
    size_t column = 1;
};
