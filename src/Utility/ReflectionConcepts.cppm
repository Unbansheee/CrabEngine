module;
#include <vector>

export module Engine.Reflection.Concepts;

export class Property;
export struct ClassType;

// Concept for auto-detecting reflected classes
export template <typename T>
concept Reflectable = requires {
    { T::GetClassProperties() } -> std::same_as<const std::vector<Property>&>;
    { T::GetStaticClass() } -> std::same_as<const ClassType&>;
};
