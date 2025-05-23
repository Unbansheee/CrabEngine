﻿module;
#pragma once
#include <array>
//#include <webgpu/webgpu.hpp>
#include "ReflectionMacros.h"
#include "MaterialHelpers.h"

export module Engine.GFX.UniformDefinitions;
export import Engine.Types;
import Engine.WGPU;

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
        struct DirectionalLight {
            Vector4 Direction;
            Vector4 Color;
        };

        std::array<DirectionalLight, 4> DirectionalLights;
        int DirectionalLightCount;
    };

    export struct UObjectData
    {
        Matrix4 ModelMatrix = Matrix4(1.0f);
        Matrix4 LocalMatrix = Matrix4(1.0f);

        // 0 is invalid. Valid ID's start at 1
        uint32_t DrawID = 0;
        float _pad[3];
    };

    
    export struct UIDPassObjectData
    {
        Matrix4 ModelMatrix = Matrix4(1.0f);
        Matrix4 LocalMatrix = Matrix4(1.0f);
        uint32_t DrawID = 0;
        float _pad[3];
    };


    export using PerObjectUniformsLayout = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // ObjectData


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
    MaterialHelpers::UniformBufferEntry<1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>, // LightingData
    MaterialHelpers::TextureStorageEntry<2, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment, WGPUTextureFormat_R32Uint>>; // ID Texture

    export using IDRendererUniformsLayout
    = MaterialHelpers::BindGroupLayoutBuilder<
    MaterialHelpers::UniformBufferEntry<0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>>; // CameraData
    
    export struct UStandardMaterialParameters
    {
        BEGIN_STRUCT_PROPERTIES(UStandardMaterialParameters)
            ADD_STRUCT_PROPERTY_FLAGS("Base Color Factor", BaseColorFactor, PropertyFlags::MaterialProperty | PropertyFlags::ColorHint)
            ADD_STRUCT_PROPERTY_FLAGS("Hardness", Hardness, PropertyFlags::MaterialProperty)
            ADD_STRUCT_PROPERTY_FLAGS("Diffuse Intensity", Kd, PropertyFlags::MaterialProperty)
            ADD_STRUCT_PROPERTY_FLAGS("Specular Intensity", Ks, PropertyFlags::MaterialProperty)
            ADD_STRUCT_PROPERTY_FLAGS("Normal Strength", NormalStrength, PropertyFlags::MaterialProperty)
        END_STRUCT_PROPERTIES

        // Tint
        Vector3 BaseColorFactor = {1,1,1};

        float Hardness = 32.f;
        // Diffuse Contribution
        float Kd = 1.f;
        // Specular Contribution
        float Ks = 0.5f;
        float NormalStrength = 0.f;
    };
}
