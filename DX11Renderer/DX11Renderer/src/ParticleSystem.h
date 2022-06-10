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

	class ParticleSystem : public Drawable
	{
	public:
		ParticleSystem(const size_t maxParticleCount, const UINT argsBufferByteOffset, const dx::XMVECTOR positionWS);
		~ParticleSystem();
	public:
		const size_t GetMaxParticleCount() const;
		const dx::XMVECTOR GetPositionWS() const;
		ParticleSystemSettings GetParticleSystemSettings(const size_t bufferOffset = 0u) const;
	public:
		void Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const override;
		void IssueDrawCall(const GraphicsDevice& gfx) const override;
	protected:
		const UINT m_argsBufferByteOffset;
		const size_t m_maxParticleCount;
		dx::XMVECTOR m_positionWS;
		std::shared_ptr<Material> m_pMaterial;
		std::shared_ptr<Topology> m_pTopology;
	};

}