#pragma once
#include <array>
#include <webgpu/webgpu.hpp>

#include "CrabTypes.h"
#include "MaterialHelpers.h"

namespace Uniforms
{
    struct UCameraData
    {
        Matrix4 viewMatrix = Matrix4(1.0f);
        Matrix4 projectionMatrix = Matrix4(1.0f);
        Vector3 cameraPosition = {0, 0, 0};
        float _pad[1];
    };

    struct ULightingData
    {
        std::array<Vector4, 2> LightDirections;
        std::array<Vector4, 2> LightColors;
    };

    struct UObjectData
    {
        Matrix4 ModelMatrix = Matrix4(1.0f);
        Matrix4 LocalMatrix = Matrix4(1.0f);
    };

    using PerObjectUniformsLayout = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // ObjectData

    struct UGlobalData
    {
        float Time = 0.0f;
        float _pad[3];
    };

    using GlobalUniformsLayout
    = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>
    >; // GlobalData


    using RendererUniformsLayout
    = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // CameraData
    MaterialHelpers::UniformBufferEntry<1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // LightingData
    
    struct UStandardMaterialParameters
    {
        float Hardness = 32.f;
        // Diffuse Contribution
        float Kd = 1.f;
        // Specular Contribution
        float Ks = 0.5f;
        float NormalStrength = 0.f;
    };


    

}
