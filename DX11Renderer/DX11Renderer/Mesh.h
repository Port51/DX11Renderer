#pragma once
#include "TestObject.h"

class Mesh : public TestObject<Mesh>
{
public:
	Mesh(Graphics& gfx, std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist);
	void Update(float dt) override;
	DirectX::XMMATRIX GetTransformXM() const override;
private:
	// model transform
	DirectX::XMFLOAT3X3 mt;
};