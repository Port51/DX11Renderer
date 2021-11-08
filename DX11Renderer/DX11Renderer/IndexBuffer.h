#pragma once
#include "Bindable.h"
#include <string>
#include <vector>
#include "WindowsInclude.h" // must be before D3D11
#include <wrl.h>
#include <d3d11.h>

class Graphics;

class IndexBuffer : public Bindable
{
public:
	IndexBuffer(Graphics& gfx, const std::vector<unsigned short>& indices);
	IndexBuffer(Graphics& gfx, std::string tag, const std::vector<unsigned short>& indices);
	void Bind(Graphics& gfx) override;
	UINT GetCount() const;
	std::string GetUID() const noexcept override;
public:
	static std::shared_ptr<IndexBuffer> Resolve(Graphics& gfx, const std::string& tag,
		const std::vector<unsigned short>& indices);
	template<typename...Ignore>
	static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
	{
		return GenerateNontemplatedUID(tag);
	}
private:
	static std::string GenerateNontemplatedUID(const std::string& tag);
protected:
	std::string tag;
	UINT count;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
};