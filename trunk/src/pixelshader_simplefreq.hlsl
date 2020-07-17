cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;
	float local_tent_len_;
	float cam_pos;
	float bass_coef;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

Texture2D objTexture1 : TEXTURE : register(t1);

//	return sin((p+t));// * sin((p+t*0.7)*5)/20;

float tent_move(float p, float t, float in_coef1, float in_coef2, float in_coef3, float out_coef) {
	float x = (p-t);

	return sin(x*in_coef1) * sin(x*in_coef2) * sin(x*in_coef3) * out_coef;
}

float soft_rand(float n)
{
    return frac(sin(n)*43758.5453);
}
float rand(float n)
{
    return frac(sin(n)*43758.5453);
}
float rand2d(float2 v)
{
	float2 K1 = float2(23.14069263277926,2.665144142690225);
	return frac( cos( dot(v,K1) ) * 12345.6789 );

//    return frac(sin(dot(v, float2(12.9898,78.233))*43758.5453));
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 uv = input.inPosition.xy / float2(wnd_w, wnd_h);

	float3 col = 1.0;

	if (1 - objTexture.Sample(objSamplerState, float2(uv.x/4, 0)).x > uv.y)
		col = 0.0;

	// Output to screen
	return float4(col,1.0);
}