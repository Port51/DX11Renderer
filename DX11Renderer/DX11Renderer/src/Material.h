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

	struct DrawContext;

	class Material : public Bindable
	{
	public:
		Material(const GraphicsDevice& gfx, std::string_view assetPath);
		void Bind(const GraphicsDevice& gfx, std::string_view passName);
		void SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
	public:
		const VertexLayout& GetVertexLayout() const;
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const std::string_view assetPath);
	protected:
		static std::string GenerateUID(const std::string_view assetPath);
		void AddBindable(std::shared_ptr<Bindable> pBindable);
	private:
		std::shared_ptr<VertexShader> m_pVertexShader;
		std::shared_ptr<PixelShader> m_pPixelShader;
		std::unordered_map<std::string, std::unique_ptr<MaterialPass>> m_pPasses;
		std::vector<std::shared_ptr<Bindable>> m_pBindables;
		std::string m_materialAssetPath;
		VertexLayout m_vertexLayout;
	};
}