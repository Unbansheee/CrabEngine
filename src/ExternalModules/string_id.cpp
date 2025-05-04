module Engine.StringID;
import Engine.Application;
import std;
import Engine.Filesystem;

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
    std::string converted_str = Filesystem::WStringToString( str );
    return string_id{converted_str.c_str(), Application::GetStringDB()};
}