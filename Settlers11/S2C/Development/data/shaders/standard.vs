cbuffer camera
{
	float4x4 wvp;
	float3 lightPosition;
}

Buffer<float2> scaleTextureInfo;
Buffer<int> heightBuffer;

struct VS_IN
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 blendWeights : TEXCOORD0;
	uint3 matIDs : TEXCOORD1;
	
	
	
	//uint heightIndex : TEXCOORD1;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	uint3 matIDs : TEXCOORD0;
	
	//float height : TEXCOORD0;
	float3 worldPosition : TEXCOORD1;
	
	
	float3 blendWeights : TEXCOORD2;
	float3x2 texScale : TEXCOORD3;
	
};

void main(in VS_IN In, out VS_OUT Out)
{	
	//Out.height = 0;//heightBuffer.Load(In.heightIndex) * 0.001f;
	Out.texScale[0] = scaleTextureInfo.Load(In.matIDs.x);
	Out.texScale[1] = scaleTextureInfo.Load(In.matIDs.y);
	Out.texScale[2] = scaleTextureInfo.Load(In.matIDs.z);
	//float height = heightBuffer.Load(In.heightIndex) * 0.001f;
	
	In.position.y = heightBuffer.Load((uint)In.position.y) * 0.001f;
	
	Out.position = mul(wvp, float4(In.position.xyz, 1));
	Out.matIDs = In.matIDs;
	Out.worldPosition = In.position;
	Out.normal = In.normal;
	
	//Out.texCoord = In.position.xz;
	//Out.matIDs = In.matIDs;
	Out.blendWeights = In.blendWeights;
	
	//Out.position = float3(In.position.x, heightBuffer.Load(In.heightIndex) * 0.001f, In.position.y);
	//Out.matIDs = In.matIDs;
}