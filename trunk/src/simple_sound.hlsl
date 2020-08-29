cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;

	float bass_coef;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 uv = input.inPosition.xy / float2(wnd_w, wnd_h);
	float tex = objTexture.Sample(objSamplerState, uv.x).x;// 0.0;

	float3 col = 0;
	if (1-uv.y > tex) {
		col = 1;
	}

	// Output to screen
	return float4(col,1.0);
}