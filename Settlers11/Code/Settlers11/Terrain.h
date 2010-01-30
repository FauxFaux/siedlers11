#pragma once


#include "D3DCompat.h"
#include <xnamath.h>
#include <random>

struct TerrainVertex
{
	float x, z;
	int hIdx;
	//uint matIDs[3];
	//float bX, bY, bZ;
	//float nX, nY, nZ;
	//float zzz[16];
	//uint heightIndex;
};

struct TextureScaleInfo
{
	float scaleX, scaleY;
};

class Terrain
{
public:
	Terrain(ID3DDevice* device, ID3DDeviceContext* context, char* mapFile);
	~Terrain(void);

	uint GetPickedVertex(XMVECTOR cameraPos, XMVECTOR ray);

	void Update(ID3DDeviceContext& context, XMVECTOR& cameraPosition, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj);
	void Draw(ID3DDeviceContext& context, XMMATRIX& matrix);

private:
	ID3DBuffer* mVB;
	ID3DBuffer* mIB;
	ID3DInputLayout* mInputLayout;
	ID3DVertexShader* mVS;
	ID3DPixelShader* mFS;
	ID3DBuffer* mCB;
	ID3DSamplerState* mSampler;

	ID3DBlendState* mWaterBlendState;

	ID3DTexture2D* mTerrainTexture;
	ID3DShaderResourceView* mTerrainView;

	TerrainVertex* mVertices;

	TextureScaleInfo* mScaleInfo;
	ID3DBuffer* mScaleBuffer;
	ID3DShaderResourceView* mScaleView;

	int* mHeights;
	int mNumHeights;
	ID3DBuffer* mHeightBuffer;
	ID3DShaderResourceView* mHeightView;

	ID3DBuffer* mWaterVB;
	ID3DPixelShader* mWaterFS;

	uint mLogicalWidth, mLogicalHeight;
	uint mFineWidth, mFineHeight;

	UINT mNumIndices;
	UINT mVBStride;
	DXGI_FORMAT mIBFormat;
};
