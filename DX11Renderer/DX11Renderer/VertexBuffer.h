#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"
#include "VertexInclude.h"

class VertexBuffer : public Bindable
{
public:
	VertexBuffer(Graphics& gfx, const std::string& tag, const VertexBufferData& vbuf);
	VertexBuffer(Graphics& gfx, const VertexBufferData& vbuf);
	void Bind(Graphics& gfx) override;
	std::string GetUID() const override;
public:
	static std::shared_ptr<VertexBuffer> Resolve(Graphics& gfx, const std::string& tag, const VertexBufferData& vbuf);
	template<typename...Ignore>
	static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
	{
		return GenerateNontemplatedUID(tag);
	}
private:
	static std::string GenerateNontemplatedUID(const std::string& tag);
protected:
	std::string tag;
	UINT stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
};
