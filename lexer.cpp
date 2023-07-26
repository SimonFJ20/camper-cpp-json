#include "lexer.hpp"
#include "position.hpp"
#include <format>
#include <iostream>

auto token_type_to_string(TokenType type) noexcept -> std::string_view
{
    switch (type) {
        case TokenType::Eof:
            return "Eof";
        case TokenType::Error:
            return "Error";
        case TokenType::Id:
            return "Id";
        case TokenType::Int:
            return "Int";
        case TokenType::Decimal:
            return "Decimal";
        case TokenType::String:
            return "String";
        case TokenType::True:
            return "True";
        case TokenType::False:
            return "False";
        case TokenType::Null:
            return "Null";
        case TokenType::RBrace:
            return "RBrace";
        case TokenType::LBrace:
            return "LBrace";
        case TokenType::RBracket:
            return "RBracket";
        case TokenType::LBracket:
            return "LBracket";
        case TokenType::Colon:
            return "Colon";
        case TokenType::Comma:
            return "Comma";
    }
}

auto token_type_value(TokenType type) noexcept -> std::string_view
{
    switch (type) {
        case TokenType::True:
            return "'true'";
        case TokenType::False:
            return "'false'";
        case TokenType::Null:
            return "'null'";
        case TokenType::RBrace:
            return "'{'";
        case TokenType::LBrace:
            return "'}'";
        case TokenType::RBracket:
            return "'['";
        case TokenType::LBracket:
            return "']'";
        case TokenType::Colon:
            return "','";
        case TokenType::Comma:
            return "'.'";
        default:
            return token_type_to_string(type);
    }
}

static const std::unordered_map<std::string, TokenType> keyword_token_types = {
    { "null", TokenType::Null },
    { "false", TokenType::False },
    { "true", TokenType::True },
};

auto keyword_token_type(std::string& value) noexcept -> TokenType
{
    if (auto type = keyword_token_types.find(value); type != keyword_token_types.end()) {
        return type->second;
    }
    return TokenType::Id;
}

auto Lexer::lex_number() noexcept -> Token
{
    const auto pos = position();
    auto is_decimal = false;
    step();
    while ((current() >= '0' and current() <= '9') or current() == '.') {
        if (current() == '.') {
            is_decimal = true;
        }
        step();
    }
    return token(is_decimal ? TokenType::Decimal : TokenType::Int, pos);
}

auto Lexer::lex_string() noexcept -> Token
{
    const auto pos = position();
    auto is_escaped = false;
    step();
    while (not done() and (current() != '"' or is_escaped)) {
        if (is_escaped) {
            is_escaped = false;
        }
        else if (current() == '\\') {
            is_escaped = true;
        }
        step();
    }
    if (is_escaped or not current_is('"')) {
        this->errors->add({ pos, "malformed string" });
        return token(TokenType::Error, pos);
    }
    step();

    return token(TokenType::String, pos);
}

auto Lexer::lex_id() noexcept -> Token
{
    const auto pos = position();
    const auto is_id_char = [](char value) constexpr {
        return (value >= '0' and value <= '9') or (value >= 'a' and value <= 'z')
               or (value >= 'A' and value <= 'Z');
    };
    while (not done() and is_id_char(current())) {
        step();
    }
    auto value = pos.value(this->text, this->index - pos.index);
    return token(keyword_token_type(value), pos);
}

auto Lexer::skip_comment() noexcept -> Token
{
    auto pos = position();
    step();
    if (current_is('*')) {
        step();
        auto depth = 1;
        auto last = std::optional<char>();
        while (not done() and depth > 0) {
            if (last and *last == '*' and current() == '/') {
                depth -= 1;
            }
            else if (last and *last == '/' and current() == '*') {
                depth += 1;
            }
            last = current();
            step();
        }
        if (depth > 0) {
            this->errors->add({ pos, "malformed multiline comment" });
            return token(TokenType::Error, pos);
        }
        return next();
    }
    if (current_is('/')) {
        while (not done() and current() != '\n') {
            step();
        }
        return next();
    }
    errors->add({ pos, "malformed comment" });
    return token(TokenType::Error, pos);
}

auto Lexer::skip_whitespace() noexcept -> Token
{
    while (current_is(' ') or current_is('\t') or current_is('\r') or current_is('\n')) {
        step();
    }
    return next();
}

auto Lexer::next() noexcept -> Token
{
    const auto pos = position();
    if (done()) {
        return token(TokenType::Eof, pos);
    }
    switch (current()) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return skip_whitespace();
        case '/':
            return skip_comment();
        case '{':
            return (step(), token(TokenType::LBrace, pos));
        case '}':
            return (step(), token(TokenType::RBrace, pos));
        case '[':
            return (step(), token(TokenType::LBracket, pos));
        case ']':
            return (step(), token(TokenType::RBracket, pos));
        case ':':
            return (step(), token(TokenType::Colon, pos));
        case ',':
            return (step(), token(TokenType::Comma, pos));
        case '0' ... '9':
            return lex_number();
        case '"':
            return lex_string();
        case 'A' ... 'Z':
        case 'a' ... 'z':
            return lex_id();
        default: {
            this->errors->add({ pos, std::format("unrecognized character '{}'", current()) });
            step();
            return token(TokenType::Error, pos);
        };
    }
}
