#pragma once
#include "lexer.hpp"
#include "position.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace ast {

enum class NodeType {
    Error,

    Null,
    Bool,
    Int,
    Decimal,
    String,
    Array,
    Object,
};

struct Node {
    virtual ~Node() = default;
    virtual auto node_type() const noexcept -> NodeType = 0;
};
struct Error final : public Node {
    auto node_type() const noexcept -> NodeType override { return NodeType::Error; }
};
struct Null final : public Node {
    auto node_type() const noexcept -> NodeType override { return NodeType::Null; }
};
struct Bool final : public Node {
    Bool(bool value)
        : value(value)
    {
    }
    auto node_type() const noexcept -> NodeType override { return NodeType::Bool; }
    bool value;
};
struct Int final : public Node {
    Int(int64_t value)
        : value(value)
    {
    }
    auto node_type() const noexcept -> NodeType override { return NodeType::Int; }
    int64_t value;
};
struct Decimal final : public Node {
    Decimal(double value)
        : value(value)
    {
    }
    auto node_type() const noexcept -> NodeType override { return NodeType::Decimal; }
    double value;
};
struct String final : public Node {
    String(std::string value)
        : value(std::move(value))
    {
    }
    auto node_type() const noexcept -> NodeType override { return NodeType::String; }
    std::string value;
};
struct Array final : public Node {
    Array(std::vector<std::unique_ptr<ast::Node>> values)
        : values(std::move(values))
    {
    }
    auto node_type() const noexcept -> NodeType override { return NodeType::Array; }
    std::vector<std::unique_ptr<ast::Node>> values;
};
struct Object final : public Node {
    Object(std::unordered_map<std::string, std::unique_ptr<ast::Node>> fields)
        : fields(std::move(fields))
    {
    }
    auto node_type() const noexcept -> NodeType override { return NodeType::Object; }
    std::unordered_map<std::string, std::unique_ptr<ast::Node>> fields;
};

};

class Parser {
public:
    Parser(std::string_view text, ErrorCollector* errors)
        : text(text)
        , lexer(Lexer(text, errors))
        , errors(errors)
        , current(lexer.next())
    {
    }

    auto parse() -> std::unique_ptr<ast::Node>;

private:
    auto parse_null() noexcept -> std::unique_ptr<ast::Node>;
    auto parse_bool() noexcept -> std::unique_ptr<ast::Node>;
    auto parse_int() noexcept -> std::unique_ptr<ast::Node>;
    auto parse_decimal() noexcept -> std::unique_ptr<ast::Node>;
    auto parse_string() noexcept -> std::unique_ptr<ast::Node>;
    auto parse_array() -> std::unique_ptr<ast::Node>;
    auto parse_object_field() -> std::pair<std::string, std::unique_ptr<ast::Node>>;
    auto parse_object() -> std::unique_ptr<ast::Node>;

    constexpr auto string_literal_value(Token token) const noexcept -> std::string
    {
        return text.substr(token.pos.index + 1, token.length - 2);
    }

    constexpr auto add_unexpected_error(std::string_view expected) noexcept
    {
        this->errors->add({ current.pos,
            std::format("expected {}, got {}", expected, token_type_value(current.type)) });
    }

    constexpr void step() noexcept { this->current = this->lexer.next(); }
    constexpr auto done() const noexcept -> bool { return this->current.type == TokenType::Eof; }

    std::string text;
    Lexer lexer;
    ErrorCollector* errors;
    Token current;
};
