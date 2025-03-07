#pragma once
#include <json.hpp>

#include "Reflection.h"

struct PropertySerializer
{
    // Handle all supported types
    void operator()(PropertyView& prop, nlohmann::json* archive, int& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, float& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, bool& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, std::string& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Vector2& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Vector3& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Vector4& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Quat& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Transform& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, auto val) { std::cout << "Invalid variant for serializing: " << prop.name() << "\n"; }
    

    // Optional: Default handler
    template <typename T>
    void operator()(T&&) {
        static_assert(sizeof(T) == 0, "Unhandled type in PropertyVisitor");
    }
};

struct PropertyDeserializer
{
    // Handle all supported types
    void operator()(PropertyView& prop, nlohmann::json* archive, int& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, float& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, bool& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, std::string& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Vector2& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Vector3& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Vector4& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Quat& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, Transform& val);
    void operator()(PropertyView& prop, nlohmann::json* archive, auto val) { std::cout << "Invalid variant for deserializing: " << prop.name() << "\n"; }
    

    // Optional: Default handler
    template <typename T>
    void operator()(T&&) {
        static_assert(sizeof(T) == 0, "Unhandled type in PropertyVisitor");
    }
};
