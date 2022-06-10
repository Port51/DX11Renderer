#include "pch.h"
#include "ParticleSystem.h"
#include "Drawable.h"
#include "GraphicsDevice.h"
#include "Material.h"
#include "Topology.h"
#include "RenderState.h"
#include "ArgsBuffer.h"

namespace gfx
{
	ParticleSystem::ParticleSystem(std::shared_ptr<Material> pMaterial, const size_t maxParticleCount, const dx::XMVECTOR positionWS)
		: m_pMaterial(pMaterial), m_maxParticleCount(maxParticleCount), m_positionWS(positionWS)
	{
	}

	ParticleSystem::~ParticleSystem()
	{
	}

	const size_t ParticleSystem::GetMaxParticleCount() const
	{
		return m_maxParticleCount;
	}

	const dx::XMVECTOR ParticleSystem::GetPositionWS() const
	{
		return m_positionWS;
	}

	void ParticleSystem::Bind(const GraphicsDevice& gfx, RenderState& renderState, const DrawContext& drawContext) const
	{
		/*m_pTopology->BindIA(gfx, renderState, 0u);
		m_pVertexBufferWrapper->BindIA(gfx, renderState, 0u);

		const auto modelMatrix = GetTransformXM();
		const auto modelViewMatrix = modelMatrix * drawContext.viewMatrix;
		const auto modelViewProjectMatrix = modelViewMatrix * drawContext.projMatrix;
		const Transforms transforms{ modelMatrix, modelViewMatrix, modelViewProjectMatrix };
		m_pTransformCbuf->UpdateTransforms(gfx, transforms);

		m_pTransformCbuf->BindVS(gfx, renderState, RenderSlots::VS_TransformCB);*/
	}

	void ParticleSystem::IssueDrawCall(const GraphicsDevice& gfx) const
	{
		gfx.GetContext()->DrawInstancedIndirect(m_pArgsBuffer->GetD3DBuffer().Get(), m_argsBufferByteOffset);
	}

	ParticleSystemSettings ParticleSystem::GetParticleSystemSettings(const size_t bufferOffset) const
	{
		ParticleSystemSettings pss;
		pss.emitPositionWS = m_positionWS;
		pss.maxParticles = m_maxParticleCount;
		pss.bufferOffset = bufferOffset;
		return pss;
	}

	void ParticleSystem::SetupArgsBuffer(std::shared_ptr<ArgsBuffer> pArgsBuffer, const UINT bufferOffset)
	{
		m_pArgsBuffer = pArgsBuffer;
		m_argsBufferByteOffset = bufferOffset;
	}
}