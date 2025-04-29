module Engine.StringID;
import Engine.Application;

string_id MakeStringID(const std::string& str)
{
    return string_id{str.c_str(), Application::GetStringDB()};
}

string_id MakeStringID(const char* str)
{
    return string_id{str, Application::GetStringDB()};
}