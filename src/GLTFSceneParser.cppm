//
// Created by Vinnie on 23/02/2025.
//
module;

#pragma once
#include <memory>
#include <string>
//#include "webgpu/webgpu.hpp"
#include "tiny_gltf.h"


export module gltf_scene_parser;
import mesh_resource;
import material;
import standard_material;
import texture_resource;
import node_3d;
import wgpu;

export class GLTFSceneParser
{
public:
    std::unique_ptr<Node3D> ParseGLTF(WGPUDevice device, const std::string& path);

private:
    std::shared_ptr<MeshResource> ParseMesh(WGPUDevice device, tinygltf::Model& model, tinygltf::Mesh& mesh);
    std::shared_ptr<MaterialResource> ParseMaterial(WGPUDevice& context, tinygltf::Model& model, const std::vector<std::shared_ptr<TextureResource>>& textures, tinygltf::Material& material);
    std::shared_ptr<TextureResource> ParseTexture(WGPUDevice& context, tinygltf::Model& model,
    tinygltf::Texture& texture);

};