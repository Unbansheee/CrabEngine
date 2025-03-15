module;
#pragma once
#include <cassert>
#include <cstdint>

export module uid;
export import <uuid.h>;
export struct UID
{
    UID()
    {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        uuids::uuid_random_generator gen{generator};
        id = gen();
    }

    UID(uuids::uuid _id)
    {
        id = _id;
    }

    UID(const UID& other)
    {
        id = other.id;
    }

    UID(const std::string& string)
    {
        auto ret = uuids::uuid::from_string(string);
        if (ret.has_value())
        {
            id = ret.value();
        }
        else assert(false);
    }

    bool operator==(const UID& other) const
    {
        return id == other.id;
    }
    
    std::string to_string()
    {
        return uuids::to_string(id);
    }

    static UID empty()
    {
        static UID id = (uuids::uuid());
        return id;
    }
    
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