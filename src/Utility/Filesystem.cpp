//
// Created by Vinnie on 26/04/2025.
//

module Engine.Filesystem;

vfspp::VirtualFileSystemPtr Filesystem::GetFilesystem() {
    static vfspp::VirtualFileSystemPtr g_applicationFileSystem;
    if (!g_applicationFileSystem) g_applicationFileSystem = std::make_shared<vfspp::VirtualFileSystem>();

    return g_applicationFileSystem;
}

bool Filesystem::IsAliasRegistered(const std::string &alias) {
    auto& dirs = GetRegisteredDirectories();
    auto it = std::ranges::find(dirs, alias, &std::pair<std::string, std::string>::first);
    return it != dirs.end();
}

std::vector<std::pair<std::string, std::string>> & Filesystem::GetRegisteredDirectories() {
    static std::vector<std::pair<std::string, std::string>> g_registeredDirectories;
    return g_registeredDirectories;
}

void Filesystem::AddFileSystemDirectory(const std::string &alias, const std::string &root) {
    auto fs = std::make_unique<vfspp::NativeFileSystem>(root);
    fs->Initialize();

    GetFilesystem()->AddFileSystem(alias, std::move(fs));
    GetRegisteredDirectories().push_back({alias, root});
}

void Filesystem::AddMemoryDirectory(const std::string &alias) {
    auto fs = std::make_unique<vfspp::MemoryFileSystem>();
    fs->Initialize();

    GetFilesystem()->AddFileSystem(alias, std::move(fs));
}

bool Filesystem::IsVirtualPath(const std::string &path) {
    for (auto& [alias, dir] : GetRegisteredDirectories()) {
        if (vfspp::StringUtils::StartsWith(path, alias)) {
            return true;
        }
    }
    return false;
}

vfspp::IFileSystem::TFileList Filesystem::GetAllFiles() {
    vfspp::IFileSystem::TFileList files;
    for (auto& [alias, dir] : GetRegisteredDirectories()) {
        for (auto& fs : GetFilesystem()->GetFilesystems(alias)) {
            files.insert(fs->FileList().begin(), fs->FileList().end());
        }
    }

    return files;
}

std::string Filesystem::VirtualPath(const std::string &absolutePath) {
    if (IsVirtualPath(absolutePath)) return absolutePath;

    std::string normalizedAbs = NormalizePath(absolutePath);

    std::string bestVirtual;
    size_t bestLength = 0;

    for (const auto& [virtualPath, physicalPath] : GetRegisteredDirectories()) {
        std::string normalizedPhys = NormalizePath(physicalPath);
        size_t physLen = normalizedPhys.length();

        if (normalizedAbs.length() < physLen) {
            continue;  // Absolute path is shorter than this physical path
        }

        // Check if the absolute path starts with this physical path
        if (normalizedAbs.compare(0, physLen, normalizedPhys) == 0) {
            if (physLen == normalizedAbs.length() || normalizedAbs[physLen] == '/') {
                // Ensure the match is either exact or followed by a slash
                if (physLen > bestLength) {
                    bestLength = physLen;
                    bestVirtual = virtualPath;
                }
            }
        }
    }

    if (bestLength == 0) {
        return "";  // No matching VFS path found
    }

    // Extract the remaining path after the matched physical part
    std::string remaining = normalizedAbs.substr(bestLength);
    std::string virtualPath = bestVirtual + remaining;
    return virtualPath;
}

std::string Filesystem::AbsolutePath(const std::string &relativePath) {
    if (!IsVirtualPath(relativePath)) return relativePath;

    std::string normalizedVirtual = NormalizePath(relativePath);

    std::string bestPhysical;
    size_t bestAliasLength = 0;

    // Iterate through all registered directories to find the best match
    for (const auto& [alias, physical] : GetRegisteredDirectories()) {
        std::string normalizedAlias = NormalizePath(alias);
        size_t aliasLen = normalizedAlias.length();

        // Check if the virtual path starts with this alias
        if (normalizedVirtual.compare(0, aliasLen, normalizedAlias) == 0) {
            if (aliasLen == normalizedVirtual.length() || normalizedVirtual[aliasLen] == '/') {

                // Prefer the longest matching alias
                if (aliasLen > bestAliasLength) {
                    bestAliasLength = aliasLen;

                    // Ensure the match is followed by a '/' or is the full path
                    bestPhysical = physical;
                }
            }
        }
    }

    if (bestAliasLength == 0) {
        return ""; // No matching alias found
    }

    // Extract the remaining part of the virtual path after the alias
    std::string remaining = normalizedVirtual.substr(bestAliasLength);


    // Remove leading slash from remaining (if present)
    if (!remaining.empty() && remaining[0] == '/') {
        remaining.erase(0, 1);
    }

    std::string normalizedPhysical = NormalizePath(bestPhysical);
    std::filesystem::path absolutePath = std::filesystem::path(normalizedPhysical) / remaining;

    return absolutePath.lexically_normal().generic_string();
}

std::string Filesystem::NormalizePath(const std::string &path) {
    std::filesystem::path p(path);
    p = p.lexically_normal();
    std::string normalized = p.generic_string();

    if (normalized.size() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }

    return normalized;
}
