#include "pch.h"
#include "ModelImporter.h"
#include "GLTFInclude.h"
#include "GraphicsDevice.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "TextParser.h"
#include <assert.h>
#include <unordered_set>
#include <limits>

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

	size_t ModelImporter::GetAttributeCt(const GraphicsDevice& gfx, const tinygltf::Model& model, int accessorIdx)
	{
		const auto accessor = model.accessors[accessorIdx];
		return accessor.count;
	}

	std::pair<int, size_t> ModelImporter::GetAttributeBufferAccess(const GraphicsDevice& gfx, const tinygltf::Model& model, const int accessorIdx, const size_t byteCt)
	{
		// Each attribute owns a segment of data in the buffer
		// It can be accessed by using the view and accessor offsets
		const auto& accessor = model.accessors[accessorIdx];
		const auto& view = model.bufferViews[accessor.bufferView];

		if (view.byteLength != byteCt) THROW("Buffer view does not match expected number of elements!");

		return std::pair<int, size_t>(view.buffer, (size_t)(view.byteOffset + accessor.byteOffset));
	}

	std::shared_ptr<ModelAsset> ModelImporter::LoadGLTF(const GraphicsDevice& gfx, const char * assetFilename)
	{
		const auto settings = GetImportSettings(gfx, assetFilename);
		const tinygltf::Model model = TryLoadGLTFModel(gfx, settings.gltfPath.c_str(), settings.gltfIsBinary);

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
		
		gfx.GetLog().Info(std::string("GLTF has ") + std::to_string(nodeCt) + std::string(" nodes, ") + std::to_string(meshCt) + std::string(" meshes"));

		// Create meshes
		std::vector<std::shared_ptr<MeshAsset>> pMeshes;
		pMeshes.reserve(meshCt);
		for (const auto& mesh : model.meshes)
		{
			// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
			static const std::string quadMarker = "_quad";
			const bool createQuads = mesh.name.size() >= quadMarker.size() && 0 == mesh.name.compare(mesh.name.size() - quadMarker.size(), quadMarker.size(), quadMarker);

			// Each primitive is a submesh
			u32 subNameCt = 0u;
			for (const auto& primitive : mesh.primitives)
			{
				// Reference:
				// https://github.com/syoyo/tinygltf/wiki/Accessing-vertex-data

				pMeshes.emplace_back(std::make_shared<MeshAsset>());
				const auto& pCurrentMeshAsset = pMeshes.at(pMeshes.size() - 1u);
				pCurrentMeshAsset->m_name = settings.name + "|" + mesh.name + "(sub=" + std::to_string(subNameCt++) + ")";
				pCurrentMeshAsset->m_faceDimension = (createQuads) ? 4u : 3u;

				if (primitive.attributes.count("POSITION") != 1u) THROW("Mesh primitive " + pCurrentMeshAsset->m_name + " has no POSITION attribute!");

				// Get material index by mesh name
				if (settings.materialIndicesByName.find(mesh.name) != settings.materialIndicesByName.end())
				{
					pCurrentMeshAsset->m_materialIndex = settings.materialIndicesByName.at(mesh.name);
				}
				else
				{
					pCurrentMeshAsset->m_materialIndex = 0u;
				}

				// Get vert count from POSITION attribute
				const auto vertCt = GetAttributeCt(gfx, model, primitive.attributes.at("POSITION"));
				if (vertCt == 0u) THROW("Mesh primitive " + pCurrentMeshAsset->m_name + " has 0 vertices!");

				// Load INDICES
				{
					const int accessorIdx = primitive.indices;
					const auto accessor = model.accessors[accessorIdx];
					const auto& view = model.bufferViews[accessor.bufferView];

					const size_t indicesCt = accessor.count;
					if (indicesCt == 0u) THROW("Mesh primitive " + pCurrentMeshAsset->m_name + " has 0 indices!");

					// Determine index size
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, indicesCt * sizeof(u16));
						const u16* indices = reinterpret_cast<const u16*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);

						if (createQuads)
						{
							const size_t quadCt = indicesCt / 6u;
							for (int i = 0; i < quadCt; ++i)
							{
								// Unwind triangles
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 2u]);
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 5u]);
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 0u]);
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 1u]);
							}
						}
						else
						{
							for (size_t vi = 0u; vi < indicesCt; ++vi)
							{
								pCurrentMeshAsset->m_indices.emplace_back((u32)indices[vi]);
							}
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					{
						const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, indicesCt * sizeof(u32));
						const u32* indices = reinterpret_cast<const u32*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
						
						if (createQuads)
						{
							const size_t quadCt = indicesCt / 6u;
							for (int i = 0; i < quadCt; ++i)
							{
								// Unwind triangles
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 2u]);
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 5u]);
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 0u]);
								pCurrentMeshAsset->m_indices.emplace_back(indices[i * 6u + 1u]);
							}
						}
						else
						{
							for (size_t vi = 0u; vi < indicesCt; ++vi)
							{
								pCurrentMeshAsset->m_indices.emplace_back((u32)indices[vi]);
							}
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
						const auto posF3 = RH_to_LH(dx::XMFLOAT3(positions[vi * 3u + 0u], positions[vi * 3u + 1u], positions[vi * 3u + 2u]));
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
						pCurrentMeshAsset->m_normals.emplace_back(RH_to_LH(dx::XMFLOAT3(normals[vi * 3u + 0u], normals[vi * 3u + 1u], normals[vi * 3u + 2u])));
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
						pCurrentMeshAsset->m_tangents.emplace_back(RH_to_LH(dx::XMFLOAT4(tangents[vi * 4u + 0u], tangents[vi * 4u + 1u], tangents[vi * 4u + 2u], tangents[vi * 4u + 3u])));
					}
				}

				// Load VERTCOLOR0
				if (primitive.attributes.count("COLOR_0"))
				{
					pCurrentMeshAsset->hasVertColors = true;
					pCurrentMeshAsset->m_vertColors.reserve(vertCt);

					const int accessorIdx = primitive.attributes.at("COLOR_0");
					const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, vertCt * sizeof(u16) * 4u);
					const u16* vertColors = reinterpret_cast<const u16*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
					for (size_t vi = 0; vi < vertCt; ++vi)
					{
						// todo: move this to separate method and figure out what gamma/linear formula GLTF uses
						auto r = std::sqrt((float)(vertColors[vi * 4u + 0u]) / std::numeric_limits<u16>::max());
						auto g = std::sqrt((float)(vertColors[vi * 4u + 1u]) / std::numeric_limits<u16>::max());
						auto b = std::sqrt((float)(vertColors[vi * 4u + 2u]) / std::numeric_limits<u16>::max());
						auto a = std::sqrt((float)(vertColors[vi * 4u + 3u]) / std::numeric_limits<u16>::max());
						pCurrentMeshAsset->m_vertColors.emplace_back(dx::XMFLOAT4(r, g, b, a));
					}
				}

				// Detect up to 4 texcoords
				std::vector<std::string> texcoordStrings;
				for (size_t ti = 0u; ti < 4u; ++ti)
				{
					auto key = std::string("TEXCOORD_") + std::to_string(ti);
					if (primitive.attributes.count(key))
					{
						texcoordStrings.emplace_back(std::move(key));
					}
				}

				pCurrentMeshAsset->m_texcoords.reserve(texcoordStrings.size());

				// Load TEXCOORDs
				for (size_t ti = 0u; ti < texcoordStrings.size(); ++ti)
				{
					std::vector<dx::XMFLOAT2> texcoordSet;
					texcoordSet.reserve(vertCt);

					const int accessorIdx = primitive.attributes.at(texcoordStrings[ti]);
					const auto bufferAccess = GetAttributeBufferAccess(gfx, model, accessorIdx, vertCt * sizeof(float) * 2u);
					const float* coords = reinterpret_cast<const float*>(&model.buffers[bufferAccess.first].data[bufferAccess.second]);
					for (size_t vi = 0u; vi < vertCt; ++vi)
					{
						texcoordSet.emplace_back(dx::XMFLOAT2(coords[vi * 2u + 0u], coords[vi * 2u + 1u]));
					}

					pCurrentMeshAsset->m_texcoords.emplace_back(std::move(texcoordSet));
				}

				// Calculate AABB
				pCurrentMeshAsset->m_aabb.SetBoundsByVertices(pCurrentMeshAsset->m_vertices);

				// Double check that everything was loaded correctly
				if (pCurrentMeshAsset->m_vertices.size() == 0u) THROW("Mesh primitive " + pCurrentMeshAsset->m_name + " vertices were incorrectly loaded!");
				if (pCurrentMeshAsset->m_indices.size() == 0u) THROW("Mesh primitive " + pCurrentMeshAsset->m_name + " indices were incorrectly loaded!");
			}
		}

		// Create initial nodes
		std::vector<std::shared_ptr<ModelAssetNode>> pNodes;
		pNodes.reserve(nodeCt + 1u);
		for (const auto& node : model.nodes)
		{
			// pMeshes can be shared, so don't std::move()
			pNodes.emplace_back(std::make_shared<ModelAssetNode>(node.name, node.mesh != -1 ? pMeshes[node.mesh] : nullptr, GetNodeTransform(node).trs));
		}
		
		// Special case for single meshes, to avoid clunky node structures
		if (nodeCt == 1u)
		{
			// Create model asset for single mesh
			auto pModelAsset = std::make_shared<ModelAsset>(std::move(pNodes[0]), settings.materialPaths);
			if (pModelAsset == nullptr) THROW("Failed to create 1-mesh model asset");
			return std::move(pModelAsset);
		}

		// Connect nodes as scene graph
		std::unordered_set<int> childNodeIndices;
		for (size_t i = 0u; i < nodeCt; ++i)
		{
			const auto& node = model.nodes.at(i);

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
				pLevel0Nodes.emplace_back(pNodes.at(i));
			}
		}

		// Create empty root node (never used as a child)
		dx::XMFLOAT4X4 localTransformFloat;
		dx::XMStoreFloat4x4(&localTransformFloat, dx::XMMatrixIdentity());
		pNodes.emplace_back(std::make_shared<ModelAssetNode>(std::move("Root"), nullptr, std::move(localTransformFloat)));
		const auto& pRootNode = pNodes.at(pNodes.size() - 1u);
		pRootNode->SetChildNodes(std::move(pLevel0Nodes));

		// Create model asset
		auto pModelAsset = std::make_shared<ModelAsset>(pRootNode, settings.materialPaths);
		if (pModelAsset == nullptr) THROW("Failed to create 1-mesh model asset");
		return std::move(pModelAsset);
	}

	std::vector<dx::XMFLOAT3> ModelImporter::LoadGLTFPositions(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary)
	{
		std::vector<dx::XMFLOAT3> positions;
		const tinygltf::Model model = TryLoadGLTFModel(gfx, gltfFilename, isBinary);

		// Get positions from nodes
		positions.reserve(model.nodes.size());
		for (const auto& node : model.nodes)
		{
			positions.emplace_back(RH_to_LH(dx::XMFLOAT3((float)node.translation.at(0u), (float)node.translation.at(1u), (float)node.translation.at(2u))));
		}

		return std::move(positions);
	}

	std::vector<dx::XMFLOAT4> ModelImporter::LoadGLTFPositionsAndScales(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary)
	{
		std::vector<dx::XMFLOAT4> positions;
		const tinygltf::Model model = TryLoadGLTFModel(gfx, gltfFilename, isBinary);

		// Get positions from nodes
		positions.reserve(model.nodes.size());
		for (const auto& node : model.nodes)
		{
			const float avgScale = (node.scale.size() == 3u) ? ((float)node.scale.at(0u) + (float)node.scale.at(1u) + (float)node.scale.at(2u)) / 3.f : 1.f;
			positions.emplace_back(RH_to_LH(dx::XMFLOAT4((float)node.translation.at(0u), (float)node.translation.at(1u), (float)node.translation.at(2u), avgScale)));
		}

		return std::move(positions);
	}

	std::vector<TRS> ModelImporter::LoadGLTFTransforms(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary)
	{
		std::vector<TRS> transforms;
		const tinygltf::Model model = TryLoadGLTFModel(gfx, gltfFilename, isBinary);

		// Get positions from nodes
		transforms.reserve(model.nodes.size());
		for (const auto& node : model.nodes)
		{
			transforms.emplace_back(GetNodeTransform(node));
		}

		return std::move(transforms);
	}

	inline dx::XMFLOAT3 ModelImporter::RH_to_LH(const dx::XMFLOAT3& vec)
	{
		return dx::XMFLOAT3(vec.x, vec.y, -vec.z);
	}

	inline dx::XMFLOAT4 ModelImporter::RH_to_LH(const dx::XMFLOAT4& vec)
	{
		return dx::XMFLOAT4(vec.x, vec.y, -vec.z, vec.w);
	}

	dx::XMFLOAT4 ModelImporter::RH_to_LH_Quaternion(const dx::XMFLOAT4& q)
	{
		// ref: https://stackoverflow.com/questions/1274936/flipping-a-quaternion-from-right-to-left-handed-coordinates
		return dx::XMFLOAT4(q.z, q.y, q.x, -q.w);
	}

	TRS ModelImporter::GetNodeTransform(const tinygltf::Node& node)
	{
		// Calculate transform, using identity for any missing factors
		dx::XMMATRIX localTransform = dx::XMMatrixIdentity();

		if (node.scale.size() == 3u)
		{
			localTransform *= dx::XMMatrixScaling((float)node.scale.at(0u), (float)node.scale.at(1u), (float)node.scale.at(2u));
		}

		if (node.rotation.size() == 4u)
		{
			const auto rotationQuat = RH_to_LH_Quaternion(dx::XMFLOAT4((float)node.rotation.at(0u), (float)node.rotation.at(1u), (float)node.rotation.at(2u), (float)node.rotation.at(3u)));
			localTransform *= dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4(&rotationQuat));
		}

		if (node.translation.size() == 3u)
		{
			const auto tr = RH_to_LH(dx::XMFLOAT3((float)node.translation.at(0u), (float)node.translation.at(1u), (float)node.translation.at(2u)));
			localTransform *= dx::XMMatrixTranslation(tr.x, tr.y, tr.z);
		}

		dx::XMVECTOR scale;
		dx::XMVECTOR rotation;
		dx::XMVECTOR position;
		dx::XMMatrixDecompose(&scale, &rotation, &position, localTransform);

		TRS trs;
		dx::XMStoreFloat3(&trs.scale, scale);
		dx::XMStoreFloat3(&trs.rotation, rotation);
		dx::XMStoreFloat3(&trs.position, position);
		dx::XMStoreFloat4x4(&trs.trs, localTransform);

		return trs;
	}

	tinygltf::Model ModelImporter::TryLoadGLTFModel(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary)
	{
		tinygltf::Model model;
		TinyGLTF loader;
		std::string err;
		std::string warn;

		gfx.GetLog().Info(std::string("LoadGLTFPositions(") + std::string(gltfFilename) + std::string(")"));

		bool ret;
		if (isBinary)
		{
			ret = loader.LoadBinaryFromFile(&model, &err, &warn, gltfFilename); // for binary glTF(.glb)
		}
		else
		{
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, gltfFilename);
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
			THROW("Could not parse glTF for filename '" + std::string(gltfFilename) + "'");
		}

		if (model.scenes.size() <= model.defaultScene || model.defaultScene < 0)
		{
			THROW("No default scene for filename '" + std::string(gltfFilename) + "'");
		}

		return model;
	}
}