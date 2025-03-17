module;
#pragma once
#include <array>
//#include <webgpu/webgpu.hpp>

#include "MaterialHelpers.h"

export module uniform_definitions;
export import crab_types;
import wgpu;

namespace Uniforms
{
    export struct UCameraData
    {
        Matrix4 viewMatrix = Matrix4(1.0f);
        Matrix4 projectionMatrix = Matrix4(1.0f);
        Vector3 cameraPosition = {0, 0, 0};
        float _pad[1];
    };

    export struct ULightingData
    {
        std::array<Vector4, 2> LightDirections;
        std::array<Vector4, 2> LightColors;
    };

    export struct UObjectData
    {
        Matrix4 ModelMatrix = Matrix4(1.0f);
        Matrix4 LocalMatrix = Matrix4(1.0f);
    };

    export using PerObjectUniformsLayout = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::DynamicOffsetUniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // ObjectData

    export struct UGlobalData
    {
        float Time = 0.0f;
        float _pad[3];
    };

    export using GlobalUniformsLayout
    = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>
    >; // GlobalData


    export using RendererUniformsLayout
    = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // CameraData
    MaterialHelpers::UniformBufferEntry<1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // LightingData
    
    export struct UStandardMaterialParameters
    {
        float Hardness = 32.f;
        // Diffuse Contribution
        float Kd = 1.f;
        // Specular Contribution
        float Ks = 0.5f;
        float NormalStrength = 0.f;
    };
}
