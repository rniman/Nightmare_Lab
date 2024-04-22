#pragma once

struct FrameTimeInfo;

class TextureBlendObject :   public CGameObject
{
public:
	float m_fLocalTime = 0.0f;
	FrameTimeInfo*					m_pcbMappedTime;
private:
	shared_ptr<CPlayer> m_pPlayer;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dTimeCbvGPUDescriptorHandle;
	ComPtr<ID3D12Resource>			m_pd3dcbTime;
public:
	TextureBlendObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<CGameObject> object,shared_ptr<CPlayer> player);
	~TextureBlendObject();

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

};
