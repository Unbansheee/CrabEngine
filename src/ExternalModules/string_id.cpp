module Engine.StringID;
import Engine.Application;
import std;

string_id MakeStringID(const std::string& str)
{
    return string_id{str.c_str(), Application::GetStringDB()};
}

string_id MakeStringID(const char* str)
{
    return string_id{str, Application::GetStringDB()};
}


string_id MakeStringID(const std::wstring& str)
{
    //setup converter
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    std::string converted_str = converter.to_bytes( str );

    return string_id{converted_str.c_str(), Application::GetStringDB()};
}