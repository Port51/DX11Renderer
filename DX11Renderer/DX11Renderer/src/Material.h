#pragma once
#include <vector>
#include <unordered_map>
#include <string_view>
#include <string>
#include "VertexLayout.h"
#include "Bindable.h"

namespace gfx
{
	class Graphics;
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
		Material(Graphics& gfx, std::string_view assetPath);
		void Bind(Graphics& gfx, std::string_view passName);
		void SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
	public:
		const VertexLayout& GetVertexLayout() const;
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const std::string_view assetPath);
	protected:
		static std::string GenerateUID(const std::string_view assetPath);
		void AddBindable(std::shared_ptr<Bindable> pBindable);
	public:
		std::shared_ptr<VertexShader> pVertexShader;
		std::shared_ptr<PixelShader> pPixelShader;
	private:
		std::unordered_map<std::string, std::unique_ptr<MaterialPass>> pPasses;
		std::vector<std::shared_ptr<Bindable>> pBindables;
		std::string materialAssetPath;
		VertexLayout vertexLayout;
	};
}