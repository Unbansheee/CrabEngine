//
// Created by Vinnie on 23/02/2025.
//

#pragma once
#include <memory>
#include <string>

#include "Resource/Mesh.h"
//#include "Texture.h"
#include "tiny_gltf.h"

class Material;
class TextureResource;
class Node3D;

class GLTFSceneParser
{
public:
    std::unique_ptr<Node3D> ParseGLTF(WGPUDevice device, const std::string& path);

private:
    std::shared_ptr<Mesh> ParseMesh(WGPUDevice device, tinygltf::Model& model, tinygltf::Mesh& mesh);
    std::shared_ptr<Material> ParseMaterial(WGPUDevice& context, tinygltf::Model& model, const std::vector<std::shared_ptr<TextureResource>>& textures, tinygltf::Material& material);
    std::shared_ptr<TextureResource> ParseTexture(WGPUDevice& context, tinygltf::Model& model,
    tinygltf::Texture& texture);

};