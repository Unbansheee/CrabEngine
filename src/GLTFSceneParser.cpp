//
// Created by Vinnie on 23/02/2025.
//
module;
#include <tiny_gltf.h>

module Engine.GLTFSceneParser;

//#include "NodeMesh3D.h"

import Engine.Resource.ArrayMesh;
import Engine.Resource.ResourceManager;
import Engine.Resource.RuntimeTexture;
import std;
import Engine.Node.MeshInstance3D;
import Engine.Resource.ArrayMesh;
import Engine.GFX.MeshVertex;
import Engine.GFX.UniformDefinitions;
import Engine.Resource.ShaderFile;


/*
#include "MaterialStandard.h"
#include "NodeBoxShape3D.h"
#include "NodeCollisionShape3D.h"
#include "NodeConvexShape3D.h"
#include "NodeRigidBody3D.h"
#include "NodeSphereShape3D.h"
#include "NodeStaticBody3D.h"
#include "NodeTrimeshShape3D.h"
*/

std::unique_ptr<Node3D> GLTFSceneParser::ParseGLTF(WGPUDevice device, const std::string& path)
{
	std::unique_ptr<Node3D> root = Node::NewNode<Node3D>();
	root->SetName(path.substr(path.find_last_of('/') + 1).substr( 0, path.find_last_of('.')));

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	// Load binary GLTF
	bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
		return nullptr;
	}

	std::vector<std::shared_ptr<TextureResource>> parsed_textures;
	std::vector<std::shared_ptr<MeshResource>> parsed_meshes;
	std::vector<std::shared_ptr<MaterialResource>> parsed_materials;
	std::vector<Node*> parsed_nodes;

	
	// parse textures
	for (tinygltf::Texture& texture : model.textures)
	{
		parsed_textures.push_back(ParseTexture(device, model, texture));
	}
	

	// parse meshes
	for (auto& mesh : model.meshes)
	{
		parsed_meshes.push_back(ParseMesh(device, model, mesh));
	}

	
	// parse materials
	for (auto& material : model.materials)
	{
		parsed_materials.push_back(ParseMaterial(device, model, parsed_textures, material));
	}
	

	// Parse node tree
	//SharedRef<StandardMaterial> mat = MakeShared<StandardMaterial>(device, ENGINE_RESOURCE_DIR"/standard_material.wgsl");
	//mat->TargetTextureFormat = wgpu::TextureFormat::BGRA8UnormSrgb;
	//mat->Initialize();
	for (tinygltf::Node& node : model.nodes)
	{
		std::unique_ptr<Node3D> createdNode = nullptr;
		if (node.mesh >= 0) {
			//if (node.name.find(" -colonly") == std::string::npos)
			//{
			// Create mesh node
			auto m = Node::NewNode<NodeMeshInstance3D>();
			m->SetMesh(parsed_meshes.at(node.mesh));

			// Does the mesh have primitives that have a material assigned?
			tinygltf::Mesh& meshdata = model.meshes.at(node.mesh);
			tinygltf::Primitive& prim = meshdata.primitives.front();
			if (prim.material >= 0)
			{
				m->SetMaterial(parsed_materials.at(prim.material));
				//m->SetMaterial(mat);
			}

			createdNode = std::move(m);
		}
			//}

			/*
			// Make a rigidBody outer for the mesh node
			if (node.name.find(" -rigid") != std::string::npos)
			{
				NodeRigidBody3D* rb = new NodeRigidBody3D();
				if (createdNode)
					rb->AddChild(std::move(createdNode), true);
				createdNode.reset(rb);
			}
			// Make a StaticBody outer for the mesh node
			else if (node.name.find(" -static") != std::string::npos)
			{
				NodeStaticBody3D* sb = new NodeStaticBody3D();
				if (createdNode)
					sb->AddChild(std::move(createdNode), true);
				createdNode.reset(sb);
			}

			// Create a trimesh child shape
			if (node.name.find(" -tri") != std::string::npos)
			{
				if (createdNode)
				{
					NodeTrimeshShape3D* shape = createdNode->AddChild<NodeTrimeshShape3D>();
					shape->SetMesh(parsed_meshes.at(node.mesh));
				}
				else
				{
					NodeTrimeshShape3D* shape = new NodeTrimeshShape3D();
					shape->SetMesh(parsed_meshes.at(node.mesh));
					createdNode.reset(shape);
				}
			}

			// Create a convex child shape
			if (node.name.find(" -conv") != std::string::npos)
			{
				if (createdNode)
				{
					NodeConvexShape3D* shape = createdNode->AddChild<NodeConvexShape3D>();
					shape->SetMesh(parsed_meshes.at(node.mesh));
				}
				else
				{
					NodeConvexShape3D* shape = new NodeConvexShape3D();
					shape->SetMesh(parsed_meshes.at(node.mesh));
					createdNode.reset(shape);
				}
			}
		}
		// Make a rigidBody outer for the mesh node
		else if (node.name.find(" -rigid") != std::string::npos)
		{
			NodeRigidBody3D* rb = new NodeRigidBody3D();
			createdNode.reset(rb);
		}
		// Make a StaticBody outer for the mesh node
		else if (node.name.find(" -col") != std::string::npos)
		{
			NodeStaticBody3D* sb = new NodeStaticBody3D();
			createdNode.reset(sb);
		}
		// Import Box Collision
		else if (node.name.find(" -box") != std::string::npos)
		{
			NodeBoxShape3D* shape = new NodeBoxShape3D();
			createdNode.reset(shape);
		}

		// Import Sphere Collision
		else if (node.name.find(" -sphere") != std::string::npos)
		{
			NodeSphereShape3D* shape = new NodeSphereShape3D();
			createdNode.reset(shape);
		}
		*/

		// Empty Transform Node

		else
		{
			createdNode = Node::NewNode<Node3D>();
		}



		// Import Position
		if (node.translation.size() == 3)
			createdNode->SetPosition({ (float)node.translation.at(0), (float)node.translation.at(2), (float)node.translation.at(1) });


		// Import Scale
		if (node.scale.size() == 3)
			createdNode->SetScale({ (float)node.scale.at(0), -(float)node.scale.at(2), (float)node.scale.at(1) });

		// Import Rotation
		if (node.rotation.size() == 4) {
			Quat baseOrientation = {-(float)node.rotation.at(3), (float)node.rotation.at(0), (float)node.rotation.at(1), (float)node.rotation.at(2)};


			//Quat composed = rotated1 * rotated2;
			//Quat rotated = glm::rotate(baseOrientation, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			//rotated = glm::rotate(rotated, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			createdNode->SetOrientation({ baseOrientation });
		}

		createdNode->SetName(node.name);
		parsed_nodes.push_back(createdNode.get());
		root->AddChild(std::move(createdNode));
	}


	// Parse node relationships
	for (int i = 0; i < parsed_nodes.size(); i++)
	{
		Node* currentNode = parsed_nodes.at(i);
		tinygltf::Node gltfNode = model.nodes.at(i);

		for (int child : gltfNode.children)
		{
			std::unique_ptr<Node> childMapping;
			if (std::unique_ptr<Node> m = parsed_nodes.at(child)->RemoveFromParent())
			{
				childMapping = std::move(m);
			}
			else
			{
				childMapping.reset(parsed_nodes.at(child));
			}

			currentNode->AddChild(std::move(childMapping));
		}
	}


	return root;
}

std::shared_ptr<MeshResource> GLTFSceneParser::ParseMesh(WGPUDevice device, tinygltf::Model& model, tinygltf::Mesh& mesh)
{
	std::vector<MeshVertex> vertices;
	std::vector<uint16_t> indices;

	for (tinygltf::Primitive& primitive : mesh.primitives)
	{
		// Positions buffer
		const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes["POSITION"]];
		const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];

		const tinygltf::Buffer& posBuffer = model.buffers[positionBufferView.buffer];
		const float* positions = reinterpret_cast<const float*>(&posBuffer.data[positionBufferView.byteOffset + positionAccessor.byteOffset]);

		// UVS buffer
		const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
		const tinygltf::BufferView& uvBufferView = model.bufferViews[uvAccessor.bufferView];

		const tinygltf::Buffer& uvBuffer = model.buffers[uvBufferView.buffer];
		const float* uvs = reinterpret_cast<const float*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);

		// Normals buffer
		const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes["NORMAL"]];
		const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];

		const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];
		const float* normals = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);

		// Vertex Colors buffer
		bool vColorExists = primitive.attributes.find("COLOR_0") != primitive.attributes.end();

		const tinygltf::Accessor& vertexColorAccessor = model.accessors[primitive.attributes["COLOR_0"]];
		const tinygltf::BufferView& vertexColorBufferView = model.bufferViews[vertexColorAccessor.bufferView];

		const tinygltf::Buffer& vertexColorBuffer = model.buffers[vertexColorBufferView.buffer];

		const float* vertexColors = reinterpret_cast<const float*>(&vertexColorBuffer.data[vertexColorBufferView.byteOffset + vertexColorAccessor.byteOffset]);

		vertices.reserve(positionAccessor.count);

		for (size_t i = 0; i < positionAccessor.count; ++i) {

			MeshVertex v{};

			// swizzled to enforce Z up
			v.position.x = positions[i * 3 + 0];
			v.position.y = -positions[i * 3 + 2];
			v.position.z = positions[i * 3 + 1];

			v.uv.x = uvs[i * 2 + 0];
			v.uv.y = uvs[i * 2 + 1];

			if (vColorExists)
			{
				v.color.r = float(vertexColors[i * 3 + 0]);
				v.color.g = float(vertexColors[i * 3 + 1]);
				v.color.b = float(vertexColors[i * 3 + 2]);
			}
			else
			{
				v.color.r = 1.0f;
				v.color.g = 1.0f;
				v.color.b = 1.0f;
			}

			// swizzled to enforce Z up
			v.normal.x = normals[i * 3 + 0];
			v.normal.y = -normals[i * 3 + 2];
			v.normal.z = normals[i * 3 + 1];

			vertices.push_back(v);
		}

		// Indices buffer
		const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
		const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
		const tinygltf::Buffer& indBuffer = model.buffers[indexBufferView.buffer];
		const uint16_t* indexBuf = reinterpret_cast<const uint16_t*>(&indBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);

		indices.reserve(indexAccessor.count);

		for (size_t i = 0; i < indexAccessor.count; ++i) {
			indices.push_back(indexBuf[i]);
		};
	}

	ResourceManager::populateTextureFrameAttributes(vertices, indices);

	auto m = MakeShared<ArrayMeshResource>();
	m->indices = std::move(indices);
	m->vertices = std::move(vertices);
	m->bIsRuntime = true;
	m->LoadData();
	return m;
}


std::shared_ptr<MaterialResource> GLTFSceneParser::ParseMaterial(WGPUDevice& context, tinygltf::Model& model, const std::vector<std::shared_ptr<TextureResource>>& textures, tinygltf::Material& material)
{
	std::shared_ptr<MaterialResource> mat = std::make_shared<MaterialResource>();
	mat->shader_file = ResourceManager::Load<ShaderFileResource>(ENGINE_RESOURCE_DIR"/standard_material.wgsl");
	
	auto& bcf = material.pbrMetallicRoughness.baseColorFactor;
	//mat->params.ColourFactor = Vector4( bcf.at(0), bcf.at(1), bcf.at(2), bcf.at(3) );
	//mat->params.MetallicFactor = material.pbrMetallicRoughness.metallicFactor;
	//mat->params.RoughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
	Uniforms::UStandardMaterialParameters params;
	params.BaseColorFactor = {bcf[0], bcf[1], bcf[2]};
	//mat->MaterialParameters.SetData();

	if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
	{
		//mat->TBaseColour = textures.at(material.pbrMetallicRoughness.baseColorTexture.index);
		//mat->BaseColorTextureView = textures.at(material.pbrMetallicRoughness.baseColorTexture.index);
	}
	if (material.normalTexture.index >= 0)
	{
		//mat->NormalTextureView = textures.at(material.normalTexture.index);
		//mat->TMetallicRoughness = textures.at(material.pbrMetallicRoughness.metallicRoughnessTexture.index);
	}

	mat->LoadData();

	return mat;
}


std::shared_ptr<TextureResource> GLTFSceneParser::ParseTexture(WGPUDevice& context, tinygltf::Model& model,
	tinygltf::Texture& texture)
{
	tinygltf::Image& imageRef = model.images.at(texture.source);

	std::shared_ptr<RuntimeTextureResource> res = MakeShared<RuntimeTextureResource>();
	res->LoadFromPixelData(imageRef.width, imageRef.height, imageRef.bits, imageRef.image.data());

	return res;
	/*
	sce::Gnm::TextureSpec spec;
	spec.init();
	spec.m_textureType = sce::Gnm::kTextureType2d;
	spec.m_width = imageRef.width;
	spec.m_height = imageRef.height;
	spec.m_depth = 1;
	spec.m_pitch = 0;
	spec.m_numMipLevels = 1;
	spec.m_numSlices = 1;
*/
/*
	if (imageRef.bits == 8)
	{
		spec.m_format = sce::Gnm::kDataFormatR8G8B8A8UnormSrgb;
	}
	else if (imageRef.bits == 16)
	{
		spec.m_format = sce::Gnm::kDataFormatR16G16B16A16Unorm;
	}

	spec.m_tileModeHint = sce::Gnm::kTileModeDisplay_LinearAligned;
	spec.m_minGpuMode = sce::Gnm::kGpuModeBase;
	spec.m_numFragments = sce::Gnm::kNumFragments1;
*/
	//std::shared_ptr<TextureResource> tex = std::make_shared<TextureResource>(context, imageRef.image, spec);
	
}
