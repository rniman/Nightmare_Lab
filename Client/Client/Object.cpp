#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"
#include "TextureBlendMesh.h"


vector<shared_ptr<CStandardMesh>> CStandardMesh::g_vAllstandardMesh;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters)
{
	//m_nTextureType = nTextureType;

	m_nTextures = nTextures;
	if (m_nTextures > 0)
	{
		m_vpd3dTextureUploadBuffers.reserve(m_nTextures);
		m_vpd3dTextures.reserve(m_nTextures);

		m_vd3dSrvGpuDescriptorHandles.reserve(m_nTextures);
		m_vnResourceTypes.reserve(m_nTextures);
		m_vdxgiBufferFormats.reserve(m_nTextures);
		m_vnBufferElements.reserve(m_nTextures);
		m_vnBufferStrides.reserve(m_nTextures);

		for (int i = 0; i < m_nTextures; ++i)
		{
			m_vpd3dTextureUploadBuffers.emplace_back();
			m_vpd3dTextures.emplace_back();

			m_vd3dSrvGpuDescriptorHandles.emplace_back();
			m_vnResourceTypes.emplace_back();
			m_vdxgiBufferFormats.emplace_back();
			m_vnBufferElements.emplace_back();
			m_vnBufferStrides.emplace_back();
		}

	}
	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0)
	{
		m_vnRootParameterIndices.reserve(m_nRootParameters);
		for (int i = 0; i < m_nRootParameters; ++i)
		{
			m_vnRootParameterIndices.emplace_back();
		}
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0)
	{
		m_vd3dSamplerGpuDescriptorHandles.reserve(m_nSamplers);
		for (int i = 0; i < m_nSamplers; ++i)
		{
			m_vd3dSamplerGpuDescriptorHandles.emplace_back();
		}
	}
}

CTexture::CTexture(int nTextures, UINT nResourceType, int nSamplers, int nRootParameters, int nSrvGpuHandles, int nUavGpuHandles)
{
	m_nTextures = nTextures;
	if (m_nTextures > 0)
	{
		m_vpd3dTextureUploadBuffers.reserve(m_nTextures);
		m_vpd3dTextures.reserve(m_nTextures);

		m_vnResourceTypes.reserve(m_nTextures);
		m_vdxgiBufferFormats.reserve(m_nTextures);
		m_vnBufferElements.reserve(m_nTextures);
		m_vnBufferStrides.reserve(m_nTextures);

		for (int i = 0; i < m_nTextures; ++i)
		{
			m_vpd3dTextureUploadBuffers.emplace_back();
			m_vpd3dTextures.emplace_back();

			m_vnResourceTypes.emplace_back();
			m_vdxgiBufferFormats.emplace_back();
			m_vnBufferElements.emplace_back();
			m_vnBufferStrides.emplace_back();
		}
	}

	if (nSrvGpuHandles > 0)
	{
		m_vd3dSrvGpuDescriptorHandles.reserve(nSrvGpuHandles);
		for (int i = 0; i < nSrvGpuHandles; ++i)
		{
			m_vd3dSrvGpuDescriptorHandles.emplace_back();
		}
	}

	if (nUavGpuHandles > 0)
	{
		m_vd3dUavGpuDescriptorHandles.reserve(nUavGpuHandles);
		for (int i = 0; i < nUavGpuHandles; ++i)
		{
			m_vd3dUavGpuDescriptorHandles.emplace_back();
		}
	}

	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0)
	{
		m_vnRootParameterIndices.reserve(m_nRootParameters);
		for (int i = 0; i < m_nRootParameters; ++i)
		{
			m_vnRootParameterIndices.emplace_back();
		}
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0)
	{
		m_vd3dSamplerGpuDescriptorHandles.reserve(m_nSamplers);
		for (int i = 0; i < m_nSamplers; ++i)
		{
			m_vd3dSamplerGpuDescriptorHandles.emplace_back();
		}
	}
}



CTexture::~CTexture()
{
	if (!m_vpd3dTextures.empty())
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_vpd3dTextures[i].Get()) m_vpd3dTextures[i].Reset();
		}
	}
	//if (m_vnResourceTypes) delete[] m_pnResourceTypes;
	//if (m_vdxgiBufferFormats) delete[] m_pdxgiBufferFormats;
	//if (m_vnBufferElements) delete[] m_pnBufferElements;

	//if (m_vnRootParameterIndices) delete[] m_pnRootParameterIndices;
	//if (m_vd3dSrvGpuDescriptorHandles) delete[] m_pd3dSrvGpuDescriptorHandles;

	//if (m_vd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootParameterIndex(int nIndex, UINT nRootParameterIndex)
{
	m_vnRootParameterIndices[nIndex] = nRootParameterIndex;
}

void CTexture::SetSrvGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_vd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetUavGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGpuDescriptorHandle)
{
	m_vd3dUavGpuDescriptorHandles[nIndex] = d3dUavGpuDescriptorHandle;
}

void CTexture::UpdateSrvShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nRootParameterIndex, int nDescriptorHandlesIndex)
{
	pd3dCommandList->SetComputeRootDescriptorTable(m_vnRootParameterIndices[0], m_vd3dSrvGpuDescriptorHandles[0]);
}

void CTexture::UpdateUavShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nRootParameterIndex, int nDescriptorHandlesIndex)
{
	pd3dCommandList->SetComputeRootDescriptorTable(nRootParameterIndex, m_vd3dUavGpuDescriptorHandles[nDescriptorHandlesIndex]);
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_vd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nRootParameters == m_nTextures)
	{
		for (int i = 0; i < m_nRootParameters; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_vnRootParameterIndices[i], m_vd3dSrvGpuDescriptorHandles[i]);
		}
	}
	else
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_vnRootParameterIndices[0], m_vd3dSrvGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_vnRootParameterIndices[nParameterIndex], m_vd3dSrvGpuDescriptorHandles[nTextureIndex]);
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::ReleaseUploadBuffers()
{
	if (!m_vpd3dTextureUploadBuffers.empty())
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_vpd3dTextureUploadBuffers[i].Get()) m_vpd3dTextureUploadBuffers[i].Reset();
		}

	}
}

void CTexture::SetName(_TCHAR* s)
{
	_tcscpy(m_sTextureName, s);
	//m_sTextureName = s;
}
_TCHAR* CTexture::GetName()
{
	return m_sTextureName;
}

//void CTexture::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex)
//{
//	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ);
//}

void CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex)
{
	m_vnResourceTypes[nIndex] = nResourceType;
	m_vpd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, m_vpd3dTextureUploadBuffers[nIndex].GetAddressOf(), D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

void CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex)
{
	m_vnResourceTypes[nIndex] = RESOURCE_BUFFER;
	m_vdxgiBufferFormats[nIndex] = ndxgiFormat;
	m_vnBufferElements[nIndex] = nElements;
	m_vnBufferStrides[nIndex] = nStride;
	m_vpd3dTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, m_vpd3dTextureUploadBuffers[nIndex].GetAddressOf());
}


ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	m_vnResourceTypes[nIndex] = nResourceType;
	m_vpd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, nWidth, nHeight, nElements, nMipLevels, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_vpd3dTextures[nIndex].Get());
}

void CTexture::SetTextures(UINT nResourceType, ID3D12Resource* pResource, int idx)
{
	m_vnResourceTypes[idx] = nResourceType;
	m_vpd3dTextures[idx] = pResource;
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	ID3D12Resource* pShaderResource = GetResource(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_vdxgiBufferFormats[nIndex];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_vnBufferElements[nIndex];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

D3D12_UNORDERED_ACCESS_VIEW_DESC CTexture::GetUnorderedAccessViewDesc(int nIndex)
{
	ID3D12Resource* pShaderResource = GetResource(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		d3dUnorderedAccessViewDesc.Texture2DArray.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		d3dUnorderedAccessViewDesc.Texture2DArray.PlaneSlice = 0;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dUnorderedAccessViewDesc.Format = m_vdxgiBufferFormats[nIndex];
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		d3dUnorderedAccessViewDesc.Buffer.FirstElement = 0;
		d3dUnorderedAccessViewDesc.Buffer.NumElements = 0;
		d3dUnorderedAccessViewDesc.Buffer.StructureByteStride = 0;
		d3dUnorderedAccessViewDesc.Buffer.CounterOffsetInBytes = 0;
		d3dUnorderedAccessViewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	}
	return(d3dUnorderedAccessViewDesc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures;

	m_vpTextures.reserve(m_nTextures);
	m_ppstrTextureNames = new _TCHAR[m_nTextures][128];
	for (int i = 0; i < m_nTextures; i++)
	{
		m_vpTextures.push_back(nullptr);
	}
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
}

CMaterial::~CMaterial()
{
	if (m_nTextures > 0)
	{

		if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
	}
}


void CMaterial::SetTexture(const shared_ptr<CTexture>& pTexture, UINT nTexture)
{
	if (m_vpTextures.size() > nTexture)
	{
		if (m_vpTextures[nTexture])
		{
			m_vpTextures[nTexture].reset();
		}
		m_vpTextures[nTexture] = pTexture;
	}
}

void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_vpTextures[i]) m_vpTextures[i]->ReleaseUploadBuffers();
	}
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, VS_CB_OBJECT_INFO* objectInfo)
{
	if (objectInfo) {
		objectInfo->material.Albedo = m_xmf4AlbedoColor;
		objectInfo->material.Ambient = m_xmf4AmbientColor;
		objectInfo->material.Emissive = m_xmf4EmissiveColor;
		objectInfo->material.Specular = m_xmf4SpecularColor;

		objectInfo->gnTexturesMask = m_nType;
	}

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_vpTextures[i]) m_vpTextures[i]->UpdateShaderVariables(pd3dCommandList);
	}
}

// 텍스처를 이곳에 담아 중복되는 텍스처를 줄일것이다.
vector<shared_ptr<CTexture>> CMaterial::m_vTextureContainer;

void CMaterial::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter
	, _TCHAR* pwstrTextureName, shared_ptr<CTexture>& pTexture, shared_ptr<CGameObject> pParent, FILE* pInFile)
{
	char pstrTextureName[128] = { '\0' };

	BYTE nStrLength = 128;
	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
	pstrTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null"))
	{
		SetMaterialType(nType);

		char pstrFilePath[128] = { '\0' };
		char fileHighPath[] = "Asset/Textures/";
		int fileHighPath_Len = strlen(fileHighPath);

		strcpy_s(pstrFilePath, 128, fileHighPath);

		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + fileHighPath_Len, 128 - fileHighPath_Len, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		strcpy_s(pstrFilePath + fileHighPath_Len + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 128 - fileHighPath_Len - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 128, pstrFilePath, _TRUNCATE);
		//if (!_tcscmp(pwstrTextureName, _T("Asset/Textures/Laboratory_Floor_1_Laboratory_Floor_3_AlbedoTransparency.dds"))) {
		//	int x = 0;
		//}
		//#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif
		
		for (auto& pStoredTexture : m_vTextureContainer)
		{
			if (_tcscmp(pwstrTextureName, pStoredTexture->GetName()))
			{ // 이름이 같지 않으면 계속
				continue;
			}
			if (pTexture)
			{
				pTexture.reset();
			}
			pTexture = pStoredTexture;
			return;
		}

		if (!bDuplicated)
		{
			if (pTexture)
			{
				pTexture.reset();
			}
			pTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
			pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, pwstrTextureName, RESOURCE_TEXTURE2D, 0);

			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, nRootParameter);

			pTexture->SetName(pwstrTextureName);
			m_vTextureContainer.push_back(pTexture);
		}
		else
		{
			/*if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
				CGameObject* pRootGameObject = pParent;
				*vpTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
				if (*vpTexture) (*vpTexture)->AddRef();
			}*/
			//else { // parent가 없다면 컨테이너에서 텍스처가 있는지 찾아본다.(로딩시간 증가)
			for (auto& pStoredTexture : m_vTextureContainer)
			{
				if (_tcscmp(pwstrTextureName, pStoredTexture->GetName()))
				{ // 이름이 같지 않으면 계속
					continue;
				}
				if (pTexture)
				{
					pTexture.reset();
				}
				pTexture = pStoredTexture;
				break;
			}
			//}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

#ifdef _WITH_ANIMATION_SRT
	m_nKeyFrameTranslations = nKeyFrames;
	m_pfKeyFrameTranslationTimes = new float[m_nKeyFrameTranslations];
	m_ppxmf3KeyFrameTranslations = new XMFLOAT3 * [m_nKeyFrameTranslations];
	for (int i = 0; i < m_nKeyFrameTranslations; i++) m_ppxmf3KeyFrameTranslations[i] = new XMFLOAT4X4[nAnimatedBones];

	m_nKeyFrameScales = nKeyFrames;
	m_pfKeyFrameScaleTimes = new float[m_nKeyFrameScales];
	m_ppxmf3KeyFrameScales = new XMFLOAT3 * [m_nKeyFrameScales];
	for (int i = 0; i < m_nKeyFrameScales; i++) m_ppxmf3KeyFrameScales[i] = new XMFLOAT4X4[nAnimatedBones];

	m_nKeyFrameRotations = nKeyFrames;
	m_pfKeyFrameRotationTimes = new float[m_nKeyFrameRotations];
	m_ppxmf4KeyFrameRotations = new XMFLOAT3 * [m_nKeyFrameRotations];
	for (int i = 0; i < m_nKeyFrameRotations; i++) m_ppxmf4KeyFrameRotations[i] = new XMFLOAT4X4[nAnimatedBones];
#else

	m_vvxmf4x4KeyFrameTransforms.reserve(m_nKeyFrames);
	for (int i = 0; i < m_nKeyFrames; ++i)
	{
		m_vfKeyFrameTimes.push_back(0.0f);
		m_vvxmf4x4KeyFrameTransforms.emplace_back();
		m_vvxmf4x4KeyFrameTransforms.reserve(nAnimatedBones);
		for (int j = 0; j < nAnimatedBones; ++j)
		{
			m_vvxmf4x4KeyFrameTransforms[i].emplace_back();
		}
	}

	//m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	//for (int subMeshIndex = 0; subMeshIndex < nKeyFrames; subMeshIndex++) m_ppxmf4x4KeyFrameTransforms[subMeshIndex] = new XMFLOAT4X4[nAnimatedBones];
#endif
}

CAnimationSet::~CAnimationSet()
{
#ifdef _WITH_ANIMATION_SRT
	if (m_pfKeyFrameTranslationTimes) delete[] m_pfKeyFrameTranslationTimes;
	for (int j = 0; j < m_nKeyFrameTranslations; j++) if (m_ppxmf3KeyFrameTranslations[j]) delete[] m_ppxmf3KeyFrameTranslations[j];
	if (m_ppxmf3KeyFrameTranslations) delete[] m_ppxmf3KeyFrameTranslations;

	if (m_pfKeyFrameScaleTimes) delete[] m_pfKeyFrameScaleTimes;
	for (int j = 0; j < m_nKeyFrameScales; j++) if (m_ppxmf3KeyFrameScales[j]) delete[] m_ppxmf3KeyFrameScales[j];
	if (m_ppxmf3KeyFrameScales) delete[] m_ppxmf3KeyFrameScales;

	if (m_pfKeyFrameRotationTimes) delete[] m_pfKeyFrameRotationTimes;
	for (int j = 0; j < m_nKeyFrameRotations; j++) if (m_ppxmf4KeyFrameRotations[j]) delete[] m_ppxmf4KeyFrameRotations[j];
	if (m_ppxmf4KeyFrameRotations) delete[] m_ppxmf4KeyFrameRotations;
#else
	//if (m_pfKeyFrameTimes) delete[] m_pfKeyFrameTimes;
	//for (int j = 0; j < m_nKeyFrames; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	//if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;
#endif
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameTranslationTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i + 1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameScaleTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i + 1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameRotationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i + 1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone])), XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i + 1][nBone])), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
#else   
	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_vfKeyFrameTimes[i] <= fPosition) && (fPosition < m_vfKeyFrameTimes[i + 1]))
		{
			float t = (fPosition - m_vfKeyFrameTimes[i]) / (m_vfKeyFrameTimes[i + 1] - m_vfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_vvxmf4x4KeyFrameTransforms[i][nBone], m_vvxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}
	if (fPosition >= m_vfKeyFrameTimes[m_nKeyFrames - 1])
	{
		xmf4x4Transform = m_vvxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];
	}

#endif
	return(xmf4x4Transform);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_vpAnimationSets.reserve(m_nAnimationSets);
	for (int i = 0; i < m_nAnimationSets; ++i)
	{
		m_vpAnimationSets.emplace_back();
	}
}

CAnimationSets::~CAnimationSets()
{
	//for (int subMeshIndex = 0; subMeshIndex < m_nAnimationSets; subMeshIndex++) if (m_pAnimationSets[subMeshIndex]) delete m_pAnimationSets[subMeshIndex];
	//if (m_pAnimationSets) delete[] m_pAnimationSets;

	//if (m_vpBoneFrameCaches) delete[] m_vpBoneFrameCaches;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationTrack::~CAnimationTrack()
{
	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
}

void CAnimationTrack::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationTrack::SetCallbackKey(int nKeyIndex, float fKeyTime, void* pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationTrack::SetAnimationCallbackHandler(const shared_ptr<CAnimationCallbackHandler>& pCallbackHandler)
{
	if (m_pAnimationCallbackHandler)
	{
		m_pAnimationCallbackHandler.reset();
	}
	m_pAnimationCallbackHandler = pCallbackHandler;
}

void CAnimationTrack::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; i++)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData) m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

float CAnimationTrack::UpdatePosition(float fTrackPosition, float fElapsedTime, float fAnimationLength)
{
	float fTrackElapsedTime = fElapsedTime * m_fSpeed;
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		if (m_fPosition < 0.0f) m_fPosition = 0.0f;
		else
		{
			m_fPosition = fTrackPosition + fTrackElapsedTime;

			if (m_fPosition > fAnimationLength && m_fSpeed >= 0.0f)
			{
				m_fPosition = -ANIMATION_CALLBACK_EPSILON;
				return(fAnimationLength);
			}
			else if (m_fPosition < 0.0f && m_fSpeed < 0.0f)
			{
				m_fPosition = fAnimationLength;
				return 0.0f;
			}
		}
		break;
	}
	case ANIMATION_TYPE_ONCE:
		m_fPosition = fTrackPosition + fTrackElapsedTime;
		if (m_fPosition > fAnimationLength) m_fPosition = fAnimationLength;
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	return(m_fPosition);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationController::CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, const shared_ptr<CLoadedModelInfo>& pModel)
{
	m_nAnimationTracks = nAnimationTracks;
	m_vAnimationTracks.reserve(m_nAnimationTracks);
	for (int i = 0; i < m_nAnimationTracks; ++i)
	{
		m_vAnimationTracks.emplace_back();
	}

	m_pAnimationSets = pModel->m_pAnimationSets;

	m_pModelRootObject = pModel->m_pModelRootObject;

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;

	m_vpSkinnedMeshes.reserve(m_nSkinnedMeshes);
	m_vpd3dcbSkinningBoneTransforms.reserve(m_nSkinnedMeshes);
	m_vpcbxmf4x4MappedSkinningBoneTransforms.reserve(m_nSkinnedMeshes);
	m_vd3dCbvSkinningBoneTransformsGPUDescriptorHandle.reserve(m_nSkinnedMeshes);

	//m_vpSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_vpSkinnedMeshes.emplace_back();
		m_vpSkinnedMeshes[i] = pModel->m_vpSkinnedMeshes[i];

		m_vpd3dcbSkinningBoneTransforms.emplace_back();
		m_vpcbxmf4x4MappedSkinningBoneTransforms.emplace_back();
		m_vd3dCbvSkinningBoneTransformsGPUDescriptorHandle.emplace_back();
	}

	//m_vpd3dcbSkinningBoneTransforms = new ID3D12Resource * [m_nSkinnedMeshes];
	//m_vpcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes];
	//m_vd3dCbvSkinningBoneTransformsGPUDescriptorHandle = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_vpd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_vpd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void**)&m_vpcbxmf4x4MappedSkinningBoneTransforms[i]);
		m_vd3dCbvSkinningBoneTransformsGPUDescriptorHandle[i] = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_vpd3dcbSkinningBoneTransforms[i].Get(), ncbElementBytes);
	}
}

CAnimationController::~CAnimationController()
{
	//if (m_vAnimationTracks) delete[] m_vAnimationTracks;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_vpd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_vpd3dcbSkinningBoneTransforms[i]->Release();
	}
	//if (m_vpd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	//if (m_vpcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;
	//if (!m_vpSkinnedMeshes.empty()) delete[] m_vpSkinnedMeshes;
}

void CAnimationController::SetCallbackKeys(int nAnimationTrack, int nCallbackKeys)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetCallbackKeys(nCallbackKeys);
#else
	m_vAnimationTracks[nAnimationTrack].SetCallbackKeys(nCallbackKeys);
#endif	
}

void CAnimationController::SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fKeyTime, void* pData)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetCallbackKey(nKeyIndex, fKeyTime, pData);
#else
	m_vAnimationTracks[nAnimationTrack].SetCallbackKey(nKeyIndex, fKeyTime, pData);
#endif	
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationTrack, const shared_ptr<CAnimationCallbackHandler>& pCallbackHandler)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetAnimationCallbackHandler(pCallbackHandler);
#else
	m_vAnimationTracks[nAnimationTrack].SetAnimationCallbackHandler(pCallbackHandler);
#endif	
}



void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
#else
	m_vAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
#endif	

}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetEnable(bEnable);
#else
	m_vAnimationTracks[nAnimationTrack].SetEnable(bEnable);
#endif	

}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetPosition(fPosition);
#else
	m_vAnimationTracks[nAnimationTrack].SetPosition(fPosition);
#endif		
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
#else
	m_vAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
#endif	
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
#ifdef _DEBUG
	if (m_vAnimationTracks.size() > nAnimationTrack)
		m_vAnimationTracks[nAnimationTrack].SetWeight(fWeight);
#else
	m_vAnimationTracks[nAnimationTrack].SetWeight(fWeight);
#endif	
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_vpSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_vpd3dcbSkinningBoneTransforms[i];
		m_vpSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_vpcbxmf4x4MappedSkinningBoneTransforms[i].get();
		m_vpSkinnedMeshes[i]->m_d3dCbvSkinningBoneTransformsGPUDescriptorHandle = m_vd3dCbvSkinningBoneTransformsGPUDescriptorHandle[i];
	}
}

void CAnimationController::AddBlendWeight(float fBlendWeight)
{
	m_vfBlendWeight.push_back(fBlendWeight);
}

void CAnimationController::SetBlendWeight(int nBlendIndex, float fBlendWeight)
{
	m_vfBlendWeight[nBlendIndex] = fBlendWeight;
}

void CAnimationController::AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject)
{
	//m_fTime += fElapsedTime;
	if (!m_vAnimationTracks.empty())
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_vAnimationTracks[k].m_bEnable)
			{
				shared_ptr<CAnimationSet> pAnimationSet = m_pAnimationSets->m_vpAnimationSets[m_vAnimationTracks[k].m_nAnimationSet];
				float fPosition = m_vAnimationTracks[k].UpdatePosition(m_vAnimationTracks[k].m_fPosition, fElapsedTime, pAnimationSet->m_fLength);
				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
					XMFLOAT4X4 xmf4x4mtx = Matrix4x4::Scale(xmf4x4TrackTransform, m_vAnimationTracks[k].m_fWeight);

					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, xmf4x4mtx);
					m_pAnimationSets->m_vpBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}
				m_vAnimationTracks[k].HandleCallback();
			}
		}

		pRootGameObject->UpdateTransform(NULL);

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}


XMFLOAT4X4 CAnimationController::GetBoneFrameTransform(int index)
{
	return m_pAnimationSets->m_vpBoneFrameCaches[index]->m_xmf4x4ToParent;
}

XMFLOAT3 CAnimationController::GetBoneFramePositionVector(int index)
{
	XMFLOAT4X4* matrix = &m_pAnimationSets->m_vpBoneFrameCaches[index]->m_xmf4x4World;
	return XMFLOAT3{ matrix->_41, matrix->_42, matrix->_43 };
}


XMFLOAT3 CAnimationController::GetBoneFrameLookVector(int index)
{
	XMFLOAT4X4* matrix = &m_pAnimationSets->m_vpBoneFrameCaches[index]->m_xmf4x4World;
	return XMFLOAT3{ matrix->_31, matrix->_32, matrix->_33 };
}

XMFLOAT3 CAnimationController::GetBoneFrameUpVector(int index)
{
	XMFLOAT4X4* matrix = &m_pAnimationSets->m_vpBoneFrameCaches[index]->m_xmf4x4World;
	return XMFLOAT3{ matrix->_21, matrix->_22, matrix->_23 };
}

shared_ptr<CGameObject> CAnimationController::GetBoneFrameObject(int index)
{
	return m_pAnimationSets->m_vpBoneFrameCaches[index];
}

XMFLOAT3 CAnimationController::GetBoneFrameRightVector(int index)
{
	XMFLOAT4X4* matrix = &m_pAnimationSets->m_vpBoneFrameCaches[index]->m_xmf4x4World;
	return XMFLOAT3{ matrix->_11, matrix->_12, matrix->_13 };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLoadedModelInfo::~CLoadedModelInfo()
{
	//	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_vpSkinnedMeshes.reserve(m_nSkinnedMeshes);
	for (int i = 0; i < m_nSkinnedMeshes; ++i)
	{
		m_vpSkinnedMeshes.emplace_back();
	}
	m_pModelRootObject->FindAndSetSkinnedMesh(m_vpSkinnedMeshes, &nSkinnedMesh);

	//m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	//m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_vpSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGameObject::CGameObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
{
	m_xmf4x4ToParent = xmf4x4World;
	m_xmf4x4World = xmf4x4World;
	strcpy(m_pstrFrameName, pstrFrameName);

	if (!pMesh)
	{
		return;
	}
	//m_pMesh = make_shared<CMesh>(pMesh);

	for (const auto& oobb : pMesh->GetVectorOOBB())
	{
		m_voobbOrigin.push_back(oobb);
	}
	if (m_voobbOrigin.size() == 0) {
		int x = 0;
	}
}

CGameObject::CGameObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, const shared_ptr<CMesh>& pMesh)
{
	m_xmf4x4ToParent = xmf4x4World;
	m_xmf4x4World = xmf4x4World;
	strcpy(m_pstrFrameName, pstrFrameName);

	if (!pMesh)
	{
		return;
	}
	//m_pMesh = pMesh;

	for (const auto& oobb : pMesh->GetVectorOOBB())
	{
		m_voobbOrigin.push_back(oobb);
	}
}


CGameObject::CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CGameObject::CGameObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nMaterials)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_vpMaterials.reserve(m_nMaterials);
		for (int i = 0; i < m_nMaterials; ++i)
		{
			m_vpMaterials.emplace_back();
			m_vpMaterials[i] = NULL;
		}
	}

}

CGameObject::~CGameObject()
{
}

void CGameObject::SetChild(const shared_ptr<CGameObject>& pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		// 모델을 불렀을때 문제가있는 부분
		pChild->m_pParent = shared_from_this();
		//if(bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild)
		{
			pChild->m_pSibling = m_pChild->m_pSibling;
		}
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}


void CGameObject::SetMesh(const shared_ptr<CMesh>& pMesh)
{
	m_pMesh = pMesh;
}

void CGameObject::SetMaterial(int nMaterial, const shared_ptr<CMaterial>& pMaterial)
{
	if (m_vpMaterials[nMaterial])
	{
		m_vpMaterials[nMaterial].reset();
	}
	m_vpMaterials[nMaterial] = pMaterial;
}

shared_ptr<CSkinnedMesh> CGameObject::FindSkinnedMesh(char* pstrSkinnedMeshName)
{
	shared_ptr<CSkinnedMesh> pSkinnedMesh;
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		pSkinnedMesh = dynamic_pointer_cast<CSkinnedMesh>(m_pMesh);
		if (!strncmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName, strlen(pstrSkinnedMeshName))) return(pSkinnedMesh);
	}

	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);

	return(NULL);
}

void CGameObject::FindAndSetSkinnedMesh(vector<shared_ptr<CSkinnedMesh>>& vpSkinnedMeshes, int* pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		vpSkinnedMeshes[(*pnSkinnedMesh)++] = dynamic_pointer_cast<CSkinnedMesh>(m_pMesh);
	}

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(vpSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(vpSkinnedMeshes, pnSkinnedMesh);
}

shared_ptr<CGameObject> CGameObject::FindFrame(const char* pstrFrameName)
{
	shared_ptr<CGameObject> pFrameObject;

	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return shared_from_this();

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

shared_ptr<CGameObject> CGameObject::FindFrame(char* pstrFrameName)
{
	shared_ptr<CGameObject>  pFrameObject;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return shared_from_this();

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

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void CGameObject::Animate(float fElapsedTime)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CGameObject::AnimateOOBB()
{
}

void CGameObject::AddOOBB(const vector<XMFLOAT3>& vxmf3Center, const vector<XMFLOAT3>& vxmf3Extents)
{
	for (int i = 0; i < vxmf3Center.size(); ++i)
	{
		BoundingOrientedBox oobbOrigin;
		oobbOrigin.Center = vxmf3Center[i];
		oobbOrigin.Extents = vxmf3Extents[i];

		m_voobbOrigin.push_back(oobbOrigin);
	}
}

void CGameObject::Collide(float fElapsedTime, const shared_ptr<CGameObject>& pGameObject)
{

}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList,char* pstrFramname)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (strcmp(m_pstrFrameName, pstrFramname) != 0) {


		if (m_pMesh)
		{
			if (m_nMaterials > 0)
			{
				for (int subMeshIndex = 0; subMeshIndex < m_nMaterials; subMeshIndex++)
				{
					if (m_vpMaterials[subMeshIndex])
					{
						m_vpMaterials[subMeshIndex]->UpdateShaderVariable(pd3dCommandList, m_cbMappedObject);
					}

					UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

					m_pMesh->Render(pd3dCommandList, subMeshIndex);
				}
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pstrFramname);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pstrFramname);
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);


	if (m_pMesh)
	{
		if (m_nMaterials > 0)
		{
			for (int subMeshIndex = 0; subMeshIndex < m_nMaterials; subMeshIndex++)
			{
				if (m_vpMaterials[subMeshIndex])
				{
					m_vpMaterials[subMeshIndex]->UpdateShaderVariable(pd3dCommandList, m_cbMappedObject);
				}

				UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

				m_pMesh->Render(pd3dCommandList, subMeshIndex);
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList);
	if (m_pChild) m_pChild->Render(pd3dCommandList);
}

void CGameObject::RenderOpaque(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh)
	{
		if (m_nMaterials > 0)
		{
			for (int subMeshIndex = 0; subMeshIndex < m_nMaterials; subMeshIndex++)
			{
				if (find(m_vTransparentMaterialNumbers.begin(), m_vTransparentMaterialNumbers.end(), subMeshIndex) != m_vTransparentMaterialNumbers.end()) {
					continue;
				}
				if (m_vpMaterials[subMeshIndex])
				{
					m_vpMaterials[subMeshIndex]->UpdateShaderVariable(pd3dCommandList, m_cbMappedObject);
				}

				UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

				m_pMesh->Render(pd3dCommandList, subMeshIndex);
			}
		}
	}

	if (m_pSibling) m_pSibling->RenderOpaque(pd3dCommandList);
	if (m_pChild) m_pChild->RenderOpaque(pd3dCommandList);
}

void CGameObject::RenderTransparent(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh)
	{
		if (m_nMaterials > 0)
		{
			for (int subMeshIndex = 0; subMeshIndex < m_nMaterials; subMeshIndex++)
			{
				if (find(m_vTransparentMaterialNumbers.begin(), m_vTransparentMaterialNumbers.end(), subMeshIndex) == m_vTransparentMaterialNumbers.end()) {
					continue;
				}
				if (m_vpMaterials[subMeshIndex])
				{
					m_vpMaterials[subMeshIndex]->UpdateShaderVariable(pd3dCommandList, m_cbMappedObject);
				}

				UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

				m_pMesh->Render(pd3dCommandList, subMeshIndex);
			}
		}
	}

	if (m_pSibling) m_pSibling->RenderTransparent(pd3dCommandList);
	if (m_pChild) m_pChild->RenderTransparent(pd3dCommandList);
}


void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	UINT ncbElementBytes = ((sizeof(VS_CB_OBJECT_INFO) + 255) & ~255); //256의 배수
	m_d3dcbvObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_d3dcbvObject->Map(0, NULL, (void**)&m_cbMappedObject);

	m_d3dCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_d3dcbvObject.Get(), ncbElementBytes);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&m_cbMappedObject->xmf4x4world, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRootDescriptorTable(1, GetDescriptorHandle());
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_vpMaterials[i]) m_vpMaterials[i]->ReleaseUploadBuffers();
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

void CGameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4ToParent._41 += xmf3Offset.x;
	m_xmf4x4ToParent._42 += xmf3Offset.y;
	m_xmf4x4ToParent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
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

shared_ptr<CTexture> CGameObject::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_vpMaterials[i])
		{
			for (int j = 0; j < m_vpMaterials[i]->m_nTextures; j++)
			{
				if (m_vpMaterials[i]->m_vpTextures[j])
				{
					if (!_tcsncmp(m_vpMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName)))
					{
						return(m_vpMaterials[i]->m_vpTextures[j]);
					}
				}
			}
		}
	}
	shared_ptr<CTexture>  pTexture;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

UINT CGameObject::GetMeshType()
{
	if (m_pMesh)
	{
		return m_pMesh->GetType();
	}
	return 0x00;
}

void CGameObject::SetLookAt(XMFLOAT3& xmf3target)
{
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);
	XMFLOAT3 position(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	XMFLOAT3 look = Vector3::Subtract(xmf3target, position);
	XMFLOAT3 right = Vector3::CrossProduct(up, look);
	m_xmf4x4World._11 = right.x;m_xmf4x4World._12 = right.y;m_xmf4x4World._13 = right.z;
	m_xmf4x4World._21 = up.x;m_xmf4x4World._22 = up.y;m_xmf4x4World._23 = up.z;
	m_xmf4x4World._31 = look.x;m_xmf4x4World._32 = look.y;m_xmf4x4World._33 = look.z;

	m_xmf4x4ToParent._41 = 0.0f;m_xmf4x4ToParent._42 = 0.0f;m_xmf4x4ToParent._43 = 0.0f;
	m_xmf4x4World = Matrix4x4::Multiply(m_xmf4x4ToParent, m_xmf4x4World);
}

void CGameObject::ObjectCopy(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,shared_ptr<CGameObject> srcobject)
{
	SetMesh(srcobject->m_pMesh);

	m_nMaterials = srcobject->m_nMaterials;
	for (int i = 0; i < srcobject->m_nMaterials;++i) {
		m_vpMaterials.push_back(srcobject->m_vpMaterials[i]);
	}
	memcpy(&m_xmf4x4ToParent, &srcobject->m_xmf4x4ToParent, sizeof(XMFLOAT4X4));
	strcpy(m_pstrFrameName, srcobject->m_pstrFrameName);

	if (srcobject->m_pChild) {
		// child 복사
		m_pChild = make_shared<CGameObject>(pd3dDevice, pd3dCommandList);
		m_pChild->ObjectCopy(pd3dDevice, pd3dCommandList, srcobject->m_pChild);
	}
	if (srcobject->m_pSibling) {
		// sibling 복사
		m_pSibling = make_shared<CGameObject>(pd3dDevice, pd3dCommandList);
		m_pSibling->ObjectCopy(pd3dDevice, pd3dCommandList, srcobject->m_pSibling);
	}
}


void CGameObject::InstanceObjectCopy(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<CGameObject> srcobject)
{
	SetMesh(srcobject->m_pMesh);

	m_nMaterials = srcobject->m_nMaterials;
	for (int i = 0; i < srcobject->m_nMaterials;++i) {
		m_vpMaterials.push_back(srcobject->m_vpMaterials[i]);
	}
	memcpy(&m_xmf4x4ToParent, &srcobject->m_xmf4x4ToParent, sizeof(XMFLOAT4X4));
	strcpy(m_pstrFrameName, srcobject->m_pstrFrameName);

	if (srcobject->m_pChild) {
		// child 복사
		m_pChild = make_shared<CInstanceObject>(pd3dDevice, pd3dCommandList);
		m_pChild->ObjectCopy(pd3dDevice, pd3dCommandList, srcobject->m_pChild);
	}
	if (srcobject->m_pSibling) {
		// sibling 복사
		m_pSibling = make_shared<CInstanceObject>(pd3dDevice, pd3dCommandList);
		m_pSibling->ObjectCopy(pd3dDevice, pd3dCommandList, srcobject->m_pSibling);
	}
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<CGameObject> pParent, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	m_vpMaterials.reserve(m_nMaterials);
	for (int i = 0; i < m_nMaterials; i++)
	{
		m_vpMaterials.emplace_back();
		//m_vpMaterials[subMeshIndex] = NULL;
	}

	shared_ptr<CMaterial> pMaterial;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			//pMaterial = new CMaterial(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
			pMaterial = make_shared<CMaterial>(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

			/*if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();
					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}*/
			SetMaterial(nMaterial, pMaterial);
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 3, pMaterial->m_ppstrTextureNames[0], (pMaterial->m_vpTextures[0]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			m_vpMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, 4, pMaterial->m_ppstrTextureNames[1], (pMaterial->m_vpTextures[1]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			m_vpMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[2], (pMaterial->m_vpTextures[2]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			m_vpMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, 6, pMaterial->m_ppstrTextureNames[3], (pMaterial->m_vpTextures[3]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			m_vpMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 7, pMaterial->m_ppstrTextureNames[4], (pMaterial->m_vpTextures[4]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		{
			m_vpMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, 8, pMaterial->m_ppstrTextureNames[5], (pMaterial->m_vpTextures[5]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		{
			m_vpMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, 9, pMaterial->m_ppstrTextureNames[6], (pMaterial->m_vpTextures[6]), pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
}

vector<shared_ptr<CMesh>> CGameObject::m_vMeshContainer;

shared_ptr<CGameObject> CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, shared_ptr<CGameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes, MeshType meshtype)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	//pGameObject = new CGameObject(pd3dDevice, pd3dCommandList);
	shared_ptr<CGameObject> pGameObject = make_shared<CGameObject>(pd3dDevice, pd3dCommandList);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);

			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			shared_ptr<CStandardMesh> pMesh;

			switch (meshtype)
			{
			case MeshType::Standard:
				pMesh = make_shared<CStandardMesh>(pd3dDevice, pd3dCommandList);
				break;
			case MeshType::Blend:
				pMesh = make_shared<TextureBlendMesh>(pd3dDevice, pd3dCommandList);
				break;
			default:
				break;
			}

			if (!pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile))
			{
				for (auto pStoredMesh : m_vMeshContainer)
				{
					if (!strcmp(pStoredMesh->m_pstrMeshName, pMesh->m_pstrMeshName + 1))
					{
						pMesh = dynamic_pointer_cast<CStandardMesh>(pStoredMesh);
						break;
					}
				}

			}
			else
			{
				m_vMeshContainer.push_back(pMesh);
			}
			pGameObject->SetMesh(pMesh);
		}
		//else if (!strcmp(pstrToken, "<InstanceMesh>:"))
		//{
		//	CInstanceStandardMesh* pMesh = new CInstanceStandardMesh(pd3dDevice, pd3dCommandList);
		//	pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile, pGameObject);
		//	pGameObject->SetMesh(pMesh);
		//}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			//CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			shared_ptr<CSkinnedMesh> pSkinnedMesh = make_shared<CSkinnedMesh>(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					shared_ptr<CGameObject> pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pnSkinnedMeshes, meshtype);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

shared_ptr<CGameObject> CGameObject::LoadInstanceFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, shared_ptr<CGameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	shared_ptr<CInstanceObject> pGameObject = make_shared<CInstanceObject>(pd3dDevice, pd3dCommandList);
	//new CInstanceObject(pd3dDevice, pd3dCommandList);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);

			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			shared_ptr<CStandardMesh> pMesh = make_shared<CStandardMesh>(pd3dDevice, pd3dCommandList);
			if (!pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile))
			{
				for (auto pStoredMesh : m_vMeshContainer)
				{
					if (!strcmp(pStoredMesh->m_pstrMeshName, pMesh->m_pstrMeshName + 1))
					{
						pMesh = dynamic_pointer_cast<CStandardMesh>(pStoredMesh);
						break;
					}
				}

			}
			else {
				m_vMeshContainer.push_back(pMesh);
			}
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<InstanceMesh>:"))
		{
			shared_ptr<CInstanceStandardMesh> pMesh = make_shared<CInstanceStandardMesh>(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile, pGameObject);
			CStandardMesh::SaveStandardMesh(pMesh);
			//pMesh->m_pOriginInstance = pGameObject;
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<BoxColliders>:"))
		{
			vector<XMFLOAT3> vxmf3AABBCenter, vxmf3AABBExtents;
			int nBoundingBox = ::ReadIntegerFromFile(pInFile);
			for (int i = 0; i < nBoundingBox; ++i)
			{
				int nIndex = 0;
				XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
				::ReadStringFromFile(pInFile, pstrToken); // Bound
				nReads = fread(&nIndex, sizeof(int), 1, pInFile);
				nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
				nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
				vxmf3AABBCenter.push_back(xmf3AABBCenter);
				vxmf3AABBExtents.push_back(xmf3AABBExtents);
			}
			int nChild = 0;
			::fread(&nChild, sizeof(int), 1, pInFile);
			XMFLOAT4X4* pxmf4x4InstanceTransformMatrix = new XMFLOAT4X4[nChild];
			nReads = (UINT)::fread(pxmf4x4InstanceTransformMatrix, sizeof(XMFLOAT4X4), nChild, pInFile);
			for (int i = 0; i < nChild; ++i)
			{
				char pFrameName[] = "Wall_BoundingBox";
				XMFLOAT4X4 xmf4x4TranposeMatrix = Matrix4x4::Transpose(pxmf4x4InstanceTransformMatrix[i]);
				shared_ptr<CGameObject> pBoundinBoxObject = make_shared<CGameObject>(pFrameName, xmf4x4TranposeMatrix, nullptr);
				pBoundinBoxObject->AddOOBB(vxmf3AABBCenter, vxmf3AABBExtents);
				pGameObject->m_vInstanceObjectInfo.push_back(pBoundinBoxObject);
				g_collisionManager.AddCollisionObject(pBoundinBoxObject);
			}
			delete[] pxmf4x4InstanceTransformMatrix;
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			shared_ptr<CSkinnedMesh> pSkinnedMesh = make_shared<CSkinnedMesh>(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					shared_ptr<CGameObject> pChild = CGameObject::LoadInstanceFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

void CGameObject::PrintFrameInfo(const shared_ptr<CGameObject>& pGameObject, const shared_ptr<CGameObject>& pParent)
{
	TCHAR pstrDebug[256] = { 0 };
	
	//_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	//OutputDebugString(pstrDebug);
	
	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

void CGameObject::LoadAnimationFromFile(FILE* pInFile, const shared_ptr<CLoadedModelInfo>& pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = make_shared<CAnimationSets>(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nBoneFrames = ::ReadIntegerFromFile(pInFile);
			//pLoadedModel->m_pAnimationSets->m_vpBoneFrameCaches = new CGameObject * [pLoadedModel->m_pAnimationSets->m_nBoneFrames];
			pLoadedModel->m_pAnimationSets->m_vpBoneFrameCaches.reserve(pLoadedModel->m_pAnimationSets->m_nBoneFrames);

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				pLoadedModel->m_pAnimationSets->m_vpBoneFrameCaches.emplace_back();
				pLoadedModel->m_pAnimationSets->m_vpBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			pLoadedModel->m_pAnimationSets->m_vpAnimationSets[nAnimationSet] = make_shared<CAnimationSet>(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					shared_ptr<CAnimationSet> pAnimationSet = pLoadedModel->m_pAnimationSets->m_vpAnimationSets[nAnimationSet];

					int nKey = ::ReadIntegerFromFile(pInFile); //subMeshIndex
					float fKeyTime = ::ReadFloatFromFile(pInFile);

#ifdef _WITH_ANIMATION_SRT
					m_pfKeyFrameScaleTimes[i] = fKeyTime;
					m_pfKeyFrameRotationTimes[i] = fKeyTime;
					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
#else
					pAnimationSet->m_vfKeyFrameTimes[i] = fKeyTime;
					//for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; ++j)
					//{
					//	nReads = (UINT)::fread(&pAnimationSet->m_vvxmf4x4KeyFrameTransforms[subMeshIndex][j], sizeof(XMFLOAT4X4), 1, pInFile);
					//}

					nReads = (UINT)::fread(pAnimationSet->m_vvxmf4x4KeyFrameTransforms[i].data(), sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}
 
shared_ptr<CLoadedModelInfo> CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, MeshType meshtype)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, &pLoadedModel->m_nSkinnedMeshes, meshtype);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pLoadedModel->m_pModelRootObject, NULL);
#endif

	return(pLoadedModel);
}


void CGameObject::LoadBoundingBox(vector<BoundingOrientedBox>& voobbOrigin)
{
	if (m_pMesh)
	{
		for (const auto& oobb : m_pMesh->GetVectorOOBB())
		{
			voobbOrigin.push_back(oobb);
		}
	}

	if (m_pSibling) m_pSibling->LoadBoundingBox(voobbOrigin);
	if (m_pChild) m_pChild->LoadBoundingBox(voobbOrigin);
}

bool CGameObject::CheckPicking(const weak_ptr<CGameObject>& pGameObject, const XMFLOAT3& xmf3PickPosition, const XMFLOAT4X4& xmf4x4ViewMatrix, float& fDistance)
{
	shared_ptr<CGameObject> pCollisionGameObject = pGameObject.lock();
	if (!pCollisionGameObject || !pCollisionGameObject->GetCollision())
	{
		return false;
	}

	XMFLOAT4X4 xmf4x4WorldMatrix = pCollisionGameObject->m_xmf4x4World;
	XMFLOAT4X4 xmf4x4ModelMatrix;
	XMStoreFloat4x4(&xmf4x4ModelMatrix, XMMatrixInverse(nullptr, XMLoadFloat4x4(&xmf4x4WorldMatrix) * XMLoadFloat4x4(&xmf4x4ViewMatrix)));
	XMMATRIX xmmtModelMatrix = XMMatrixInverse(nullptr, XMLoadFloat4x4(&xmf4x4WorldMatrix) * XMLoadFloat4x4(&xmf4x4ViewMatrix));

	XMFLOAT3 xmf3CameraPosition{ 0.0f, 0.0f, 0.0f };
	XMVECTOR xmvCameraOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraPosition), xmmtModelMatrix);
	XMFLOAT3 xmf3CameraOrigin;
	XMStoreFloat3(&xmf3CameraOrigin, xmvCameraOrigin);

	XMVECTOR xmvPickDirection = XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&xmf3PickPosition), xmmtModelMatrix) - xmvCameraOrigin);
	XMFLOAT3 xmf3PickDirection;
	XMStoreFloat3(&xmf3PickDirection, xmvPickDirection);

	//for (auto& OOBB : pGameObject->m_OOBB)
	//{
	//	if (OOBB.Intersects(xmvCameraOrigin, xmvPickDirection, fDistance))
	//	{
	//		return true;
	//	}
	//}

	for (auto& oobb : pCollisionGameObject->GetVectorOOBB())
	{
		XMFLOAT4X4 xmf4x4ToModel = Matrix4x4::Inverse(pCollisionGameObject->m_xmf4x4ToParent);
		XMMATRIX mtxToModel =  XMLoadFloat4x4(&xmf4x4ToModel);
		if (oobb.Intersects(xmvCameraOrigin, xmvPickDirection, fDistance))
		{
			return true;
		}
	}

	//if (pGameObject->m_oobbOrigin.Intersects(xmvCameraOrigin, xmvPickDirection, fDistance))
	//{
	//	return true;
	//}

	/*if (pGameObject->m_OOBB[0].Intersects(xmf3CameraOrigin, xmvPickDirection, fDistance))
	{
		return true;
	}*/

	//if (pGameObject->m_pChild)
	//{
	//	return CheckPicking(pGameObject->m_pChild, xmf3PickPosition, xmf4x4ViewMatrix, fDistance);
	//}

	//if (pGameObject->m_pSibling)
	//{
	//	return CheckPicking(pGameObject->m_pSibling, xmf3PickPosition, xmf4x4ViewMatrix, fDistance);
	//}

	return false;
}

void CGameObject::SetTransparentObjectInfo(vector<int> vNumbers)
{
	m_bThisContainTransparent = true;
	m_vTransparentMaterialNumbers = vNumbers;

	if (m_pSibling) {
		m_pSibling->SetTransparentObjectInfo(vNumbers);
	}
	if (m_pChild) {
		m_pChild->SetTransparentObjectInfo(vNumbers);
	}
}

CHexahedronObject::CHexahedronObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nMaterials) 
	: CGameObject(pd3dDevice,  pd3dCommandList,  nMaterials)
{
	shared_ptr<CTexture> pTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/Textures/나뭇잎.dds", RESOURCE_TEXTURE2D, 0);
	//if (pTexture)pTexture->AddRef();

	m_vpMaterials[0] = make_shared<CMaterial>(1); // 텍스처가 1개
	m_vpMaterials[0]->SetMaterialType(MATERIAL_ALBEDO_MAP);
	m_vpMaterials[0]->SetTexture(pTexture, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 3); // 3은 루트시그니처의 Albedo에 해당하는 인덱스
}

CHexahedronObject::~CHexahedronObject()
{
}

int cntCbvModelObject(const shared_ptr<CGameObject>& pGameObject, int nCnt)
{
	nCnt++;
	if (pGameObject->m_pSibling) nCnt = cntCbvModelObject(pGameObject->m_pSibling, nCnt);
	if (pGameObject->m_pChild) nCnt = cntCbvModelObject(pGameObject->m_pChild, nCnt);

	return nCnt;
}

CInstanceObject::CInstanceObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
}

void CInstanceObject::Animate(float fElapsedTime)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	for (auto& instanceObject : m_vInstanceObjectInfo)
	{
		instanceObject->Animate(fElapsedTime);
	}

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CInstanceObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::Render(pd3dCommandList);
}

CFullScreenTextureObject::CFullScreenTextureObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<CMaterial>& material)
	: CGameObject(pd3dDevice,pd3dCommandList)
{
	// 메테리얼을 넘겨받아서 크기를 할당한다.
	m_nMaterials = 1;
	m_vpMaterials.reserve(m_nMaterials);
	m_vpMaterials.emplace_back();
	m_vpMaterials[0] = material;

	m_cbMappedObject->option.alphaValue = 1.0f;
	m_fSetAlpha = 1.0f;

	m_bRender = false;
}

void CFullScreenTextureObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!m_bRender) {
		return;
	}

	if (m_Component) {

		switch (m_Component->GetComponentType())
		{
		case Component::TIMEONOFF: {
			float fTime = static_pointer_cast<ComponentTimeOnOff>(m_Component)->GetTime();
			float fSetTime = static_pointer_cast<ComponentTimeOnOff>(m_Component)->GetSetTime();
			m_cbMappedObject->option.alphaValue = (fTime / fSetTime) * m_fSetAlpha;
			break;
		}
		default:
			assert(0);
		}

		m_Component->Update();
	}

	m_vpMaterials[0]->UpdateShaderVariable(pd3dCommandList, m_cbMappedObject);

	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void CFullScreenTextureObject::SetAlphaValue(float val)
{
	m_cbMappedObject->option.alphaValue = val;
	m_fSetAlpha = val;
}

void CFullScreenTextureObject::SetComponent(shared_ptr<Component> component)
{
	m_Component = component;
}