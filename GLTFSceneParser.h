//
// Created by Vinnie on 23/02/2025.
//

#pragma once
#include <memory>
#include <string>

#include "Mesh.h"
//#include "Texture.h"
#include "tiny_gltf.h"

class Node3D;

class GLTFSceneParser
{
public:
    std::unique_ptr<Node3D> ParseGLTF(WGPUDevice device, const std::string& path);

private:
    std::shared_ptr<Mesh> ParseMesh(WGPUDevice device, tinygltf::Model& model, tinygltf::Mesh& mesh);
    //std::shared_ptr<Material> ParseMaterial(Context& context, tinygltf::Model& model, const std::vector<std::shared_ptr<Texture>>& textures, tinygltf::Material& material);
    //std::shared_ptr<Texture> ParseTexture(Context& context, tinygltf::Model& model, tinygltf::Texture& material);

};