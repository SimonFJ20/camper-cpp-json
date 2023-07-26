#include "lexer.hpp"
#include "parser.hpp"
#include "position.hpp"
#include <format>
#include <iostream>

using namespace std::literals;

auto main() -> int
{
    auto text = R"({ 
        "firstName": "John",
        "lastName": "Smith",
        "isAlive": true,
        "age": 27,
        "address": {
            "streetAddress": "21 2nd Street",
            "city": "New York",
            "state": "NY",
            "postalCode": "10021-3100"
        },
        "phoneNumbers": [
            {
                "type": "home",
                "number": "212 555-1234"
            },
            {
                "type": "office",
                "number": "646 555-4567"
            }
        ],
        "children": [
            "Catherine",
            "Thomas",
            "Trevor"
        ],
        "spouse": null
    })"s;

    auto errors = ErrorCollector {};
    auto parser = Parser(text, &errors);
    auto tree = parser.parse();
    auto type = std::string(tree["phoneNumbers"s][1]["type"s]);

    if (errors.contains_error()) {
        std::cout << std::format("error(s) occured:\n");
        for (auto& error : errors.errors) {
            std::cout << std::format("{}\n", error.to_string());
        }
        return 0;
    }

    std::cout << std::format("please let '{}' equal 'office'\n", type);
}
