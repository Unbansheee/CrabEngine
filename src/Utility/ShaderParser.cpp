//
// Created by Vinnie on 31/03/2025.
//

module Engine.ShaderParser;
import Engine.Types;
import Engine.Assert;
import std;

std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

std::string ToLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) c = tolower(c);
    return result;
}


ShaderMetadata ShaderParser::Parse(const std::string &source) {
    ShaderMetadata metadata;
    std::istringstream stream(source);
    std::string line{};
    
    std::stringstream lightFn{};
    std::stringstream fragFn{};
    std::stringstream vertFn{};

    std::stringstream propertiesStruct{};
    
    int fragScope = 0;
    int vertScope = 0;
    int lightScope = 0;

    std::string baseShaderType{};
    
    while (std::getline(stream, line)) {
        if (ProcessPropertiesLine(line, metadata, propertiesStruct)) continue;
        ProcessBaseShaderTypeLine(line, baseShaderType);
        ProcessFunctionLine("fragment", fragScope, line, fragFn);
        ProcessFunctionLine("vertex", vertScope, line, vertFn);
        ProcessFunctionLine("light", lightScope, line, lightFn);
    }

    Assert::Check(fragScope == 0, "fragScope == 0", "Invalid scope for frag function");
    Assert::Check(vertScope == 0, "vertScope == 0", "Invalid scope for vert function");
    Assert::Check(lightScope == 0, "lightScope == 0", "Invalid scope for light function");

    
    if (!baseShaderType.empty())
    {
        std::ifstream base_shader_file(std::string(ShaderSources.at(baseShaderType)));
        std::string content(
           (std::istreambuf_iterator<char>(base_shader_file)),
           std::istreambuf_iterator<char>()
       );

        // Define our markers and their replacements
        const std::vector<std::pair<std::string, std::string>> replacements = {
            {"#VERTEX_FN", vertFn.str()},
            {"#FRAGMENT_FN", fragFn.str()},
            {"#LIGHT_FN", lightFn.str()}
        };

        std::string result;

        result += propertiesStruct.str();
    
        size_t start_pos = 0;
        size_t end_pos = content.find('\n');

        while (end_pos != std::string::npos) {
            std::string line = content.substr(start_pos, end_pos - start_pos);
            std::string line_trim = Trim(line);
            // Check if this line matches any of our markers
            bool replaced = false;
            for (const auto& [marker, replacement] : replacements) {
                if (line_trim == marker) {
                    result += replacement + "\n";
                    replaced = true;
                    break;
                }
            }
        
            if (!replaced) {
                result += line + "\n";
            }
        
            start_pos = end_pos + 1;
            end_pos = content.find('\n', start_pos);
        }
    
        // Add the last line if there's no trailing newline
        if (start_pos < content.size()) {
            std::string line = content.substr(start_pos);
            bool replaced = false;
            for (const auto& [marker, replacement] : replacements) {
                if (line == marker) {
                    result += replacement;
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                result += line;
            }
        }
    
        ProcessedSource = std::move(result);
        return metadata;

    }

    ProcessedSource = stream.str();
    return metadata;
}

bool ShaderParser::ProcessPropertiesLine(const std::string &line, ShaderMetadata &metadata, std::stringstream& propertiesStruct) {
    static bool in_properties_block = false;
    std::string trimmed = Trim(line);
    if (trimmed.contains("struct MaterialProperties")) {
        in_properties_block = true;
        propertiesStruct << line;
        return in_properties_block;
    }

    
    if (in_properties_block) {
        propertiesStruct << line;

        if (trimmed == "}") {
            in_properties_block = false;
            return false;
        }
        
        // Parse property line
        size_t colon_pos = trimmed.find(':');
        if (colon_pos == std::string::npos) {
            std::cerr << "Invalid property syntax: " << trimmed << "\n";
            return in_properties_block;
        }

        MaterialProperty prop;
        std::string name = Trim(trimmed.substr(0, colon_pos));
        std::string rest = Trim(trimmed.substr(colon_pos + 1));

        // Split type and default value
        size_t equals_pos = rest.find('=');
        std::string type_str = Trim(rest.substr(0, equals_pos));
        std::string default_str = (equals_pos != std::string::npos) ? 
            Trim(rest.substr(equals_pos + 1)) : "";

        // Remove trailing comma if present
        if (!type_str.empty() && type_str.back() == ',') {
            type_str.pop_back();  // C++11 and later
            type_str = Trim(type_str);  // Re-trim in case comma had whitespace
        }

        // Map WGSL types to engine types
        static const std::unordered_map<std::string, MaterialPropertyType> type_map = {
            {"f32", MaterialPropertyType::Float},
            {"i32", MaterialPropertyType::Int},
            {"u32", MaterialPropertyType::UInt},
            {"vec2f", MaterialPropertyType::Vector2},
            {"vec3f", MaterialPropertyType::Vector3},
            {"vec4f", MaterialPropertyType::Vector4},
            {"texture_2d", MaterialPropertyType::Texture2D},
            {"texture_cube", MaterialPropertyType::TextureCube},
        };

        auto type_it = type_map.find(ToLower(type_str));
        if (type_it == type_map.end()) {
            std::cerr << "Unknown property type: " << type_str << "\n";
            return in_properties_block;
        }
        prop.Type = type_it->second;
        prop.WGSLTypeRaw = type_str;

        // Parse default value
        try {
            switch (prop.Type) {
                case MaterialPropertyType::Float:
                    prop.DefaultValue = default_str.empty() ? 0.0f : std::stof(default_str);
                break;
                case MaterialPropertyType::Int:
                    prop.DefaultValue = default_str.empty() ? 0 : std::stoi(default_str);
                break;
                case MaterialPropertyType::Vector3: {
                    // Expect format: vec3f(1.0, 0.5, 0.0)
                    if (!default_str.empty()) {
                        size_t start = default_str.find('(');
                        size_t end = default_str.find(')');
                        std::string contents = default_str.substr(start + 1, end - start - 1);
                        std::istringstream ss(contents);
                        Vector3 vec;
                        char comma;
                        ss >> vec.x >> comma >> vec.y >> comma >> vec.z;
                        prop.DefaultValue = vec;
                    }
                    break;
                }
                case MaterialPropertyType::Texture2D:
                    // Textures don't have default values in this system
                        break;
                // Add other types as needed
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing default value for " << name
                     << ": " << e.what() << "\n";
        }

        metadata.Properties[name] = prop;
        return in_properties_block;
    }

    return false;
}

void ShaderParser::ProcessFunctionLine(const std::string& scopeName, int& counter, const std::string& line, std::stringstream& output)
{
    std::string trimmed = Trim(line);
    if (line.contains("fn " + scopeName)) {
        counter++;
        return;
    }
    
    if (counter > 0 )
    {
        if (trimmed == "{")
        {
            counter++;
        }
        else if (trimmed == "}")
        {
            counter--;
            if (counter == 0) return;
        }

        output << line << "\n";
    }
}

void ShaderParser::ProcessBaseShaderTypeLine(const std::string& line, std::string& output)
{
    std::string trimmed = Trim(line);
    const std::string prefix = "#implements ";
    const char suffix = ';';

    if (line.starts_with(prefix) && line.ends_with(suffix))
    {
        size_t start_pos = prefix.length();
        size_t end_pos = line.length() - 1; // Position before the semicolon
        if (start_pos < end_pos) {
            output = line.substr(start_pos, end_pos - start_pos);
        }
    }
}
