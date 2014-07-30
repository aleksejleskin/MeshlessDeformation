#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
	m_physicsManager = new PhysicsManager(); 
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::LoadContent(DxGraphics* dx)
{
	if (!m_modelManager.LoadContent(dx))
	{
		MessageBoxA(NULL, "Failed to load model Assets!", "ERROR!", MB_OK | MB_ICONERROR);
		return false;
	}
	if (!m_shaderManager.LoadContent(dx))
	{
		MessageBoxA(NULL, "Failed to load shader Assets!", "ERROR!", MB_OK | MB_ICONERROR);
		return false;
	}
	if (!m_textureManager.LoadContent(dx))
	{
		MessageBoxA(NULL, "Failed to load texture Assets!", "ERROR!", MB_OK | MB_ICONERROR);
		return false;
	}

	m_physicsManager->LoadContent();
	return true;
}

void ResourceManager::UnloadContent()
{
	m_modelManager.UnloadContent();
	m_shaderManager.UnloadContent();
	m_textureManager.UnloadContent();
	m_physicsManager->~PhysicsManager();
}

ModelManager& ResourceManager::GetModels()
{
	return m_modelManager;
}
ShaderManager& ResourceManager::GetShaders()
{
	return m_shaderManager;
}
TextureManager& ResourceManager::GetTextures()
{
	return m_textureManager;
}

PhysicsManager* ResourceManager::GetPhysicsManager()
{
	return m_physicsManager;
}
