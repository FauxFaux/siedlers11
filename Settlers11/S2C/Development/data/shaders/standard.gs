cbuffer camera
{
	float4x4 wvp;
}

struct GS_IN
{
	float3 position : POSITION;
	//float height : TEXCOORD0;
	//float2 texCoord : TEXCOORD;
	//uint3 matIDs : TEXCOORD1;
	//float3 blendWeights : TEXCOORD3;
};

struct GS_OUT
{
	float4 position : SV_POSITION;
	float height : TEXCOORD0;
	//float2 texCoord : TEXCOORD;
	//uint3 matIDs : TEXCOORD1;
	//float3 blendWeights : TEXCOORD3;
};

[maxvertexcount(3)]
void main(triangle GS_IN input[3], inout TriangleStream<GS_OUT> OutputStream)
{
/*
	input[0].position = mul(wvp, float4(input[0].position.x, input[0].height, input[0].position.y, 1));
	input[1].position = mul(wvp, float4(input[1].position.x, input[1].height, input[1].position.y, 1));
	input[2].position = mul(wvp, float4(input[2].position.x, input[2].height, input[2].position.y, 1));

	OutputStream.Append(input[0]);
	OutputStream.Append(input[1]);
	OutputStream.Append(input[2]);
	
	GS_OUT outputVert = (GS_OUT)0;
	
	for(int i = 0; i < 3; i++)
	{
		outputVert.height = input[i].position.y;
		outputVert.position = mul(wvp, float4(input[i].position, 1));
		OutputStream.Append(outputVert);
	}
	
	OutputStream.RestartStrip();*/
	
	float4 pos[3];
	pos[0] = mul(wvp, float4(input[0].position, 1));
	pos[1] = mul(wvp, float4(input[1].position, 1));
	pos[2] = mul(wvp, float4(input[2].position, 1));
	
	float4 t0 = saturate(pos[0].xyxy * float4(-1, -1, 1, 1) - pos[0].w);
	float4 t1 = saturate(pos[1].xyxy * float4(-1, -1, 1, 1) - pos[1].w);
	float4 t2 = saturate(pos[2].xyxy * float4(-1, -1, 1, 1) - pos[2].w);
	float4 t = t0 * t1 * t2;
	
	[branch]
	if(!any(t))
	{
		//float2 d0 = pos[1].xy * pos[0].w - pos[0].xy * pos[1].w;
		//float2 d1 = pos[2].xy * pos[0].w - pos[0].xy * pos[2].w;
		
		//[branch]
		//if(d1.x * d0.y > d0.x * d1.y || min(min(pos[0].w, pos[1].w), pos[2].w) < 0.0)
		{	
			GS_OUT vert = (GS_OUT)0;
			
			[unroll]for(int i = 0; i < 3; i++)
			{
				vert.position = pos[i];
				vert.height = input[i].position.y;
				OutputStream.Append(vert);
			}
			
			OutputStream.RestartStrip();
		}
	}
}