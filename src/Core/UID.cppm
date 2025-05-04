module;
#pragma once
#include <cassert>
#include <cstdint>

export module Engine.UID;
export import <vendor/uuid.h>;
export struct UID
{
    // Default constructor creates a new UID. For an empty one use UID::Empty();
    UID();

    // Construct from an underlying uuid object
    UID(uuids::uuid _id);

    // Copy ctor
    UID(const UID& other);

    // Construct from a valid UID string
    UID(const std::string& string);

    bool operator==(const UID& other) const;

    // Get as a string
    std::string ToString() const;

    // Return an empty UID
    static UID Empty();

private:
    friend struct std::hash<UID>;
    uuids::uuid id;
};

template<>
struct std::hash<UID>
{
    std::size_t operator()(const UID& s) const noexcept
    {
        return std::hash<uuids::uuid>()(s.id);
    }
};