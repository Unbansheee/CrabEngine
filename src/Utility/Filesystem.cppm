//
// Created by Vinnie on 25/04/2025.
//


module;

export module Engine.Filesystem;
import Engine.Assert;
import vfspp;

export class Filesystem {
    friend class Application;

public:
    static vfspp::VirtualFileSystemPtr GetFilesystem();
    static std::vector<std::pair<std::string, std::string>>& GetRegisteredDirectories();

    static void AddFileSystemDirectory(const std::string& alias, const std::string& root);
    static void AddMemoryDirectory(const std::string& alias);


    static bool IsVirtualPath(const std::string& path);

    static vfspp::IFileSystem::TFileList GetAllFiles();

    static std::string VirtualPath(const std::string& absolutePath);
    static std::string AbsolutePath(const std::string& relativePath);
    static std::string NormalizePath(const std::string& path);

    static std::wstring StringToWString(const std::string& str)
    {
        std::wstring wstr;
        size_t size;
        wstr.resize(str.length());
        mbstowcs_s(&size,&wstr[0],wstr.size()+1,str.c_str(),str.size());
        return wstr;
    }

    static std::string WStringToString(const std::wstring& wstr)
    {
        std::string str;
        size_t size;
        str.resize(wstr.length());
        wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
        return str;
    }
};
