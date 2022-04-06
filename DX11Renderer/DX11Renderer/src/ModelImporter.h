#pragma once
#include <vector>
#include <unordered_map>
#include <string>

namespace tinygltf
{
	class Model;
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
	private:
		static ModelImportSettings GetImportSettings(const GraphicsDevice& gfx, const char* assetFilename);
		static size_t GetAttributeCt(const GraphicsDevice& gfx, tinygltf::Model& model, int accessorIdx);
		static std::pair<int, size_t> GetAttributeBufferAccess(const GraphicsDevice& gfx, tinygltf::Model& model, int accessorIdx, size_t byteCt);
	};
}