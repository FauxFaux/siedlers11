Texture2DArray tex;
SamplerState TexSampler;

cbuffer camera
{
	float4x4 wvp;
	float3 lightPosition;
}

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

float4 main(in VS_OUT In) : SV_TARGET
{		
//	In.normal = normalize(In.normal);
	//return length(In.normal);
	//return abs(In.normal.x);
	float4 colour = 0;
	
	[unroll]
	for(int i = 0; i < 3; i++)
		colour += tex.Sample(TexSampler, float3(In.worldPosition.xz * In.texScale[i], In.matIDs[i])) * In.blendWeights[i];
		
	float3 toLight = normalize(lightPosition - In.worldPosition);
	float dp = dot(toLight, normalize(In.normal));
		
	return colour * dp;
}