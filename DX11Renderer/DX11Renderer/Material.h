#pragma once
#include <memory>
#include <vector>
//#include "VertexShader.h"

class Graphics;
class VertexShader;
class PixelShader;
class Bindable;

class Material
{
public:
	Material(std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<PixelShader> pPixelShader);
public:
	void Bind(Graphics& gfx);
private:
	std::vector<std::shared_ptr<Bindable>> bindablePtrs;
};