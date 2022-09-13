#pragma once
#include "CommonHeader.h"
#include "DrawCommand.h"
#include <vector>

namespace gfx
{
	class GraphicsDevice;
	class Binding;
	class Bindable;
	class RenderTexture;

	///
	/// Wrapper containing jobs
	///
	class RenderPass
	{
	protected:
		///
		/// Wrapper containing bindings for programmable graphics stages (CS, VS, PS, etc.)
		///
		class ProgrammableStageBindings
		{
		public:
			std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> CB_Binds;
			std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> SRV_Binds;
			std::vector<std::pair<UINT, ComPtr<ID3D11UnorderedAccessView>>> UAV_Binds;
			std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> SPL_Binds;
		public:
			void clear();
		};

	public:
		RenderPass(const RenderPassType renderPassType);
		virtual ~RenderPass();

	public:
		const std::string& GetName() const;
		const RenderPassType GetRenderPassType() const;
		void EnqueueJob(DrawCommand job);
		virtual void BindSharedResources(const GraphicsDevice& gfx, RenderState& renderState) const;
		virtual void UnbindSharedResources(const GraphicsDevice& gfx, RenderState& renderState) const;
		virtual void Execute(const GraphicsDevice& gfx, RenderState& renderState) const;
		void Reset();
		virtual void DrawImguiControls(const GraphicsDevice& gfx);

	public:
		RenderPass& ClearBinds();
		RenderPass& SetGlobalCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource);
		RenderPass& SetGlobalSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource);
		RenderPass& SetGlobalSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource);
		RenderPass& CSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource);
		RenderPass& CSSetSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource);
		RenderPass& CSSetUAV(const slotUINT slot, const ComPtr<ID3D11UnorderedAccessView>& pResource);
		RenderPass& CSSetSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource);
		RenderPass& VSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource);
		RenderPass& VSSetSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource);
		RenderPass& VSSetSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource);
		RenderPass& HSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource);
		RenderPass& DSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource);
		RenderPass& PSSetCB(const slotUINT slot, const ComPtr<ID3D11Buffer>& pResource);
		RenderPass& PSSetSRV(const slotUINT slot, const ComPtr<ID3D11ShaderResourceView>& pResource);
		RenderPass& PSSetSPL(const slotUINT slot, const ComPtr<ID3D11SamplerState>& pResource);
		RenderPass& SetCameraColorOut(std::shared_ptr<RenderTexture> pCameraColor);
		Binding& AddBinding(const std::shared_ptr<Bindable>& pBindable);

		const RenderTexture& GetCameraColorOut() const;
		static const std::size_t GetHash(const std::string passName);
	protected:
		RenderPass& GetSubPass(const u8 pass) const;
		const RenderPass& CreateSubPass(const u8 pass);
		const RenderPass& CreateSubPass(const u8 pass, std::unique_ptr<RenderPass> pRenderPass);

	protected:
		std::string m_name;
		std::vector<Binding> m_bindings;
		const RenderPassType m_renderPassType;
		std::vector<DrawCommand> m_jobs; // will be replaced by render graph
		UINT m_bindingCount;

		// Binds shared by everything in this render pass
		ProgrammableStageBindings m_CS_Bindings;
		ProgrammableStageBindings m_VS_Bindings;
		ProgrammableStageBindings m_HS_Bindings;
		ProgrammableStageBindings m_DS_Bindings;
		ProgrammableStageBindings m_PS_Bindings;

		std::shared_ptr<RenderTexture> m_pCameraColorOut;
		std::unordered_map<UINT, std::unique_ptr<RenderPass>> m_pSubPasses;
	};
}