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
	class Drawable;
	class Renderer;
	class VertexLayout;
	class BindingList;

	struct DrawContext;

	enum RenderPassType : int;

	class Material : public Bindable
	{
	public:
		Material(const GraphicsDevice& gfx, const std::string_view assetPath, const bool instancingOn);
		virtual void Release() override;

	public:
		void Bind(const GraphicsDevice& gfx, const std::string_view passName);
		void SubmitDrawCommands(GraphicsDevice& gfx, const Drawable& drawable, const DrawContext& drawContext) const;

	public:
		void VerifyInstancing(const bool requireInstancing) const;
		const u64 GetMaterialCode(const RenderPassType renderPassType) const;
		const VertexLayout& GetVertexLayout() const;
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const std::string_view assetPath);
		static std::shared_ptr<Bindable> Resolve(const GraphicsDevice& gfx, const std::string_view assetPath, const bool instancingOn);

	protected:
		static std::string GenerateUID(const std::string_view assetPath, const bool instancingOn);
		void AddBindable(std::shared_ptr<Bindable> pBindable);

	private:
		// Contains bindings like textures and uniform data, shared across 1+ material passes
		std::vector<std::unique_ptr<BindingList>> m_pPropertySlots;
		std::unordered_map<RenderPassType, std::unique_ptr<MaterialPass>> m_pMaterialPassesByType;
		std::vector<std::shared_ptr<Bindable>> m_pBindables;
		std::string m_materialAssetPath;
		u64 m_fallbackMaterialCode;
		VertexLayout m_vertexLayout;
		size_t m_instancedPasses = 0u;
		size_t m_nonInstancedPasses = 0u;

	};
}