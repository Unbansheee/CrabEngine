module Engine.UID;
UID::UID() {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};
    id = gen();
}

UID::UID(uuids::uuid _id) {
    id = _id;
}

UID::UID(const UID &other) {
    id = other.id;
}

UID::UID(const std::string &string) {
    auto ret = uuids::uuid::from_string(string);
    if (ret.has_value())
    {
        id = ret.value();
    }
    else assert(false);
}

bool UID::operator==(const UID &other) const {
    return id == other.id;
}

std::string UID::ToString() const {
    return uuids::to_string(id);
}

UID UID::empty() {
    static UID id = (uuids::uuid());
    return id;
}
