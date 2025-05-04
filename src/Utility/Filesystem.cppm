//
// Created by Vinnie on 25/04/2025.
//


module;

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__

#include <mach-o/dyld.h>
#include <climits>

#elif
#include <unistd.h>
#endif


export module Engine.Filesystem;
import Engine.Assert;
import vfspp;

// https://stackoverflow.com/questions/50889647/best-way-to-get-exe-folder-path/51023983#51023983
std::filesystem::path _GetExeDirectory() {
#ifdef _WIN32
    // Windows specific
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW( NULL, szPath, MAX_PATH );
#elif __APPLE__
    char szPath[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (!_NSGetExecutablePath(szPath, &bufsize))
        return std::filesystem::path{szPath}.parent_path() / ""; // to finish the folder path with (back)slash
    return {};  // some error
#else
    // Linux specific
    char szPath[PATH_MAX];
    ssize_t count = readlink( "/proc/self/exe", szPath, PATH_MAX );
    if( count < 0 || count >= PATH_MAX )
        return {}; // some error
    szPath[count] = '\0';
#endif
    return std::filesystem::path(szPath).parent_path();
}


export class Filesystem {
    friend class Application;

public:
    static vfspp::VirtualFileSystemPtr GetFilesystem();
    // Get a map of all registered aliases and directories
    static std::vector<std::pair<std::string, std::string>>& GetRegisteredDirectories();

    static bool IsAliasRegistered(const std::string &alias);

    // Add a new physical directory with an alias
    static void AddFileSystemDirectory(const std::string& alias, const std::string& root);

    // Add a virtual memory directory with an alias
    static void AddMemoryDirectory(const std::string& alias);

    // Get the folder that the exe is in
    static std::filesystem::path GetProgramDirectory() { return _GetExeDirectory(); }

    // Is the supplied path a virtual path (eg /res/stuff/things.txt)
    static bool IsVirtualPath(const std::string& path);

    // Get all files under all directorues. Files may double up if they exist under multiple directories
    static vfspp::IFileSystem::TFileList GetAllFiles();

    // Convert absolute to virtual path
    static std::string VirtualPath(const std::string& absolutePath);

    // Convert virtual path to absolute path
    static std::string AbsolutePath(const std::string& relativePath);

    // Normalize path and ensure all slashes are forward slashes
    static std::string NormalizePath(const std::string& path);

    // Convert std::string to std::wstring
    static std::wstring StringToWString(const std::string& str)
    {
        std::wstring wstr;
        size_t size;
        wstr.resize(str.length());
        mbstowcs_s(&size,&wstr[0],wstr.size()+1,str.c_str(),str.size());
        return wstr;
    }

    // Convert std::wstring to std::string
    static std::string WStringToString(const std::wstring& wstr)
    {
        std::string str;
        size_t size;
        str.resize(wstr.length());
        wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
        return str;
    }
};
