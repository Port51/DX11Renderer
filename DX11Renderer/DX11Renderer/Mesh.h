#pragma once
#include "TestObject.h"
#include <string>

class Mesh : public DrawableTemplated<Mesh>
{
public:
	Mesh(Graphics& gfx, std::string name, std::vector<std::unique_ptr<Bindable>> pBindables);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	mutable DirectX::XMFLOAT4X4 modelMatrix;
	std::string name;
};