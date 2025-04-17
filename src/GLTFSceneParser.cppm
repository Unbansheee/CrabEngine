//
// Created by Vinnie on 23/02/2025.
//
module;

#pragma once
#include <memory>
#include <string>
//#include "webgpu/webgpu.hpp"
#include "tiny_gltf.h"


export module Engine.GLTFSceneParser;
import Engine.Resource.Mesh;
import Engine.Resource.Material;
import Engine.Resource.Texture;
import Engine.Node.Node3D;
import Engine.WGPU;

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