module;
#include "fmt/args.h"
#include "fmt/format.h"

export module fmt;

export namespace fmt
{
    inline std::string format_map(const std::string& src, 
                      const std::unordered_map<std::string, std::string>& formats) {
        
        // For each placeholder in the string, find the corresponding value in the map
        fmt::dynamic_format_arg_store<fmt::format_context> store;
        for (const auto& [key, value] : formats) {
            store.push_back(fmt::arg(key.c_str(), value));
        }
    
        return fmt::vformat(src, store);
    }

    using fmt::format;
    using fmt::print;

    inline void replace_all(std::string& data, std::string to_search, std::string replace_str){

        size_t pos = data.find(to_search);

        while( pos != std::string::npos){
            data.replace(pos, to_search.size(), replace_str);
            pos =data.find(to_search, pos + replace_str.size());
        }
    }

    inline void replace_all(std::string& data, std::unordered_map<std::string, std::string> replace_strs)
    {
        for (auto [key, val] : replace_strs)
        {
            replace_all(data, key, val);
        }
    }

}
