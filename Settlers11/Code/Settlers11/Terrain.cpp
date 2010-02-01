
#include "Terrain.h"
#include <assert.h>
#include <stdio.h>
#include <xnamath.h>

extern HWND g_hWnd;
extern ID3D10Blob* g_vsBlob, *g_fsBlob;

struct PatternInfo
{
	uint id;
	char* texturePath;	
	float scaleX, scaleY;
	bool loaded;
};

PatternInfo patternInfos[] = 
{ 
	{ 0xBABEB00B, "z_seaground_00", 0.06f, 0.06f },
	{ 0xFA1CA583, "m_rock_01", 0.09f, 0.09f },
	{ 0xFA1CA584, "m_rock_01", 0.085f, 0.085f },
	{ 0xFA1CA585, "m_rock_01", 0.04f, 0.04f },
	{ 0xBADEB00F, "z_sand_01", 0.06f, 0.06f },
	{ 0xFA1CA589, "m_rock_09", 0.075f, 0.075f },
	{ 0xBADEB012, "z_sand_01", 0.085f, 0.085f },
	{ 0xFA1CA586, "m_rock_09", 0.045f, 0.045f },
	{ 0xBADEB010, "z_sand_01", 0.085f, 0.085f },
	{ 0xFA1CA588, "m_rock_08", 0.075f, 0.075f },
	{ 0xFA1CA587, "m_rock_07", 0.075f, 0.075f },
	{ 0xBADEB011, "z_sand_01", 0.085f, 0.085f },
	{ 0x4545FAC9, "z_meadow_09", 0.08f, 0.08f },
	{ 0xFA1CA571, "m_ground_01", 0.07f, 0.07f },
	{ 0xBADEB00D, "z_sand_00", 0.085f, 0.085f },
	{ 0xFA1CA570, "m_ground_00", 0.08f, 0.08f },
	{ 0xBADEB013, "z_sand_01", 0.085f, 0.085f },
	{ 0xBADEB00E, "z_seaground_00", 0.06f, 0.06f },
	{ 0x4545FAC2, "z_meadow_02", 0.08f, 0.08f },
	{ 0x013374E5, "z_sand_00", 0.085f, 0.085f },
	{ 0xFA1CA562, "z_meadow_02", 0.07f, 0.07f },
	{ 0xDECADE01, "z_pavement_00", 0.15f, 0.15f },
	{ 0x4545FAC4, "z_meadow_04", 0.13f, 0.13f },
	{ 0xCAFECAFE, "z_rock_00", 0.13f, 0.13f },
	{ 0x4545FAC5, "z_meadow_05", 0.13f, 0.13f },
	{ 0xDEADBEEF, "z_rock_00", 0.062f, 0.062f },
	{ 0xBFE4E8E3, "z_meadow_00", 0.065f, 0.065f },
	{ 0xFA1CA560, "m_meadow_00", 0.08f, 0.08f },
	{ 0xFA1CA563, "m_meadow_02", 0.08f, 0.08f },
	{ 0x777FA8C0, "z_earth_01", 0.08f, 0.08f },
	{ 0x4545FAC3, "z_meadow_03", 0.055f, 0.055f },
	{ 0xD00FAFFE, "z_rock_00", 0.09f, 0.09f },
	{ 0xFA1CA561, "m_meadow_01", 0.07f, 0.07f },
	{ 0x4545FAC6, "z_meadow_06", 0.13f, 0.13f },
	{ 0xDE5E1110, "z_ground_00", 0.08f, 0.08f },
	{ 0xCAFECB04, "z_rock_05", 0.075f, 0.075f },
	{ 0xCAFECAFF, "z_rock_03", 0.075f, 0.075f },
	{ 0x680004E4, "z_swamp_00", 0.035f, 0.035f },
	{ 0x680004E5, "z_swamp_01", 0.07f, 0.07f },
	{ 0xFA1CA58A, "m_rock_10", 0.075f, 0.075f },
	{ 0x4545FAC7, "z_meadow_07", 0.06f, 0.06f },
	{ 0xCAFECB02, "z_rock_03", 0.045f, 0.045f },
	{ 0x0FADE0FF, "z_snow_00", 0.1f, 0.1f },
	{ 0xCAFECB05, "z_rock_06", 0.075f, 0.075f },
	{ 0x013374E6, "z_seaground_03", 0.13f, 0.13f },
	{ 0xCAFECB03, "z_rock_04", 0.075f, 0.075f },
	{ 0x4545FAC1, "z_meadow_01", 0.06f, 0.06f },
};

//ID3DShaderResourceView* views[sizeof(patternInfos) / sizeof(PatternInfo)];



char GetIndex(uint pattern)
{
	static const char numPatterns = sizeof(patternInfos) / sizeof(PatternInfo);

	for(char i = 0; i < numPatterns; i++)
	{
		if(patternInfos[i].id == pattern)
			return i;
	}

	return 0xFF;
}

uint GetPatternArrayOffset(int x, int z, int vIdx, int width)
{
	switch(vIdx)
	{
	case 0: 
		{
			if(x % 2 == z % 2)
			{
				return (x / 2) + (z * width);
			}
			else
			{
				return (x / 2) + ((z+1) * width);
			}
		} break;
	case 1:
		{
			if(x % 2 == z % 2)
			{
				return (x / 2) + 1 + (z * width);
			}
			else
			{
				return (x / 2) + 1 + ((z + 1) * width);
			}
		}break;
	case 2:
		{
			if(x % 2 == z % 2)
			{
				return (x / 2) + ((z + 1) * width) + (x % 2);
			}
			else
			{
				return (x / 2) + (z * width) + (x % 2);
			}
		}break;
	}

	assert(false);
	return 0;
}

uint GetHeightArrayOffset(int x, int z, int row, int col, int fineWidth)
{
	static const uint offsets[] = { 0, 0, 1, 1, 2 };
	
	uint heightOffset = (z % 2 == x % 2) ? ((z * 4) + row) * fineWidth : (((z + 1) * 4) - row) * fineWidth;

	if(x % 2 == 0)
		heightOffset += offsets[row];
	else
		heightOffset += offsets[4- row];

	uint o = (x % 2 == 1) ? row : 0;
	return heightOffset + col + o + ((x / 2) * 4);
}

const float xDiff = 1.5f;
const float zDiff = 1.299f;

void CalculateNormal(uint fineX, uint fineZ, int* heights, uint fineWidth, uint fineHeight, Float3& normal)
{
	Float3 total(0,0,0);

	Float3 origin(0,0,0);

	Float3 points[6];	//Up right, right, down right, down left, left, up left.
	bool valid[6] = {0};

	points[0] = Float3(0.5f * xDiff, 0, zDiff);
	points[1] = Float3(xDiff, 0, 0);
	points[2] = Float3(0.5f * xDiff, 0, -zDiff);
	points[3] = Float3(-0.5f * xDiff, 0, -zDiff);
	points[4] = Float3(-xDiff, 0, 0);
	points[5] = Float3(-0.5f* xDiff, 0, zDiff);

	uint originIndex = fineX + fineZ * fineWidth;

	origin[1] = heights[originIndex] / 1000.0f;

	if(fineZ < fineHeight - 1)
	{
		if(fineZ % 8 < 4)
		{
			points[0][1] = heights[originIndex + fineWidth] / 1000.0f;	valid[0] = true;
		}
		else
		{
			if(fineX < fineWidth - 1)
				points[0][1] = heights[originIndex + fineWidth + 1] / 1000.0f;
		}
	}

	if(fineX < fineWidth - 1)		
	{
		points[1][1] = heights[originIndex + 1] / 1000.0f; valid[1] = true;
	}

	if(fineZ > 0)
	{
		if(fineZ % 8 >= 1 && fineZ % 8 <= 4) 
		{
			points[2][1] = heights[originIndex - fineWidth + 1] / 1000.0f;
		}
		else
		{
			points[2][1] = heights[originIndex - fineWidth] / 1000.0f;
		}
		valid[2] = true;
	}

	if(fineZ > 0)
	{
		if(fineZ % 8 >= 1 && fineZ % 8 <= 4)
		{
			points[3][1] = heights[originIndex - fineWidth] / 1000.0f; valid[3] = true;
		}
		else
		{
			if(fineX > 0)
			{
				points[3][1] = heights[originIndex - fineWidth - 1] / 1000.0f; valid[3] = true;
			}
		}
	}

	if(fineX > 0)		
	{
		points[4][1] = heights[originIndex - 1] / 1000.0f; valid[4] = true;
	}

	if(fineZ < fineHeight - 1)
	{
		if(fineZ % 8 < 4)
		{
			if(fineX > 0)
			{
				points[5][1] = heights[originIndex + fineWidth - 1] / 1000.0f; valid[5] = true;
			}
		}
		else
		{
			points[5][1] = heights[originIndex + fineWidth] / 1000.0f; valid[5] = true;
		}
	}

	int numValid = 0;

	for(int i = 0; i < 6; i++)
	{
		uint iA = i;
		uint iB = (i + 1) % 6;

		if(valid[iA] && valid[iB])
		{
			Float3 edgeA = points[iA] - origin;
			Float3 edgeB = points[iB] - origin;
			Float3 crossed = edgeA.Cross(edgeB);
			//crossed.Normalize();

			total += crossed;
			numValid++;
		}	
	}

	assert(numValid >= 1);

	//total /= numValid;
	normal = total;
	normal.Normalize();

	if(fineX == 4 && fineZ == 0)
	{
		Float3 d = normal;
		int zzz = 0;
	}
	//float length = 
}

Terrain::Terrain(ID3DDevice* device, ID3DDeviceContext* context, char* mapFilename) : mTerrainTexture(NULL)
{
	uint numRecognisedPatterns = sizeof(patternInfos) / sizeof(PatternInfo);

	mScaleInfo = new TextureScaleInfo[numRecognisedPatterns];

	for(uint i = 0; i < numRecognisedPatterns; i++)
	{
		mScaleInfo[i].scaleX = patternInfos[i].scaleX;
		mScaleInfo[i].scaleY = patternInfos[i].scaleY;
	}

	D3D_BUFFER_DESC scaleDesc;
	scaleDesc.BindFlags = D3D_BIND_SHADER_RESOURCE;
	scaleDesc.ByteWidth = sizeof(TextureScaleInfo) * numRecognisedPatterns;
	scaleDesc.CPUAccessFlags = 0;
	scaleDesc.MiscFlags = 0;
	//scaleDesc.StructureByteStride = 0;
	scaleDesc.Usage = D3D_USAGE_IMMUTABLE;

	D3D_SUBRESOURCE_DATA scaleData;
	scaleData.pSysMem = mScaleInfo;
	HRESULT bufferResult = device->CreateBuffer(&scaleDesc, &scaleData, &mScaleBuffer); 
	D3D_SHADER_RESOURCE_VIEW_DESC scaleViewDesc;
	scaleViewDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFER;
	scaleViewDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	scaleViewDesc.Buffer.FirstElement = 0;
	scaleViewDesc.Buffer.ElementWidth = numRecognisedPatterns;
	bufferResult = device->CreateShaderResourceView(mScaleBuffer, &scaleViewDesc, &mScaleView);

	context->VSSetShaderResources(0, 1, &mScaleView);

	D3D_TEXTURE2D_DESC desc;
	desc.ArraySize = numRecognisedPatterns;
	desc.BindFlags = D3D_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = DXGI_FORMAT_BC1_UNORM;
	desc.Height = 1024;
	desc.MipLevels = 11;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D_USAGE_DEFAULT;
	desc.Width = 1024;

	HRESULT arrayHR = device->CreateTexture2D(&desc, NULL, &mTerrainTexture);

	D3D_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Format = desc.Format;
	viewDesc.Texture2DArray.ArraySize = desc.ArraySize;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.MipLevels = desc.MipLevels;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	arrayHR = device->CreateShaderResourceView(mTerrainTexture, &viewDesc, &mTerrainView);

	FILE* mapFile = fopen(mapFilename, "rb");
	fread(&mLogicalWidth, 4, 1, mapFile);
	fread(&mLogicalHeight, 4, 1, mapFile);
	fread(&mFineWidth, 4, 1, mapFile);
	fread(&mFineHeight, 4, 1, mapFile);	
	
	mNumHeights = mFineWidth * mFineHeight;
	mHeights = new int[mNumHeights];

	//mDist = new std::tr1::uniform_int<int>(0, mNumHeights);

	fread(mHeights, sizeof(int), mNumHeights, mapFile);
	

	D3D_BUFFER_DESC heightDesc;
	heightDesc.BindFlags = D3D_BIND_SHADER_RESOURCE;
	heightDesc.ByteWidth = sizeof(int) * mNumHeights;
	heightDesc.CPUAccessFlags = 0;
	heightDesc.MiscFlags = 0;
	heightDesc.Usage = D3D_USAGE_DEFAULT;
	//D3D_SUBRESOURCE_DATA heightData;
	//heightData.pSysMem = mHeights;
	HRESULT heightHR = device->CreateBuffer(&heightDesc,NULL, &mHeightBuffer);

	context->UpdateSubresource(mHeightBuffer, 0, NULL, mHeights, sizeof(int) * mNumHeights, 0);

	D3D_SHADER_RESOURCE_VIEW_DESC heightViewDesc;
	heightViewDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFER;
	heightViewDesc.Format = DXGI_FORMAT_R32_SINT;
	heightViewDesc.Buffer.FirstElement = 0;
	heightViewDesc.Buffer.ElementWidth = mNumHeights;

	heightHR = device->CreateShaderResourceView(mHeightBuffer, &heightViewDesc, &mHeightView);
	context->VSSetShaderResources(1, 1, &mHeightView);

	uint numPatterns = mLogicalWidth * mLogicalHeight;
	uint* patterns = new unsigned int[numPatterns];

	fread(patterns, sizeof(uint), numPatterns, mapFile);

	for(int i = 0; i < numPatterns; i++)
	{
		uint idx = GetIndex(patterns[i]);

		if(idx != 0xFF)
		{
			if(patternInfos[idx].loaded == false)
			{
				patternInfos[idx].loaded = true;

				ID3DTexture2D* tex;

				char str[256];
				sprintf_s(str, 256, "Z:/S2C/Development/data/map/textures/%s.dds", patternInfos[idx].texturePath);
				HRESULT result = D3DXCreateTextureFromFile(device, str, NULL, NULL, (ID3DResource**)&tex, NULL);
				assert(result == S_OK);

				D3D_TEXTURE2D_DESC pDesc;
				tex->GetDesc(&pDesc);

				for(int i = 0; i < 11; i++)
				{
					uint destResource = D3DCalcSubresource(i, idx, 11);
					assert(destResource == (i + (idx * 11)));
					uint sourceResource = D3DCalcSubresource(i, 0, 11);
					assert(sourceResource == i);
					context->CopySubresourceRegion(mTerrainTexture, destResource, 0, 0, 0, tex, sourceResource, NULL);
				}

				context->Flush();
				tex->Release();
			}
		}
		else
		{
			printf("%x not found.\n", patterns[i]);
		}
	}

	fclose(mapFile);

	uint numSubVerts = 15;
	uint numSubTriangles = 16;
	uint numBigTriangles = mLogicalWidth * 2 * mLogicalHeight;

	unsigned int numVertices = numBigTriangles * numSubVerts;
	mVertices = new TerrainVertex[numVertices];

	uint idx = 0;

	float weights[45] = 
	{
		1,		0,		0,
		0.75f,	0.25f,	0,
		0.5f,	0.5f,	0,
		0.25f,	0.75f,	0,
		0,		1,		0,
		0.75f,	0,		0.25f,
		0.5f,	0.25f,	0.25f,
		0.25f,	0.5f,	0.25f,
		0,		0.75f,	0.25f,
		0.5f,	0,		0.5f,
		0.25f,	0.25f,	0.5f,
		0,		0.5f,	0.5f,
		0.25f,	0,		0.75f,
		0,		0.25f,	0.75f,
		0,		0,		1
	};

	for(int z = 0; z < mLogicalHeight; z++)
	{
		for(int x = 0; x < mLogicalWidth; x++)
		{
			uint matIdIdx[3];
			matIdIdx[0] = z * mLogicalWidth + x;
			matIdIdx[1] = matIdIdx[0] + 1;
			matIdIdx[2] = matIdIdx[0] + mLogicalWidth;

			if(z % 2 == 0)
			{
				//Pointing up.
				for(int row = 0; row < 5; row++)
				{
					for(int col = 0; col < 5 - row; col++)
					{
						uint xCoord = (x * 4.0f) + col;
						uint zCoord = (z * 4.0f) + row;

						mVertices[idx].position[0] = (xCoord + (row * 0.5f)) * xDiff;
						mVertices[idx].position[1] = (z * mFineWidth * 4.0f) + (row * mFineWidth) + ((x * 4.0f) + col);
						mVertices[idx].position[2] = zCoord * zDiff;

						CalculateNormal(xCoord, zCoord, mHeights, mFineWidth, mFineHeight, mVertices[idx].normal);

						if(idx == 4)
						{
							int zz = 0;
						}

						for(int i = 0; i < 3; i++)
							mVertices[idx].blendWeights[i] = weights[(idx % 15) * 3 + i];

						for(int i = 0; i < 3; i++)
							mVertices[idx].matIDs[i] = GetIndex(patterns[matIdIdx[i]]);

						idx++;
					}
				}

				matIdIdx[0] = (z+1) * mLogicalWidth + x;
				matIdIdx[1] = matIdIdx[0] + 1;
				matIdIdx[2] = matIdIdx[1] - mLogicalWidth;

				//Pointing down.
				for(int row = 0; row < 5; row++)
				{
					for(int col = 0; col < 5 - row; col++)
					{
						uint xCoord = (x * 4.0f) + col;
						uint zCoord = ((z+1) * 4.0f) - row;

						mVertices[idx].position[0] = (xCoord + 2 + (row * 0.5f)) * xDiff;
						mVertices[idx].position[1] = ((z+1) * mFineWidth * 4.0f) - (row * mFineWidth) + ((x * 4.0f) + col) + row;
						mVertices[idx].position[2] = zCoord * zDiff;

						

						CalculateNormal(xCoord + row, zCoord, mHeights, mFineWidth, mFineHeight, mVertices[idx].normal);

						if(idx == 29)
						{
							int zz = 0;
						}

						for(int i = 0; i < 3; i++)
							mVertices[idx].blendWeights[i] = weights[(idx % 15) * 3 + i];

						for(int i = 0; i < 3; i++)
							mVertices[idx].matIDs[i] = GetIndex(patterns[matIdIdx[i]]);

						idx++;
					}
				}	
			}
			else
			{
				uint matIdIdx[3];
				matIdIdx[0] = (z + 1) * mLogicalWidth + x;
				matIdIdx[1] = matIdIdx[0] + 1;
				matIdIdx[2] = matIdIdx[0] - mLogicalWidth;

				//Pointing down.
				for(int row = 0; row < 5; row++)
				{
					for(int col = 0; col < 5 - row; col++)
					{
						uint xCoord = (x * 4.0f) + col;
						uint zCoord = ((z+1) * 4.0f) - row;

						mVertices[idx].position[0] = (xCoord + (row * 0.5f)) * xDiff;
						mVertices[idx].position[1] = ((z+1) * mFineWidth * 4.0f) - (row * mFineWidth) + ((x * 4.0f) + col);
						mVertices[idx].position[2] = zCoord * zDiff;

						CalculateNormal(xCoord, zCoord, mHeights, mFineWidth, mFineHeight, mVertices[idx].normal);

						for(int i = 0; i < 3; i++)
							mVertices[idx].blendWeights[i] = weights[(idx % 15) * 3 + i];

						for(int i = 0; i < 3; i++)
							mVertices[idx].matIDs[i] = GetIndex(patterns[matIdIdx[i]]);

						idx++;
					}
				}

				matIdIdx[0] = z * mLogicalWidth + x;
				matIdIdx[1] = matIdIdx[0] + 1;
				matIdIdx[2] = matIdIdx[1] + mLogicalWidth;

				//Pointing up.
				for(int row = 0; row < 5; row++)
				{
					for(int col = 0; col < 5 - row; col++)
					{
						uint xCoord = (x * 4.0f) + col;
						uint zCoord = (z * 4.0f) + row;

						mVertices[idx].position[0] = (xCoord + 2 + (row * 0.5f)) * xDiff;
						mVertices[idx].position[1] = (z * mFineWidth * 4.0f) + (row * mFineWidth) + ((x * 4.0f) + col) + row;
						mVertices[idx].position[2] = zCoord * zDiff;

						CalculateNormal(xCoord + row, zCoord, mHeights, mFineWidth, mFineHeight, mVertices[idx].normal);

						for(int i = 0; i < 3; i++)
							mVertices[idx].blendWeights[i] = weights[(idx % 15) * 3 + i];

						for(int i = 0; i < 3; i++)
							mVertices[idx].matIDs[i] = GetIndex(patterns[matIdIdx[i]]);

						idx++;
					}
				}
			}
		}
	}

	D3D_BUFFER_DESC vbDesc = CREATE_D3D_BUFFER_DESC(sizeof(TerrainVertex) * numVertices, D3D_USAGE_IMMUTABLE, D3D_BIND_VERTEX_BUFFER, 0, 0, 0);
	D3D_SUBRESOURCE_DATA vbInitData = { mVertices, 0, 0};
	
	HRESULT hr = device->CreateBuffer(&vbDesc, &vbInitData, &mVB);
	
	const int numRegionsWidth = 20;
	const int numRegionsHeight = 20;
	const int regionWidth = (mFineWidth - 1) / numRegionsWidth;
	const int regionHeight = (mFineHeight - 1) / numRegionsHeight;

	const uint R = 0xFFFFFFFF;
	const int numIndicesPerTriangle = 28;
	
	
	uint triIndices0[numIndicesPerTriangle] = 
	{
		0, 5, 1, 6, 2, 7, 3, 8, 4, R,
		5, 9, 6, 10, 7, 11, 8, R,
		9, 12, 10, 13, 11, R,
		12, 14, 13, R
	};

	uint triIndices1[numIndicesPerTriangle] = 
	{
		4, 8, 3, 7, 2, 6, 1, 5, 0, R,
		8, 11, 7, 10, 6, 9, 5, R,
		11, 13, 10, 12, 9, R,
		13, 14, 12, R
	};

	uint indexOffset = 0;

	mNumIndices = (numIndicesPerTriangle * (numBigTriangles));// + ((numIndicesPerTriangle + 1) * (numBigTriangles / 2));
	unsigned int* indices = new unsigned int[mNumIndices];

	idx = 0;

	for(int z = 0; z < mLogicalHeight; z++)
	{
		for(int x = 0; x < mLogicalWidth * 2; x++)
		{
			for(int j = 0; j < numIndicesPerTriangle; j++)
			{
				if(x % 2 == z % 2)
					indices[idx++] = (triIndices0[j] == R) ? R : triIndices0[j] + indexOffset;
				else
					indices[idx++] = (triIndices1[j] == R) ? R : triIndices1[j] + indexOffset;
			}

			indexOffset += numSubVerts;
		}		
	}

	D3D_BUFFER_DESC ibDesc = CREATE_D3D_BUFFER_DESC(sizeof(unsigned int) * mNumIndices, D3D_USAGE_IMMUTABLE, D3D_BIND_INDEX_BUFFER, 0, 0, 0);
	D3D_SUBRESOURCE_DATA ibInitData = { indices, 0, 0 };

	hr = device->CreateBuffer(&ibDesc, &ibInitData, &mIB);

	delete[] mVertices;
	delete[] indices;
	//delete[] patterns;

	D3D_SAMPLER_DESC samplerDesc;
	float borderColor[] = { 0, 0, 0, 0};
	samplerDesc.ComparisonFunc = D3D_COMPARISON_NEVER;
	samplerDesc.MipLODBias = 0;
	samplerDesc.Filter = D3D_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = 1000;
	samplerDesc.MinLOD = 0;
	samplerDesc.AddressU = D3D_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D_TEXTURE_ADDRESS_CLAMP;
	hr = device->CreateSamplerState(&samplerDesc, &mSampler);

	D3D_INPUT_ELEMENT_DESC layout[4] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 36, D3D_INPUT_PER_VERTEX_DATA, 0 },
		
		//{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D_INPUT_PER_VERTEX_DATA, 0 },
		
		//{ "TEXCOORD", 2, DXGI_FORMAT_R32_UINT, 0, 36, D3D_INPUT_PER_VERTEX_DATA, 0 }
		//{ "TEXCOORD", 1, DXGI_FORMAT_R32_SINT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(layout, sizeof(layout) / sizeof(D3D_INPUT_ELEMENT_DESC), g_vsBlob->GetBufferPointer(), g_vsBlob->GetBufferSize(), &mInputLayout);

	//D3D_BUFFER_DESC waterBufferDesc;
	//waterBufferDesc.BindFlags = D3D_BIND_VERTEX_BUFFER;
	//waterBufferDesc.ByteWidth = sizeof(TerrainVertex) * 6;
	//waterBufferDesc.CPUAccessFlags = 0;
	//waterBufferDesc.MiscFlags = 0;
	//waterBufferDesc.Usage = D3D_USAGE_IMMUTABLE;

	//float infinity = 2000;
	//float waterHeight = -1.0f;

	//TerrainVertex waterVertices[6] = 
	//{
	//	{ infinity, waterHeight, -infinity, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0xFFFFFFFF},
	//	{ infinity, waterHeight, infinity, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0xFFFFFFFF},
	//	{ -infinity, waterHeight, infinity, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0xFFFFFFFF},

	//	{ -infinity, waterHeight, infinity, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0xFFFFFFFF},
	//	{ -infinity, waterHeight, -infinity, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0xFFFFFFFF},
	//	{ infinity, waterHeight, -infinity, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0xFFFFFFFF},
	//};


	//D3D_SUBRESOURCE_DATA waterData;
	//waterData.pSysMem = waterVertices;
	//hr = device->CreateBuffer(&waterBufferDesc, &waterData, &mWaterVB);

	//ID3D10Blob* waterFSBlob;

	//hr = LoadShader("F:/Z/S2C/Development/data/shaders/water.fs", "ps_4_0", &waterFSBlob);
	//hr = device->CreatePixelShader(waterFSBlob->GetBufferPointer(), waterFSBlob->GetBufferSize(), &mWaterFS);

	//D3D_BLEND_DESC blendDesc;
	//blendDesc.AlphaToCoverageEnable = false;
	//blendDesc.BlendEnable[0] = true;
	//blendDesc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	//blendDesc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
	//blendDesc.BlendOp = D3D10_BLEND_OP_ADD;
	//blendDesc.SrcBlendAlpha = D3D10_BLEND_ONE;
	//blendDesc.DestBlendAlpha = D3D10_BLEND_ZERO;
	//blendDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	//for(int i = 0; i < 8; i++)
	//	blendDesc.RenderTargetWriteMask[i] = D3D10_COLOR_WRITE_ENABLE_ALL;

	//hr = device->CreateBlendState(&blendDesc, &mWaterBlendState);

}

Terrain::~Terrain(void)
{
}

XMVECTOR GetLineIntersection(XMVECTOR a1, XMVECTOR a2, XMVECTOR b1, XMVECTOR& b2, bool& intersects)
{
	float x1 = XMVectorGetX(a1); float y1 = XMVectorGetZ(a1);
	float x2 = XMVectorGetX(a2); float y2 = XMVectorGetZ(a2);
	float x3 = XMVectorGetX(b1); float y3 = XMVectorGetZ(b1);
	float x4 = XMVectorGetX(b2); float y4 = XMVectorGetZ(b2);

	float denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);

	if(denom == 0)
	{
		intersects = false;
		return XMVectorZero();
	}
	else
	{
		float aNumerator = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
		float bNumerator = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3);

		float ua = aNumerator / denom;
		float ub = bNumerator / denom;

		float x = x1 + ua * (x2 - x1);
		float y = y1 + ua * (y2 - y1);

		if(ua > 0 && ub > 0 && ua < 1 && ub < 1)
		{
			intersects = true;
			return XMVectorSet(x, 0, y, 0);
		}
		else
		{
			intersects = false;
			return XMVectorZero();
		}
	}
}

uint Terrain::GetPickedVertex(XMVECTOR cameraPos, XMVECTOR ray)
{
	float worldEndX = mFineWidth * xDiff;
	float worldEndZ = mFineHeight * zDiff;

	bool startedInWorld = true;

	XMVECTOR currentRayPos = cameraPos;

	XMVECTOR entryPoint;	float distanceToEntryPoint = FLT_MAX;
	XMVECTOR exitPoint;		float distanceToExitPoint = -FLT_MAX;

	if(XMVectorGetX(cameraPos) < 0 || XMVectorGetX(cameraPos) > worldEndX || XMVectorGetZ(cameraPos) < 0 || XMVectorGetZ(cameraPos) > worldEndZ)
	{
		startedInWorld = false;
	}
	else
	{
		entryPoint = cameraPos;
		distanceToEntryPoint = 0;
	}

	float stepsToZeroX = -XMVectorGetX(cameraPos) / XMVectorGetX(ray);
	float stepsToZeroZ = -XMVectorGetZ(cameraPos) / XMVectorGetZ(ray);
	float stepsToWorldEndX = (worldEndX - XMVectorGetX(cameraPos)) / XMVectorGetX(ray);
	float stepsToWorldEndZ = (worldEndZ - XMVectorGetZ(cameraPos)) / XMVectorGetZ(ray);

	bool rayEntersWorld = false;

	if(stepsToZeroX > 0)
	{
		XMVECTOR zeroXIP = XMVectorAdd(cameraPos, XMVectorScale(ray, stepsToZeroX));
		
		if(XMVectorGetZ(zeroXIP) > 0 && XMVectorGetZ(zeroXIP) < worldEndZ)
		{
			if(!startedInWorld && stepsToZeroX < distanceToEntryPoint)
			{
				distanceToEntryPoint = stepsToZeroX;
				entryPoint = zeroXIP;
			}				

			if(stepsToZeroZ > distanceToExitPoint)
			{
				distanceToExitPoint = stepsToZeroX;
				exitPoint = zeroXIP;
			}				
		}
	}

	if(stepsToZeroZ > 0)
	{
		XMVECTOR zeroZIP = XMVectorAdd(cameraPos, XMVectorScale(ray, stepsToZeroZ));

		if(XMVectorGetX(zeroZIP) > 0 && XMVectorGetX(zeroZIP) < worldEndX)
		{
			if(!startedInWorld && stepsToZeroZ < distanceToEntryPoint)
			{
				distanceToEntryPoint = stepsToZeroZ;
				entryPoint = zeroZIP;
			}

			if(stepsToZeroZ > distanceToExitPoint)
			{
				distanceToExitPoint = stepsToZeroZ;
				exitPoint = zeroZIP;
			}
		}
	}

	if(stepsToWorldEndX > 0)
	{
		XMVECTOR weXIP = XMVectorAdd(cameraPos, XMVectorScale(ray, stepsToWorldEndX));

		if(XMVectorGetZ(weXIP) > 0 && XMVectorGetZ(weXIP) < worldEndZ)
		{
			if(!startedInWorld && stepsToWorldEndX < distanceToEntryPoint)
			{
				distanceToEntryPoint = stepsToWorldEndX;
				entryPoint  = weXIP;
			}

			if(stepsToWorldEndX > distanceToExitPoint)
			{
				distanceToExitPoint = stepsToWorldEndX;
				exitPoint = weXIP;
			}
		}
	}

	if(stepsToWorldEndZ > 0)
	{
		XMVECTOR weZIP = XMVectorAdd(cameraPos, XMVectorScale(ray, stepsToWorldEndZ));
		
		if(XMVectorGetX(weZIP) > 0 && XMVectorGetZ(weZIP) < worldEndX)
		{
			if(!startedInWorld && stepsToWorldEndZ < distanceToEntryPoint)
			{
				distanceToEntryPoint = stepsToWorldEndZ;
				entryPoint = weZIP;
			}
			if(stepsToWorldEndZ > distanceToExitPoint)
			{
				distanceToExitPoint = stepsToWorldEndZ;
				exitPoint = weZIP;
			}
		}
	}

	const XMVECTOR leftTranslateUnit = XMVector3Normalize(XMVectorSet(-xDiff, 0, -zDiff, 0));
	const XMVECTOR rightTranslateUnit = XMVector3Normalize(XMVectorSet(xDiff, 0, -zDiff, 0));

	

	const float fhmo = mFineHeight - 1;

	const XMVECTOR leftLeaningLine = XMVectorSet(-xDiff / 2 * fhmo, 0, fhmo * zDiff, 0);
	const XMVECTOR rightLeaningLine = XMVectorSet(xDiff / 2 * fhmo, 0, fhmo * zDiff, 0);

	const float rlATan2 = atan2f(zDiff, xDiff);
	const float llATan2 = atan2f(zDiff, -xDiff);

	XMVECTOR entryTranslatedLeftToZeroX = XMVectorSetY(XMVectorAdd(entryPoint, XMVectorScale(leftTranslateUnit, XMVectorGetZ(entryPoint) / -XMVectorGetZ(leftTranslateUnit))), 0);
	XMVECTOR entryTranslatedRightToZeroX = XMVectorSetY(XMVectorAdd(entryPoint, XMVectorScale(rightTranslateUnit, XMVectorGetZ(entryPoint) / -XMVectorGetZ(rightTranslateUnit))), 0);

	float rayATan2 = atan2f(XMVectorGetZ(ray), XMVectorGetX(ray));
	bool moveRight = !(rayATan2 > rlATan2 || rayATan2 < -llATan2);

	if(moveRight)
		entryTranslatedLeftToZeroX = XMVectorSetX(entryTranslatedLeftToZeroX, XMVectorGetX(entryTranslatedLeftToZeroX) + (xDiff - fmodf(XMVectorGetX(entryTranslatedLeftToZeroX), xDiff)));
	else
		entryTranslatedLeftToZeroX = XMVectorSetX(entryTranslatedLeftToZeroX, XMVectorGetX(entryTranslatedLeftToZeroX) - fmodf(XMVectorGetX(entryTranslatedLeftToZeroX), xDiff));

	XMVECTOR modifier = XMVectorSet((moveRight) ? xDiff : -xDiff, 0, 0, 0);

	XMVECTOR rightLeanExit = XMVectorAdd(entryTranslatedLeftToZeroX, rightLeaningLine);

	int idx = 0;

	char str[256];

	while(true)
	{
		
		

		bool intersection = false;
		XMVECTOR ip = GetLineIntersection(entryTranslatedLeftToZeroX, rightLeanExit, entryPoint, exitPoint, intersection);

		//printf("%d\n", idx++);
		sprintf_s(str, 256, "(%f,%f)\n", XMVectorGetX(ip), XMVectorGetZ(ip));
		OutputDebugString(str);

		entryTranslatedLeftToZeroX = XMVectorAdd(entryTranslatedLeftToZeroX, modifier);
		rightLeanExit = XMVectorAdd(rightLeanExit, modifier);

		if(!intersection)
			break;
	}

	//int nextXLine

	//int nextPosXLine = 
	

	//Work out where we hit the map.
	
	//float stepsTo



	return 0;
}

void Terrain::Update(ID3DDeviceContext& context, XMVECTOR& cameraPosition, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj)
{
	POINT ptCursor;
	GetCursorPos( &ptCursor );
	ScreenToClient( g_hWnd, &ptCursor );

	XMVECTOR nearVector = XMVector3Unproject(XMVectorSet(ptCursor.x, ptCursor.y, 0, 0), 0, 0, 1600, 1000, 0, 1, proj, view, world);
	XMVECTOR farVector = XMVector3Unproject(XMVectorSet(ptCursor.x, ptCursor.y, 1, 0), 0, 0, 1600, 1000, 0, 1, proj, view, world);

//	uint vertex = GetPickedVertex(cameraPosition, XMVector3Normalize(XMVectorSubtract(farVector, nearVector)));

	/*ID3DShaderResourceView* n = NULL;
	context.VSSetShaderResources(1, 1, &n);

	int numEdits = 100;

	std::tr1::uniform_int<int>& dist = *mDist;

	for(int i = 0; i < numEdits; i++)
	{
		mHeights[dist(mGen)] += (dist(mGen) % 5000) - 2500;
	}*/

	//context.VSSetShaderResources(1, 1, &mHeightView);
}

void Terrain::Draw(ID3DDeviceContext& context, XMMATRIX& matrix)
{
	UINT offset = 0;
	UINT stride = sizeof(TerrainVertex);

	const float blendFactor[4] = { 1, 1, 1, 1};

	//context.OMSetBlendState(NULL, blendFactor, 0xFFFFFFFF);
	context.IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	context.IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
	context.PSSetShaderResources(0, 1, &mTerrainView);
	context.PSSetSamplers(0, 1, &mSampler);
	context.IASetInputLayout(mInputLayout);
	context.DrawIndexed(mNumIndices, 0, 0);
	
	
	/*context.OMSetBlendState(mWaterBlendState, blendFactor, 0xFFFFFFFF);
	context.IASetVertexBuffers(0, 1, &mWaterVB, &stride, &offset);
	context.PSSetShader(mWaterFS);
	context.Draw(6, 0);*/
}
