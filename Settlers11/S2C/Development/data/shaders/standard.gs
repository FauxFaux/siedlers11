cbuffer camera
{
	float4x4 wvp;
}

struct GS_IN
{
	uint vertexID : TEXCOORD0;
	//float3 position : POSITION;
	//float height : TEXCOORD0;
	//float2 texCoord : TEXCOORD;
	//uint3 matIDs : TEXCOORD1;
	//float3 blendWeights : TEXCOORD3;
};

struct GS_OUT
{
	float4 position : SV_POSITION;
	//float height : TEXCOORD0;
	//float2 texCoord : TEXCOORD;
	//uint3 matIDs : TEXCOORD1;
	//float3 blendWeights : TEXCOORD3;
};

[maxvertexcount(24)]
void main(point GS_IN input[1], inout TriangleStream<GS_OUT> OutputStream)
{
	

	const float xDiff = 1.5f;
	const float zDiff = 1.299f;
	
	float xOffset = (input[0].vertexID % 860) * (xDiff * 4);
	float zOffset = (input[0].vertexID / 860) * (zDiff * 4);
	float vertexID = input[0].vertexID * xDiff * 4;

	float4 pos[15];
	
	pos[0] = mul(wvp, float4(xOffset + (xDiff * 0.0f), 0, zOffset + (zDiff * 0.0f), 1));
	pos[1] = mul(wvp, float4(xOffset + (xDiff * 1.0f), 0, zOffset + (zDiff * 0.0f), 1));
	pos[2] = mul(wvp, float4(xOffset + (xDiff * 2.0f), 0, zOffset + (zDiff * 0.0f), 1));
	pos[3] = mul(wvp, float4(xOffset + (xDiff * 3.0f), 0, zOffset + (zDiff * 0.0f), 1));
	pos[4] = mul(wvp, float4(xOffset + (xDiff * 4.0f), 0, zOffset + (zDiff * 0.0f), 1));
	
	pos[5] = mul(wvp, float4(xOffset + (xDiff * 0.5f), 0, zOffset + (zDiff * 1.0f), 1));
	pos[6] = mul(wvp, float4(xOffset + (xDiff * 1.5f), 0, zOffset + (zDiff * 1.0f), 1));
	pos[7] = mul(wvp, float4(xOffset + (xDiff * 2.5f), 0, zOffset + (zDiff * 1.0f), 1));
	pos[8] = mul(wvp, float4(xOffset + (xDiff * 3.5f), 0, zOffset + (zDiff * 1.0f), 1));
	
	pos[9] =  mul(wvp, float4(xOffset + (xDiff * 1.0f), 0, zOffset + (zDiff * 2.0f), 1));
	pos[10] = mul(wvp, float4(xOffset + (xDiff * 2.0f), 0, zOffset + (zDiff * 2.0f), 1));
	pos[11] = mul(wvp, float4(xOffset + (xDiff * 3.0f), 0, zOffset + (zDiff * 2.0f), 1));
	
	pos[12] = mul(wvp, float4(xOffset + (xDiff * 1.5f), 0, zOffset + (zDiff * 3.0f), 1));
	pos[13] = mul(wvp, float4(xOffset + (xDiff * 2.5f), 0, zOffset + (zDiff * 3.0f), 1));
	
	pos[14] = mul(wvp, float4(xOffset + (xDiff * 2.0f), 0, zOffset + (zDiff * 4.0f), 1));
	
	GS_OUT vert = (GS_OUT)0;
	
	vert.position = pos[0];
	OutputStream.Append(vert);
	vert.position = pos[5];
	OutputStream.Append(vert);
	vert.position = pos[1];
	OutputStream.Append(vert);
	vert.position = pos[6];
	OutputStream.Append(vert);
	vert.position = pos[2];
	OutputStream.Append(vert);
	vert.position = pos[7];
	OutputStream.Append(vert);
	vert.position = pos[3];
	OutputStream.Append(vert);
	vert.position = pos[8];
	OutputStream.Append(vert);
	vert.position = pos[4];
	OutputStream.Append(vert);
	
	OutputStream.RestartStrip();
	
	vert.position = pos[5];
	OutputStream.Append(vert);
	vert.position = pos[9];
	OutputStream.Append(vert);
	vert.position = pos[6];
	OutputStream.Append(vert);
	vert.position = pos[10];
	OutputStream.Append(vert);
	vert.position = pos[7];
	OutputStream.Append(vert);
	vert.position = pos[11];
	OutputStream.Append(vert);
	vert.position = pos[8];
	OutputStream.Append(vert);
	
	OutputStream.RestartStrip();
	
	vert.position = pos[9];
	OutputStream.Append(vert);
	vert.position = pos[12];
	OutputStream.Append(vert);
	vert.position = pos[10];
	OutputStream.Append(vert);
	vert.position = pos[13];
	OutputStream.Append(vert);
	vert.position = pos[11];
	OutputStream.Append(vert);
	
	OutputStream.RestartStrip();
	
	vert.position = pos[12];
	OutputStream.Append(vert);
	vert.position = pos[14];
	OutputStream.Append(vert);	
	vert.position = pos[13];
	OutputStream.Append(vert);
}