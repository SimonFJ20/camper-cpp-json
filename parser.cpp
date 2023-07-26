#include "parser.hpp"
#include "lexer.hpp"
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

auto Parser::parse() -> std::unique_ptr<ast::Node>
{
    switch (current.type) {
        case TokenType::Null:
            return parse_null();
        case TokenType::False:
        case TokenType::True:
            return parse_bool();
        case TokenType::Int:
            return parse_int();
        case TokenType::Decimal:
            return parse_decimal();
        case TokenType::String:
            return parse_string();
        case TokenType::LBracket:
            return parse_array();
        case TokenType::LBrace:
            return parse_object();
        default:
            return (add_unexpected_error("value"), step(), std::make_unique<ast::Error>());
    }
}

auto Parser::parse_null() noexcept -> std::unique_ptr<ast::Node>
{
    step();
    return std::make_unique<ast::Null>();
}

auto Parser::parse_bool() noexcept -> std::unique_ptr<ast::Node>
{
    auto value = current.type == TokenType::True;
    step();
    return std::make_unique<ast::Bool>(value);
}

auto Parser::parse_int() noexcept -> std::unique_ptr<ast::Node>
{
    auto value = std::stoll(this->current.value(this->text));
    step();
    return std::make_unique<ast::Int>(value);
}

auto Parser::parse_decimal() noexcept -> std::unique_ptr<ast::Node>
{
    auto value = std::stod(this->current.value(this->text));
    step();
    return std::make_unique<ast::Decimal>(value);
}

auto Parser::parse_string() noexcept -> std::unique_ptr<ast::Node>
{

    auto value = string_literal_value(this->current);
    step();
    return std::make_unique<ast::String>(std::move(value));
}

auto Parser::parse_array() -> std::unique_ptr<ast::Node>
{
    auto array = std::vector<std::unique_ptr<ast::Node>> {};
    while (not done()) {
        array.emplace_back(parse());

        if (current.type == TokenType::RBracket) {
            break;
        }
        if (current.type != TokenType::Comma) {
            add_unexpected_error("','");
        }
        step();
    }
    if (current.type != TokenType::RBracket) {
        add_unexpected_error("']'");
    }
    step();
    return std::make_unique<ast::Array>(std::move(array));
}

auto Parser::parse_object() -> std::unique_ptr<ast::Node>
{
    step();
    auto fields = std::unordered_map<std::string, std::unique_ptr<ast::Node>> {};
    while (not done()) {
        if (current.type != TokenType::String) {
            add_unexpected_error("String or '}'");
            while (not done() and this->current.type != TokenType::String
                   and this->current.type != TokenType::LBrace) {
                step();
            }
            continue;
        }
        auto key = string_literal_value(this->current);
        step();
        if (current.type != TokenType::Colon) {
            add_unexpected_error("':'");
        }
        else {
            step();
        }
        auto value = parse();
        fields.emplace(key, value);
        if (current.type == TokenType::RBrace) {
            break;
        }
        if (current.type != TokenType::Comma) {
            add_unexpected_error("','");
        }
        else {
            step();
        }
    }
    if (current.type != TokenType::RBrace) {
        add_unexpected_error("'}'");
    }
    step();
    return std::make_unique<ast::Object>(std::move(fields));
};
