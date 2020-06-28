cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;
	float tent_len;
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
	const float pi = 3.14159274101;
	const uint tenatacle_count = 10;
	const float tent_pos[tenatacle_count] = {
		0.05, 0.2, 0.25, 0.4, 0.5, 0.68, 0.74, 0.81, 0.87, 0.93
	};
	const float4 tent_params[tenatacle_count] = {
		float4(1.5, 0.3, 2.8, 1.0/30.0),
		float4(0.5, 0.3, 2.8, 1.0/35.0),
		float4(1.5, 1.3, 2.8, 1.0/30.0),
		float4(4.5, 0.3, 2.8, 1.0/40.0),
		float4(1.5, 0.3, 0.8, 1.0/25.0),
		float4(2.5, 0.3, 1.0, 1.0/30.0),
		float4(1.5, 0.3, 2.8, 1.0/30.0),
		float4(1.9, 0.2, 3.0, 1.0/30.0),
		float4(2.5, 1.3, 4.8, 1.0/40.0),
		float4(3.5, 1.3, 1.8, 1.0/30.0),
	};

	float2 uv = input.inPosition.xy / float2(wnd_h, wnd_h);
//	int tx = int(uv.x*512.0);

//	float fft;
//	for (int i = 0; i < 32; ++i) {
//		fft += objTexture.Sample(objSamplerState, float2(1.0/512.0 * i, 0)).x;
//	}
//	fft /= 32;
//	float3 ccol = float3( fft, 4.0*fft*(1.0-fft), 1.0-fft ) * fft;
//	float3 ccol = 1.0;
//	if (uv.y > 1 - fft)
//		ccol = float3(1.0, 0, 0);
//	float samp = objTexture.Sample(objSamplerState, float2(uv.x, 0)).x;
//
//	return float4(ccol, 0);

//	float3 samp = objTexture1.Sample(objSamplerState, uv);
//	return float4(samp,1.0);

	// Normalized pixel coordinates (from 0 to 1)
	float ratio = float(wnd_w) / float(wnd_h);

	// background
	float dist = pow(pow(uv.x - ratio*0.5, 2) + pow(uv.y - 0.5, 2), 0.5)*1.7 * (0.3*smoothstep(0.0, 0.5, abs(uv.y - 0.5))+0.7);
	float3 col = lerp(0.8, 0.05, smoothstep(0.3, 0.8, dist-0.2));

	// wrap to circle
	float2 crc_uv;
	crc_uv.y = pow(pow(uv.x - ratio*0.5, 2) + pow(uv.y - 0.5, 2), 0.5) * 1.5;
	crc_uv.x = (pi + atan2(uv.y - 0.5, uv.x - ratio*0.5)) / (pi*2);
//	uv = new_uv;

//	if (uv.x*20 - int(uv.x*20) < 0.1 || uv.y*20 - int(uv.y*20) < 0.1)
//		return uv.x;
//	return 0.9;


	float local_tent_len = tent_len - cam_pos;

	for (int t = 0; t < 2; ++t) {
		// Tentacles
		for (int i = 0; i < tenatacle_count; ++i) {
			float4 tp = tent_params[i];
			float func = 2*tent_move(crc_uv.y, tent_len/2, tp.x, tp.y, tp.z, tp.w)+tent_pos[i];
			float coef = pow(crc_uv.y-0.3, 1)+local_tent_len-0.8;
			if (coef < 0.06) coef = 0;
			float d = lerp(0, 0.02, coef);
			float dist = abs(crc_uv.x-func);

			if (dist < d)
				col = lerp(col, float3(0.05, 0.05, 0.05), clamp((d - dist)*300, 0.0, 1.0));// float3(0.0, 0.0, 0.0);
		}
		if (crc_uv.x < 0.1)
			crc_uv.x += 1;
		else if (crc_uv.x > 0.9)
			crc_uv.x -= 1;
		else break;
	}

	// centre
	const float fade = 0.01;
	const float center_rad = 0.15;
	float shake_coef = smoothstep(0.4, 0.9, bass_coef);
	float shakex = (rand(time+100.0)-0.5) / 50.0 * shake_coef;
	float shakey = (rand(time+200.0)-0.5) / 50.0 * shake_coef;
	dist = pow(pow(uv.x - ratio*(0.5+shakex), 2) + pow(uv.y - (0.5+shakey), 2), 0.5);
	if (dist < center_rad) {
		float2 tex = (uv - float2(ratio*(0.5+shakex), (0.5+shakey)) + center_rad) / (center_rad*2);
		if (center_rad - dist > fade)
			col = objTexture1.Sample(objSamplerState, tex);// 0.0;
		else
			col = lerp(col, objTexture1.Sample(objSamplerState, tex), (center_rad - dist) / fade);
	}

	float3 nz = (rand2d(uv*time)) * 0.1;
	col += nz;

	// Output to screen
	return float4(col,1.0);
}