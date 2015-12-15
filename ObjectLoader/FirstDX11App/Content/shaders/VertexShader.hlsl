cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	return pos;
}