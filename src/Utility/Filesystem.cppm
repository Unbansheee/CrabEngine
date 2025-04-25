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

    static bool IsVirtualPath(const std::string& path);

    static std::string VirtualPath(const std::string& absolutePath);
    static std::string AbsolutePath(const std::string& relativePath);
    static std::string NormalizePath(const std::string& path);
};
