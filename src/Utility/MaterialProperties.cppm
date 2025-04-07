
export module Engine.MaterialProperties;
import std;
import Engine.WGPU;

export enum class MaterialPropertyType {
    Float,
    Int,
    UInt,
    Vector2,
    Vector3,
    Vector4,
    Matrix4x4,
    Texture2D,
    TextureCube
};

export struct MaterialProperty {
    MaterialPropertyType Type = MaterialPropertyType::Float;
    std::string WGSLTypeRaw;
    std::any Value;
    std::any DefaultValue;
};

export struct ShaderMetadata {
    std::unordered_map<std::string, MaterialProperty> Properties{};
    std::string vertexFn;
    std::string fragmentFn;
    std::string lightFn;
};

export struct PropertyLayoutInfo {
    size_t Offset;
    size_t Size;
};

// Helper to map engine types to WGSL alignment
export struct WGSLAligner {
    static size_t AlignOf(MaterialPropertyType type) {
        switch(type) {
        case MaterialPropertyType::Float: return 4;
        case MaterialPropertyType::Int: return 4;
        case MaterialPropertyType::UInt: return 4;
        case MaterialPropertyType::Vector2: return 8;
        case MaterialPropertyType::Vector3: return 16;
        case MaterialPropertyType::Vector4: return 16;
        case MaterialPropertyType::Matrix4x4: return 16;
        default: return 1;
        }
    }

    static size_t SizeOf(MaterialPropertyType type) {
        switch(type) {
        case MaterialPropertyType::Float: return 4;
        case MaterialPropertyType::Int: return 4;
        case MaterialPropertyType::UInt: return 4;
        case MaterialPropertyType::Vector2: return 8;
        case MaterialPropertyType::Vector3: return 12;
        case MaterialPropertyType::Vector4: return 16;
        case MaterialPropertyType::Matrix4x4: return 64;
        default: return 0;
        }
    }
};
