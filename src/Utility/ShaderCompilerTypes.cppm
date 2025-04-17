//
// Created by Vinnie on 11/04/2025.
//

export module Engine.ShaderCompiler.Types;
import Engine.WGPU;

export struct BindingLayouts {
    wgpu::raii::PipelineLayout PipelineLayout;
    std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> BindGroupLayouts;
};

export struct UniformMetadata {
    struct BufferField {
        enum FieldType {
            UNDEFINED,
            BOOL,
            INT,
            UINT,
            INT8,
            UINT8,
            INT64,
            UINT64,
            INT16,
            UINT16,
            FLOAT,
            FLOAT16,
            FLOAT64,
            VECTOR2,
            VECTOR3,
            VECTOR4,
            VECTOR2I,
            VECTOR3I,
            VECTOR4I,
            STRUCT
        };

        std::string Name = "Field";
        uint32_t SizeInBytes = 0;
        uint32_t Offset = 0;
        FieldType Type = UNDEFINED;
        std::vector<BufferField> Subfields;
    };

    std::string Name = "Binding";
    uint8_t Group = 0;
    uint8_t Location = 0;
    uint32_t SizeInBytes = 0;
    bool IsSerialized = false;
    WGPUShaderStage Visibility = wgpu::ShaderStage::None;
    WGPUBindingType BindingType = Undefined;
    WGPUTextureViewDimension Dimension = WGPUTextureViewDimension_Undefined; // For textures
    WGPUTextureFormat Format = WGPUTextureFormat_Undefined; // For textures
    WGPUTextureSampleType SampleType = WGPUTextureSampleType_Undefined; // For textures
    WGPUStorageTextureAccess StorageTextureAccess = WGPUStorageTextureAccess_Undefined;
    WGPUSamplerBindingType SamplerBindingType = WGPUSamplerBindingType_Undefined; // For Samplers
    std::vector<BufferField> BufferFields{}; // For uniform buffer reflection
    bool IsPushConstant = false;
    bool IsDynamic = false;
};
