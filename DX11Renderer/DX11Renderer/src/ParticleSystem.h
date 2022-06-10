#pragma once
#include "Drawable.h"
#include "CommonHeader.h"
#include "ParticleCommon.h"

namespace gfx
{
	class GraphicsDevice;
	class Material;
	class Topology;
	class RenderState;
	class ArgsBuffer;

	class ParticleSystem : public Drawable
	{
	public:
		ParticleSystem(std::shared_ptr<Material> pMaterial, const size_t maxParticleCount, const dx::XMVECTOR positionWS);
		~ParticleSystem();
	public:
		const size_t GetMaxParticleCount() const;
		const dx::XMVECTOR GetPositionWS() const;
		ParticleSystemSettings GetParticleSystemSettings(const size_t bufferOffset = 0u) const;
		void SetupArgsBuffer(std::shared_ptr<ArgsBuffer> pArgsBuffer, const UINT bufferOffset);
	public:
		void Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const override;
		void IssueDrawCall(const GraphicsDevice& gfx) const override;
	protected:
		const size_t m_maxParticleCount;
		dx::XMVECTOR m_positionWS;
		std::shared_ptr<Material> m_pMaterial;
		std::shared_ptr<Topology> m_pTopology;
		std::shared_ptr<ArgsBuffer> m_pArgsBuffer;
		UINT m_argsBufferByteOffset;
	};

}