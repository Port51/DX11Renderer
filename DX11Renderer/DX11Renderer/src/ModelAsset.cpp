#include "pch.h"
#include "ModelAsset.h"
#include "MeshAsset.h"
#include "Model.h"
#include <assert.h>
#include <stdio.h>

#include "DX11Window.h"
#include "App.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "RendererList.h"
#include "Model.h"
#include "ModelImporter.h"
#include "SharedCodex.h"
#include "VertexShader.h"
#include "Sampler.h"
#include "LightManager.h"
#include "Light.h"
#include "Renderer.h"

namespace gfx
{
	ModelAsset::ModelAsset(std::shared_ptr<ModelAssetNode> _pSceneGraph, std::vector<std::string> _materialPaths)
		: m_pSceneGraph(std::move(_pSceneGraph)), m_materialPaths(std::move(_materialPaths))
	{

	}

	ModelAsset::~ModelAsset()
	{}

	void ModelAsset::Release()
	{
		m_pSceneGraph->Release();
	}

	std::shared_ptr<Model> ModelAsset::CreateSharedInstance(const GraphicsDevice& gfx, dx::XMMATRIX transform)
	{
		// todo: fix alignment bug
		return nullptr;
		//return std::make_shared<Model>(gfx, *this, std::move(transform));
	}

	std::unique_ptr<Model> ModelAsset::CreateUniqueInstance(const GraphicsDevice& gfx, dx::XMMATRIX transform)
	{
		// todo: fix alignment bug
		return nullptr;
		//return std::make_unique<Model>(gfx, *this, std::move(transform));
	}

	ModelAssetNode::ModelAssetNode(std::string _name, std::shared_ptr<MeshAsset> pMeshAsset, dx::XMFLOAT4X4 _localTransform)
		: m_name(_name), m_pMeshAsset(std::move(pMeshAsset)), m_localTransform(std::move(_localTransform))
	{}

	void ModelAssetNode::Release()
	{
		m_pMeshAsset.reset();
		for (size_t i = 0, ct = m_pChildNodes.size(); i < ct; ++i)
		{
			m_pChildNodes[i]->Release();
		}
	}

	MeshAsset* ModelAssetNode::GetMeshAsset() const
	{
		return m_pMeshAsset.get();
	}

	const dx::XMFLOAT4X4 ModelAssetNode::GetLocalTransform() const
	{
		return m_localTransform;
	}

	void ModelAssetNode::SetChildNodes(std::vector<std::shared_ptr<ModelAssetNode>> _pChildNodes)
	{
		m_pChildNodes = std::move(_pChildNodes);
	}
}