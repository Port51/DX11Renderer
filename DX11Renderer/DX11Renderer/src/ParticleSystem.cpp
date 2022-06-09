#include "pch.h"
#include "ParticleSystem.h"
#include "MeshRenderer.h"
#include "GraphicsDevice.h"
#include "Material.h"
#include "Topology.h"
#include "RenderState.h"

namespace gfx
{
	ParticleSystem::ParticleSystem(const size_t maxParticleCount, const UINT argsBufferByteOffset, const dx::XMVECTOR positionWS)
		: m_maxParticleCount(maxParticleCount), m_argsBufferByteOffset(argsBufferByteOffset), m_positionWS(positionWS)
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

	void ParticleSystem::DrawIndirect(const GraphicsDevice & gfx, RenderState& renderState, ID3D11Buffer* const pArgsBuffer) const
	{
		//m_pMaterial->SubmitDrawCommands(gfx, "ParticlePass");
		m_pTopology->BindIA(gfx, renderState, 0u);
		gfx.GetContext()->DrawInstancedIndirect(pArgsBuffer, m_argsBufferByteOffset);
	}

	ParticleSystemSettings ParticleSystem::GetParticleSystemSettings(const size_t bufferOffset) const
	{
		ParticleSystemSettings pss;
		pss.emitPositionWS = m_positionWS;
		pss.maxParticles = m_maxParticleCount;
		pss.bufferOffset = bufferOffset;
		return pss;
	}
}