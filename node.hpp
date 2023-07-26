#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

enum class NodeType {
    Object,
    Array,
    Int,
    Double,
    String,
    Bool,
    Null,
};

inline auto type_to_string(const NodeType type) -> std::string
{
    switch (type) {
        case NodeType::Object: {
            return "object";
        }
        case NodeType::Array: {
            return "array";
        }
        case NodeType::Int: {
            return "int";
        }
        case NodeType::Double: {
            return "double";
        }
        case NodeType::String: {
            return "string";
        }
        case NodeType::Bool: {
            return "bool";
        }
        case NodeType::Null: {
            return "null";
        }
    }
};

class InvalidTypeAccess final : public std::runtime_error {
public:
    InvalidTypeAccess(NodeType expected, NodeType received) noexcept
        : std::runtime_error(
            "expected" + type_to_string(expected) + ", got " + type_to_string(received))
    {
    }
};

class Node;

using NodeVariantType = std::variant<std::unordered_map<std::string, Node>,
    std::vector<Node>,
    std::string,
    int64_t,
    double,
    bool,
    std::nullptr_t>;

class Node {
public:
    operator double()
    {
        if (!std::holds_alternative<double>(this->value)) {
            throw InvalidTypeAccess(NodeType::Double, this->type);
        }
        return std::get<double>(this->value);
    };
    operator int64_t()
    {
        if (!std::holds_alternative<int64_t>(this->value)) {
            throw InvalidTypeAccess(NodeType::Int, this->type);
        }
        return std::get<int64_t>(this->value);
    };
    operator std::string()
    {
        if (!std::holds_alternative<std::string>(this->value)) {
            throw InvalidTypeAccess(NodeType::String, this->type);
        }
        return std::get<std::string>(this->value);
    };
    operator bool()
    {
        if (!std::holds_alternative<bool>(this->value)) {
            throw InvalidTypeAccess(NodeType::Bool, this->type);
        }
        return std::get<bool>(this->value);
    };
    operator std::nullptr_t()
    {
        if (!std::holds_alternative<std::nullptr_t>(this->value)) {
            throw InvalidTypeAccess(NodeType::Null, this->type);
        }
        return std::get<std::nullptr_t>(this->value);
    };

    inline auto operator[](size_t index) -> Node
    {
        if (!std::holds_alternative<std::vector<Node>>(this->value)) {
            throw InvalidTypeAccess(NodeType::Array, this->type);
        }
        return std::get<std::vector<Node>>(this->value).at(index);
    };

    inline auto operator[](std::string index) -> Node
    {
        if (!std::holds_alternative<std::unordered_map<std::string, Node>>(this->value)) {
            throw InvalidTypeAccess(NodeType::Object, this->type);
        }
        auto map = std::get<std::unordered_map<std::string, Node>>(this->value);
        return map.at(index);
    };

    Node(NodeType type, NodeVariantType value)
        : type(type)
        , value(std::move(value)) {

        };

private:
    NodeType type;
    NodeVariantType value;
};
