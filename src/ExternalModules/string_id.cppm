
export module string_id;
export import <string_id/string_id.hpp>;
export import <string_id/database.hpp>;

export using string_id = foonathan::string_id::string_id;
export namespace sid = foonathan::string_id;
export using namespace sid::literals;

export string_id MakeStringID(const std::string& str);


