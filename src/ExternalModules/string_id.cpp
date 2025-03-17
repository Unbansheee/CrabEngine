module string_id;
import application;

string_id MakeStringID(const std::string& str)
{
    return string_id{str.c_str(), Application::Get().GetStringDB()};
}