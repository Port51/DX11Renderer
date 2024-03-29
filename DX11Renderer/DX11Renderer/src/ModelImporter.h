#pragma once
#include <vector>
#include <unordered_map>
#include <string>

namespace tinygltf
{
	class Model;
	class Node;
}

namespace gfx
{
	class GraphicsDevice;
	class ModelAsset;

	class ModelImporter
	{
	private:
		class ModelImportSettings
		{
		public:
			bool gltfIsBinary;
			std::string name;
			std::string gltfPath;
			std::vector<std::string> materialPaths;
			std::unordered_map<std::string, int> materialIndicesByName;
		};
	public:
		static std::shared_ptr<ModelAsset> LoadGLTF(const GraphicsDevice& gfx, const char* assetFilename);
		static std::vector<dx::XMFLOAT3> LoadGLTFPositions(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary = true);
		static std::vector<dx::XMFLOAT4> LoadGLTFPositionsAndScales(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary = true);
		static std::vector<TRS> LoadGLTFTransforms(const GraphicsDevice& gfx, const char* assetFilename, const bool isBinary = true);
	private:
		static dx::XMFLOAT3 RH_to_LH(const dx::XMFLOAT3& vec);
		static dx::XMFLOAT4 RH_to_LH(const dx::XMFLOAT4& vec);
		static dx::XMFLOAT4 RH_to_LH_Quaternion(const dx::XMFLOAT4& q);
		static TRS GetNodeTransform(const tinygltf::Node& node);
		static tinygltf::Model TryLoadGLTFModel(const GraphicsDevice& gfx, const char* gltfFilename, const bool isBinary);
		static ModelImportSettings GetImportSettings(const GraphicsDevice& gfx, const char* assetFilename);
		static size_t GetAttributeCt(const GraphicsDevice& gfx, const tinygltf::Model& model, int accessorIdx);
		static std::pair<int, size_t> GetAttributeBufferAccess(const GraphicsDevice& gfx, const tinygltf::Model& model, const int accessorIdx, const size_t byteCt);
	};
}