#include "pch.h"
#include "ModelImporter.h"
#include "GLTFInclude.h"
#include "GraphicsDevice.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "TextParser.h"
#include <assert.h>
#include <unordered_set>

namespace gfx
{
	ModelImporter::ModelImportSettings ModelImporter::GetImportSettings(const GraphicsDevice& gfx, const char * assetFilename)
	{
		TextParser parser(assetFilename);
		TextParser::ParsedKeyValues p;

		// Parse asset
		ModelImportSettings settings;
		settings.gltfIsBinary = true;

		bool hasPath = false;
		bool hasName = false;
		while (parser.ReadParsedLine(p))
		{
			if (p.key == "Name")
			{
				settings.name = p.values[0];
				hasName = true;
			}
			else if (p.key == "GLTF")
			{
				settings.gltfPath = p.values[0];
				hasPath = true;
			}
			else if (p.key == "Map")
			{
				//gfx.GetLog()->Info(p.values[0]);
				settings.materialPaths.emplace_back(p.values[1]);
				settings.materialIndicesByName[p.values[0]] = (int)settings.materialPaths.size() - 1;
			}
		}
		parser.Release();

		if (!hasName)
		{
			THROW("Model asset '" + std::string(assetFilename) + "' does not have a name property!");
		}
		if (!hasPath)
		{
			THROW("Model asset '" + std::string(assetFilename) + "' does not have a GLTF path!");
		}

		return settings;
	}

	size_t ModelImporter::GetAttributeCt(const GraphicsDevice& gfx, tinygltf::Model& model, int accessorIdx)
	{
		const auto accessor = model.accessors[accessorIdx];
		return accessor.count;
	}

	std::pair<int, size_t> ModelImporter::GetAttributeBufferAccess(const GraphicsDevice& gfx, tinygltf::Model& model, int accessorIdx, size_t byteCt)
	{
		// Each attribute owns a segment of data in the buffer
		// It can be accessed by using the view and accessor offsets
		const auto& accessor = model.accessors[accessorIdx];
		const auto& view = model.bufferViews[accessor.bufferView];

		assert(view.byteLength == byteCt && "Buffer view does not match expected number of elements");

		return std::pair<int, size_t>(view.buffer, (size_t)(view.byteOffset + accessor.byteOffset));
	}

	std::shared_ptr<ModelAsset> ModelImporter::LoadGLTF(const GraphicsDevice& gfx, const char * assetFilename)
	{
		Model model;
		TinyGLTF loader;
		std::string err;
		std::string warn;

		gfx.GetLog().Info(std::string("LoadGLTF(") + std::string(assetFilename) + std::string(")"));

		const auto settings = GetImportSettings(gfx, assetFilename);

		bool ret;
		if (settings.gltfIsBinary)
		{
			ret = loader.LoadBinaryFromFile(&model, &err, &warn, settings.gltfPath); // for binary glTF(.glb)
		}
		else
		{
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, settings.gltfPath);
		}

		if (!warn.empty())
		{
			THROW("glTF warning: '" + warn + "'");
		}

		if (!err.empty())
		{
			THROW("glTF error: '" + err + "'");
		}

		if (!ret)
		{
			THROW("Could not parse glTF for filename '" + settings.gltfPath + "'");
			return nullptr;
		}

		if (model.scenes.size() <= model.defaultScene || model.defaultScene < 0)
		{
			THROW("No default scene for filename '" + settings.gltfPath + "'");
			return nullptr;
		}

		// Good reference:
		// https://www.khronos.org/files/gltf20-reference-guide.pdf

		// scene.nodes = array of all scene graph nodes
		const auto scene = model.scenes[model.defaultScene];
		const size_t nodeCt = model.nodes.size();

		// Get total mesh count, including submeshes
		size_t meshCt = 0u;
		for (const auto& mesh : model.meshes)
		{
			meshCt += mesh.primitives.size();
		}

		assert(nodeCt >= meshCt && "Not enough nodes for mesh count");
		gfx.GetLog().Info(std::string("GLTF has ") + std::to_string(nodeCt) + std::string(" nodes, ") + std::to_string(meshCt) + std::string(" meshes"));

		// Create meshes
		std::vector<std::shared_ptr<MeshAsset>> pMeshes;
		pMeshes.reserve(meshCt);
		for (const auto& mesh : model.meshes)
		{
			// Each primitive is a submesh
			u32 subNameCt = 0u;
			for (const auto& primitive : mesh.primitives)
			{
				// Reference:
				// https://github.com/syoyo/tinygltf/wiki/Accessing-vertex-data

				pMeshes.emplace_back(std::make_shared<MeshAsset>());
				const auto& pCurrentMeshAsset = pMeshes.at(pMeshes.size() - 1u);
				pCurrentMeshAsset->m_name = settings.name + "|" + mesh.name + "(sub=" + std::to_string(subNameCt++) + ")";

				assert(primitive.attributes.count("POSITION") == 1 && "Mesh primitive has no POSITION attribute!");

				// Get vert count from POSITION attribute
				const auto vertCt = GetAttributeCt(gfx, model, primitive.attributes.at("POSITION"));
				assert(vertCt > 0 && "Mesh primitive has 0 vertices!");

				// Load INDICES
				{
					const int accessorIdx = primitive.indices;
					const auto accessor = model.accessors[accessorIdx];
					const auto& view = model.bufferViews[accessor.bufferView];

					const size_t indicesCt = accessor.count;
					assert(indicesCt > 0 && "Mesh primitive has 0 indices!");

					// Determine index size
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, indicesCt * sizeof(u16));
						const u16* indices = reinterpret_cast<const u16*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
						for (size_t vi = 0; vi < indicesCt; ++vi)
						{
							pCurrentMeshAsset->m_indices.emplace_back((u32)indices[vi]);
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					{
						const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, indicesCt * sizeof(u32));
						const u32* indices = reinterpret_cast<const u32*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
						for (size_t vi = 0; vi < indicesCt; ++vi)
						{
							pCurrentMeshAsset->m_indices.emplace_back((u32)indices[vi]);
						}
					}
					else
					{
						THROW("Mesh had unrecognized componentType for indices: " + std::to_string(accessor.componentType));
						return nullptr;
					}
				}

				// Load POSITION
				{
					pCurrentMeshAsset->m_vertices.reserve(vertCt);

					const int accessorIdx = primitive.attributes.at("POSITION");
					const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, vertCt * sizeof(float) * 3u);
					const float* positions = reinterpret_cast<const float*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
					for (size_t vi = 0; vi < vertCt; ++vi)
					{
						const auto posF3 = dx::XMFLOAT3(positions[vi * 3 + 0], positions[vi * 3 + 1], positions[vi * 3 + 2]);
						pCurrentMeshAsset->m_vertices.emplace_back(posF3);
					}
				}

				// Load NORMAL
				if (primitive.attributes.count("NORMAL"))
				{
					pCurrentMeshAsset->hasNormals = true;
					pCurrentMeshAsset->m_normals.reserve(vertCt);

					const int accessorIdx = primitive.attributes.at("NORMAL");
					const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, vertCt * sizeof(float) * 3u);
					const float* normals = reinterpret_cast<const float*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
					for (size_t vi = 0; vi < vertCt; ++vi)
					{
						pCurrentMeshAsset->m_normals.emplace_back(dx::XMFLOAT3(normals[vi * 3 + 0], normals[vi * 3 + 1], normals[vi * 3 + 2]));
					}
				}

				// Load TANGENT
				if (primitive.attributes.count("TANGENT"))
				{
					pCurrentMeshAsset->hasTangents = true;
					pCurrentMeshAsset->m_tangents.reserve(vertCt);

					const int accessorIdx = primitive.attributes.at("TANGENT");
					const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, vertCt * sizeof(float) * 4u);
					const float* tangents = reinterpret_cast<const float*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
					for (size_t vi = 0; vi < vertCt; ++vi)
					{
						pCurrentMeshAsset->m_tangents.emplace_back(dx::XMFLOAT4(tangents[vi * 4 + 0], tangents[vi * 4 + 1], tangents[vi * 4 + 2], tangents[vi * 4 + 3]));
					}
				}

				// Detect up to 4 texcoords
				std::vector<std::string> texcoordStrings;
				for (size_t ti = 0; ti < 4; ++ti)
				{
					auto key = std::string("TEXCOORD_") + std::to_string(ti);
					if (primitive.attributes.count(key))
					{
						texcoordStrings.emplace_back(std::move(key));
					}
				}

				pCurrentMeshAsset->m_texcoords.reserve(texcoordStrings.size());

				// Load TEXCOORD_0
				for (size_t ti = 0; ti < texcoordStrings.size(); ++ti)
				{
					std::vector<dx::XMFLOAT2> texcoordSet;
					texcoordSet.reserve(vertCt);

					const int accessorIdx = primitive.attributes.at(texcoordStrings[ti]);
					const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, vertCt * sizeof(float) * 2u);
					const float* coords = reinterpret_cast<const float*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
					for (size_t vi = 0; vi < vertCt; ++vi)
					{
						texcoordSet.emplace_back(dx::XMFLOAT2(coords[vi * 2 + 0], coords[vi * 2 + 1]));
					}

					pCurrentMeshAsset->m_texcoords.emplace_back(std::move(texcoordSet));
				}

				// Calculate AABB
				pCurrentMeshAsset->m_aabb.SetBoundsByVertices(pCurrentMeshAsset->m_vertices);

				// Double check that everything was loaded correctly
				assert(pCurrentMeshAsset->m_vertices.size() > 0 && "Mesh primitive vertices were incorrectly loaded!");
				assert(pCurrentMeshAsset->m_indices.size() > 0 && "Mesh primitive indices were incorrectly loaded!");
			}
		}

		// Create initial nodes
		std::vector<std::shared_ptr<ModelAssetNode>> pNodes;
		pNodes.reserve(nodeCt + 1u);
		for (size_t i = 0u; i < nodeCt; ++i)
		{
			const auto& node = model.nodes[i];

			// Calculate transform, using identity for any missing factors
			dx::XMMATRIX localTransform = dx::XMMatrixIdentity();

			if (node.scale.size() == 3)
			{
				localTransform *= dx::XMMatrixScaling((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]);
			}
			
			if (node.rotation.size() == 4)
			{
				const auto rotationQuat = dx::XMVectorSet((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]);
				localTransform *= dx::XMMatrixRotationQuaternion(rotationQuat);
			}

			if (node.translation.size() == 3)
			{
				localTransform *= dx::XMMatrixTranslation((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]);
			}

			dx::XMFLOAT4X4 localTransformFloat;
			dx::XMStoreFloat4x4(&localTransformFloat, localTransform);

			// pMeshes can be shared, so don't std::move()
			pNodes.emplace_back(std::make_shared<ModelAssetNode>(node.name, node.mesh != -1 ? pMeshes[node.mesh] : nullptr, std::move(localTransformFloat)));
		}
		
		// Special case for single meshes, to avoid clunky node structures
		if (nodeCt == 1u)
		{
			// Create model asset for single mesh
			auto pModelAsset = std::make_shared<ModelAsset>(std::move(pNodes[0]), settings.materialPaths);
			assert(pModelAsset != nullptr && "Failed to create 1-mesh model asset");
			return std::move(pModelAsset);
		}

		// Connect nodes as scene graph
		std::unordered_set<int> childNodeIndices;
		for (size_t i = 0u; i < nodeCt; ++i)
		{
			const auto& node = model.nodes[i];

			std::vector<std::shared_ptr<ModelAssetNode>> pChildNodes;
			pChildNodes.reserve(node.children.size());

			for (const auto childIdx : node.children)
			{
				pChildNodes.emplace_back(pNodes[childIdx]);
				childNodeIndices.emplace(childIdx);
			}

			pNodes[i]->SetChildNodes(std::move(pChildNodes));
		}

		// Find level 0 nodes
		std::vector<std::shared_ptr<ModelAssetNode>> pLevel0Nodes;
		const auto level0NodeCt = nodeCt - childNodeIndices.size();
		pLevel0Nodes.reserve(level0NodeCt);

		for (size_t i = 0u; i < nodeCt; ++i)
		{
			if (childNodeIndices.find((const int)i) == childNodeIndices.end())
			{
				// If is not a child node of something, parent to the root node
				pLevel0Nodes.emplace_back(pNodes[i]);
			}
		}

		// Create empty root node (never used as a child)
		dx::XMFLOAT4X4 localTransformFloat;
		dx::XMStoreFloat4x4(&localTransformFloat, dx::XMMatrixIdentity());
		pNodes.emplace_back(std::make_shared<ModelAssetNode>(std::move("Root"), nullptr, std::move(localTransformFloat)));
		const auto& pRootNode = pNodes[pNodes.size() - 1u];
		pRootNode->SetChildNodes(std::move(pLevel0Nodes));

		// Create model asset
		auto pModelAsset = std::make_shared<ModelAsset>(pRootNode, settings.materialPaths);
		assert(pModelAsset != nullptr && "Failed to create model asset");
		return std::move(pModelAsset);
	}
}