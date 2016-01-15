// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// A constant buffer storing animation bone data
cbuffer AnimationConstantBuffer : register(b1)
{
	matrix meshMatrix;
	matrix boneMatrices[50];
}

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	uint4 boneIndices : BONEINDICES;
	float4 weights : WEIGHTS;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Test animations
	matrix boneTransform = 
		boneMatrices[input.boneIndices.x] * input.weights.x +
		boneMatrices[input.boneIndices.y] * input.weights.y +
		boneMatrices[input.boneIndices.z] * input.weights.z +
		boneMatrices[input.boneIndices.w] * input.weights.w;

	pos = mul(pos, meshMatrix);
	pos = mul(pos, boneTransform);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the texCoods through without modification.
	output.uv = input.uv;

	return output;
}
