#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGameObject::CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CGameObject::CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,int nMaterials) : CGameObject(pd3dDevice, pd3dCommandList)
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		//m_ppMaterials = new CMaterial * [m_nMaterials];
		//for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}
}
CGameObject::~CGameObject()
{
	//if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			//if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	//if (m_ppMaterials) delete[] m_ppMaterials;

	//if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
}

void CGameObject::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		// (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

//void CGameObject::SetMaterial(int nMaterial, CMaterial* pMaterial)
//{
//	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
//	m_ppMaterials[nMaterial] = pMaterial;
//	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
//}
//
//CSkinnedMesh* CGameObject::FindSkinnedMesh(char* pstrSkinnedMeshName)
//{
//	CSkinnedMesh* pSkinnedMesh = NULL;
//	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
//	{
//		pSkinnedMesh = (CSkinnedMesh*)m_pMesh;
//		if (!strncmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName, strlen(pstrSkinnedMeshName))) return(pSkinnedMesh);
//	}
//
//	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
//	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
//
//	return(NULL);
//}
//
//void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh)
//{
//	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh*)m_pMesh;
//
//	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
//	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
//}

CGameObject* CGameObject::FindFrame(char* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

//void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
//{
//	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
//}
//
//void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
//{
//	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
//}

void CGameObject::Animate(float fTimeElapsed)
{

	//if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

		/*if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList);
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i);
			}
		}*/

		m_pMesh->Render(pd3dCommandList, 0);
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList);
	if (m_pChild) m_pChild->Render(pd3dCommandList);
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_d3dcbvObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_d3dcbvObject->Map(0, NULL, (void**)&m_cbMappedObject);

	m_d3dCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_d3dcbvObject, ncbElementBytes);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&m_cbMappedObject->xmf4x4world, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRootDescriptorTable(1, GetDescriptorHandle());
	//pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		//if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4ToParent._41 += xmf3Offset.x;
	m_xmf4x4ToParent._42 += xmf3Offset.y;
	m_xmf4x4ToParent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetToParentPosition()
{
	return(XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	XMFLOAT3 xmfloat3 = { m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33 };
	return(Vector3::Normalize(xmfloat3));
}

XMFLOAT3 CGameObject::GetUp()
{
	XMFLOAT3 xmfloat3 = { m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23 };
	return(Vector3::Normalize(xmfloat3));
}

XMFLOAT3 CGameObject::GetRight()
{
	XMFLOAT3 xmfloat3 = { m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13 };
	return(Vector3::Normalize(xmfloat3));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

D3D12_GPU_DESCRIPTOR_HANDLE CGameObject::GetDescriptorHandle()
{
	return m_d3dCbvGPUDescriptorHandle;
}

void CGameObject::SetDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	m_d3dCbvGPUDescriptorHandle = handle;
}

//#define _WITH_DEBUG_FRAME_HIERARCHY
//
//CTexture* CGameObject::FindReplicatedTexture(_TCHAR* pstrTextureName)
//{
//	for (int i = 0; i < m_nMaterials; i++)
//	{
//		if (m_ppMaterials[i])
//		{
//			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
//			{
//				if (m_ppMaterials[i]->m_ppTextures[j])
//				{
//					if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName))) return(m_ppMaterials[i]->m_ppTextures[j]);
//				}
//			}
//		}
//	}
//	CTexture* pTexture = NULL;
//	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
//	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);
//
//	return(NULL);
//}


//void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader)
//{
//	char pstrToken[64] = { '\0' };
//	int nMaterial = 0;
//	UINT nReads = 0;
//
//	m_nMaterials = ReadIntegerFromFile(pInFile);
//
//	m_ppMaterials = new CMaterial * [m_nMaterials];
//	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
//
//	CMaterial* pMaterial = NULL;
//
//	for (; ; )
//	{
//		::ReadStringFromFile(pInFile, pstrToken);
//
//		if (!strcmp(pstrToken, "<Material>:"))
//		{
//			nMaterial = ReadIntegerFromFile(pInFile);
//
//			pMaterial = new CMaterial(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
//
//			if (!pShader)
//			{
//				UINT nMeshType = GetMeshType();
//				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
//				{
//					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
//					{
//						pMaterial->SetSkinnedAnimationShader();
//					}
//					else
//					{
//						pMaterial->SetStandardShader();
//					}
//				}
//			}
//			SetMaterial(nMaterial, pMaterial);
//		}
//		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<SpecularColor>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<Glossiness>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<Smoothness>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<Metallic>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
//		{
//			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
//		{
//			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 3, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<SpecularMap>:"))
//		{
//			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, 4, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<NormalMap>:"))
//		{
//			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<MetallicMap>:"))
//		{
//			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, 6, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<EmissionMap>:"))
//		{
//			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 7, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
//		{
//			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, 8, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
//		{
//			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, 9, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "</Materials>"))
//		{
//			break;
//		}
//	}
//}
//
//CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes)
//{
//	char pstrToken[64] = { '\0' };
//	UINT nReads = 0;
//
//	int nFrame = 0, nTextures = 0;
//
//	CGameObject* pGameObject = new CGameObject();
//
//	for (; ; )
//	{
//		::ReadStringFromFile(pInFile, pstrToken);
//		if (!strcmp(pstrToken, "<Frame>:"))
//		{
//			nFrame = ::ReadIntegerFromFile(pInFile);
//			nTextures = ::ReadIntegerFromFile(pInFile);
//
//			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
//		}
//		else if (!strcmp(pstrToken, "<Transform>:"))
//		{
//			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
//			XMFLOAT4 xmf4Rotation;
//			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
//			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
//			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
//			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
//		}
//		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
//		{
//			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<Mesh>:"))
//		{
//			CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
//			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
//			pGameObject->SetMesh(pMesh);
//		}
//		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
//		{
//			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;
//
//			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
//			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
//			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);
//
//			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
//			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
//
//			pGameObject->SetMesh(pSkinnedMesh);
//		}
//		else if (!strcmp(pstrToken, "<Materials>:"))
//		{
//			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
//		}
//		else if (!strcmp(pstrToken, "<Children>:"))
//		{
//			int nChilds = ::ReadIntegerFromFile(pInFile);
//			if (nChilds > 0)
//			{
//				for (int i = 0; i < nChilds; i++)
//				{
//					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
//					if (pChild) pGameObject->SetChild(pChild);
//#ifdef _WITH_DEBUG_FRAME_HIERARCHY
//					TCHAR pstrDebug[256] = { 0 };
//					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
//					OutputDebugString(pstrDebug);
//#endif
//				}
//			}
//		}
//		else if (!strcmp(pstrToken, "</Frame>"))
//		{
//			break;
//		}
//	}
//	return(pGameObject);
//}
//
//void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent)
//{
//	TCHAR pstrDebug[256] = { 0 };
//
//	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
//	OutputDebugString(pstrDebug);
//
//	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
//	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
//}
//
//void CGameObject::LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel)
//{
//	char pstrToken[64] = { '\0' };
//	UINT nReads = 0;
//
//	int nAnimationSets = 0;
//
//	for (; ; )
//	{
//		::ReadStringFromFile(pInFile, pstrToken);
//		if (!strcmp(pstrToken, "<AnimationSets>:"))
//		{
//			nAnimationSets = ::ReadIntegerFromFile(pInFile);
//			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
//		}
//		else if (!strcmp(pstrToken, "<FrameNames>:"))
//		{
//			pLoadedModel->m_pAnimationSets->m_nBoneFrames = ::ReadIntegerFromFile(pInFile);
//			pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches = new CGameObject * [pLoadedModel->m_pAnimationSets->m_nBoneFrames];
//
//			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; j++)
//			{
//				::ReadStringFromFile(pInFile, pstrToken);
//				pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);
//
//#ifdef _WITH_DEBUG_SKINNING_BONE
//				TCHAR pstrDebug[256] = { 0 };
//				TCHAR pwstrAnimationBoneName[64] = { 0 };
//				TCHAR pwstrBoneCacheName[64] = { 0 };
//				size_t nConverted = 0;
//				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
//				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
//				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
//				OutputDebugString(pstrDebug);
//#endif
//			}
//		}
//		else if (!strcmp(pstrToken, "<AnimationSet>:"))
//		{
//			int nAnimationSet = ::ReadIntegerFromFile(pInFile);
//
//			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name
//
//			float fLength = ::ReadFloatFromFile(pInFile);
//			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
//			int nKeyFrames = ::ReadIntegerFromFile(pInFile);
//
//			pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);
//
//			for (int i = 0; i < nKeyFrames; i++)
//			{
//				::ReadStringFromFile(pInFile, pstrToken);
//				if (!strcmp(pstrToken, "<Transforms>:"))
//				{
//					CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet];
//
//					int nKey = ::ReadIntegerFromFile(pInFile); //i
//					float fKeyTime = ::ReadFloatFromFile(pInFile);
//
//#ifdef _WITH_ANIMATION_SRT
//					m_pfKeyFrameScaleTimes[i] = fKeyTime;
//					m_pfKeyFrameRotationTimes[i] = fKeyTime;
//					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//#else
//					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
//					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
//#endif
//				}
//			}
//		}
//		else if (!strcmp(pstrToken, "</AnimationSets>"))
//		{
//			break;
//		}
//	}
//}
//
//CLoadedModelInfo* CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
//{
//	FILE* pInFile = NULL;
//	::fopen_s(&pInFile, pstrFileName, "rb");
//	::rewind(pInFile);
//
//	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();
//
//	char pstrToken[64] = { '\0' };
//
//	for (; ; )
//	{
//		if (::ReadStringFromFile(pInFile, pstrToken))
//		{
//			if (!strcmp(pstrToken, "<Hierarchy>:"))
//			{
//				pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
//				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
//			}
//			else if (!strcmp(pstrToken, "<Animation>:"))
//			{
//				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
//				pLoadedModel->PrepareSkinning();
//			}
//			else if (!strcmp(pstrToken, "</Animation>:"))
//			{
//				break;
//			}
//		}
//		else
//		{
//			break;
//		}
//	}
//
//#ifdef _WITH_DEBUG_FRAME_HIERARCHY
//	TCHAR pstrDebug[256] = { 0 };
//	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
//	OutputDebugString(pstrDebug);
//
//	CGameObject::PrintFrameInfo(pGameObject, NULL);
//#endif
//
//	return(pLoadedModel);
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//