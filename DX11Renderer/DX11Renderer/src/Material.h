#pragma once
#include <vector>
#include <unordered_map>
#include <string_view>
#include <string>
#include "VertexLayout.h"
#include "Bindable.h"

namespace gfx
{
	class GraphicsDevice;
	class VertexShader;
	class PixelShader;
	class MaterialPass;
	class MeshRenderer;
	class Renderer;
	class VertexLayout;
	class BindingList;

	struct DrawContext;

	class Material : public Bindable
	{
	public:
		Material(const GraphicsDevice& gfx, std::string_view assetPath);
		void Bind(const GraphicsDevice& gfx, std::string_view passName);
		void SubmitDrawCommands(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
	public:
		const u64 GetMaterialCode() const;
		const VertexLayout& GetVertexLayout() const;
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const std::string_view assetPath);
	protected:
		static std::string GenerateUID(const std::string_view assetPath);
		void AddBindable(std::shared_ptr<Bindable> pBindable);
	private:
		// Contains bindings like textures and uniform data, shared across 1+ material passes
		std::vector<std::unique_ptr<BindingList>> m_pPropertySlots;
		std::unordered_map<std::string, std::unique_ptr<MaterialPass>> m_pPasses;
		std::vector<std::shared_ptr<Bindable>> m_pBindables;
		std::string m_materialAssetPath;
		u64 m_materialCode;
		VertexLayout m_vertexLayout;
	};
}