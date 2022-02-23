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
	class Graphics;
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
		static std::shared_ptr<ModelAsset> LoadGLTF(Graphics& gfx, const char* assetFilename);
	private:
		static ModelImportSettings GetImportSettings(Graphics& gfx, const char* assetFilename);
		static size_t GetAttributeCt(Graphics& gfx, tinygltf::Model& model, int accessorIdx);
		static std::pair<int, size_t> GetAttributeBufferAccess(Graphics& gfx, tinygltf::Model& model, int accessorIdx, size_t byteCt);
	};
}