cbuffer camera
{
	float4x4 wvp;
}

//Buffer<float2> scaleTextureInfo;
Buffer<int> heightBuffer;

struct VS_IN
{
	float2 position : POSITION;
	//uint3 matIDs : TEXCOORD;
	//float3 blendWeights : TEXCOORD1;
	//float3 normal : NORMAL;
	uint heightIndex : TEXCOORD0;
};

struct VS_OUT
{
	float3 position : POSITION;
	//float height : TEXCOORD0;
	//float2 texCoord : TEXCOORD;
	//uint3 matIDs : TEXCOORD1;
	//float3 blendWeights : TEXCOORD3;
	//float3x2 texScale : TEXCOORD4;
};

void main(in VS_IN In, out VS_OUT Out)
{	
	//Out.height = 0;//heightBuffer.Load(In.heightIndex) * 0.001f;
	//Out.texScale[0] = scaleTextureInfo.Load(In.matIDs.x);
	//Out.texScale[1] = scaleTextureInfo.Load(In.matIDs.y);
	//Out.texScale[2] = scaleTextureInfo.Load(In.matIDs.z);
	//Out.height = heightBuffer.Load(In.heightIndex) * 0.001f;
	//Out.position = mul(wvp, float4(In.position.x, Out.height, In.position.y, 1));
	
	//Out.texCoord = In.position.xz;
	//Out.matIDs = In.matIDs;
	//Out.blendWeights = In.blendWeights;
	
	Out.position = float3(In.position.x, heightBuffer.Load(In.heightIndex) * 0.001f, In.position.y);
}