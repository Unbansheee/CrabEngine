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
    std::string Name = "Binding";
    uint8_t Group = 0;
    uint8_t Location = 0;
    uint32_t SizeInBytes = 0;
    WGPUShaderStage Visibility = wgpu::ShaderStage::None;
    WGPUBindingType BindingType = Undefined;
    WGPUTextureViewDimension Dimension = WGPUTextureViewDimension_Undefined; // For textures
    WGPUTextureFormat Format = WGPUTextureFormat_Undefined; // For textures
    WGPUTextureSampleType SampleType = WGPUTextureSampleType_Undefined; // For textures
    bool IsPushConstant = false;
    bool IsDynamic = false;
};
