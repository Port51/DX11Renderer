#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include <memory>
#include "Technique.h"

class IndexBuffer;
class VertexBuffer;
class Topology;
class InputLayout;

class Drawable
{
public:
	Drawable() = default;
	Drawable(const Drawable&) = delete;
	Drawable(std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer);
	void AddTechnique(Technique tech_in) noexcept;
	virtual DirectX::XMMATRIX GetTransformXM() const = 0;

	// Submits draw call
	void Submit(class FrameCommander& frame) const noexcept;
	void Bind(Graphics& gfx) const noexcept;
	UINT GetIndexCount() const;
	virtual ~Drawable();
protected:
	std::shared_ptr<IndexBuffer> pIndices;
	std::shared_ptr<VertexBuffer> pVertices;
	std::shared_ptr<Topology> pTopology;
	std::vector<Technique> techniques;
};