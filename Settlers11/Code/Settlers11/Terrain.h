#pragma once


#include "D3DCompat.h"
#include <xnamath.h>
#include <random>

struct Float3
{
	Float3()
	{
		f[0] = 0;
		f[1] = 0;
		f[2] = 0;
	}

	Float3(float x, float y, float z)
	{
		f[0] = x; f[1] = y; f[2] = z;
	}

	float& operator[] (const int nIndex)
	{
		return f[nIndex];
	}

	Float3 operator-(Float3& other)
	{
		return Float3(other.f[0] - f[0], other.f[1] - f[1], other.f[2] - f[2]);
	}

	Float3& operator+=(Float3& other)
	{
		f[0] += other.f[0];
		f[1] += other.f[1];
		f[2] += other.f[2];
		return *this;
	}

	Float3& operator/=(float scalar)
	{
		f[0] /= scalar;
		f[1] /= scalar;
		f[2] /= scalar;
		return *this;
	}

	Float3 Cross(Float3& other)
	{
		return Float3(	f[1] * other.f[2] - f[2] * other.f[1],
			f[2] * other.f[0] - f[0] * other.f[2],
			f[0] * other.f[1] - f[1] * other.f[0]);
	}

	void Normalize()
	{
		float oneOverLength = 1 / sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
		f[0] *= oneOverLength;
		f[1] *= oneOverLength;
		f[2] *= oneOverLength;
	}

	float Length()
	{
		return sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
	}

	float f[3];
};

struct TerrainVertex
{
	Float3 position;
	Float3 normal;
	Float3 blendWeights;
	char matIDs[3];
	//uint hIdx;
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
