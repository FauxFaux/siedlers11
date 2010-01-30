Texture2DArray tex;
SamplerState TexSampler;

struct VS_OUT
{
	float4 position : SV_POSITION;
	float height : TEXCOORD0;
	//float2 texCoord : TEXCOORD;
	//uint3 matIDs : TEXCOORD1;
	//float3 blendWeights : TEXCOORD3;
	//float3x2 texScale : TEXCOORD4;
};

float4 main(in VS_OUT In) : SV_Target
{		

[branch]
	if(In.height < -1)
	return float4(0, 0, 1, 1);
	else
	{
		float maxHeight = 15;
		
		float whiteFraction = In.height / maxHeight;
		return float4(1,1,1,1) * whiteFraction + float4(0,1,0,1) * (1-whiteFraction);
		//return float4(0, 1, 0, 1);
	}
	
/*
	float4 colour = 0;
	
	if(In.blendWeights[0] > 0)
		colour += tex.Sample(TexSampler, float3(In.texCoord * float2(0.08, 0.08), In.matIDs[0])) * In.blendWeights[0];
		
	if(In.blendWeights[1] > 0)
		colour += tex.Sample(TexSampler, float3(In.texCoord * float2(0.08, 0.08), In.matIDs[1])) * In.blendWeights[1];
		
	if(In.blendWeights[2] > 0)
		colour += tex.Sample(TexSampler, float3(In.texCoord * float2(0.08, 0.08), In.matIDs[2])) * In.blendWeights[2];
		
	return colour;*/
}