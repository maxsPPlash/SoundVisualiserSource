cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;
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

float tent_move(float p, float t) {
	return sin((p+t)) * sin((p+t*0.7)*5)/15;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	const float pi = 3.14159274101;
	const uint tenatacle_count = 16;
//	const float tent_pos [tenatacle_count] = {
//		0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9,
//	};

	float tent_pos[tenatacle_count];
	for (int i = 0; i < tenatacle_count; ++i) {
		tent_pos[i] = float(i)/tenatacle_count;
	}

	float2 uv = input.inPosition.xy / float2(wnd_h, wnd_h);
//	int tx = int(uv.x*512.0);

	float fft;
	for (int i = 0; i < 32; ++i) {
		fft += objTexture.Sample(objSamplerState, float2(1.0/512.0 * i, 0)).x;
	}
	fft /= 32;
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

	// wrap to circle
	float2 new_uv;
	new_uv.y = pow(pow(uv.x - ratio*0.5, 2) + pow(uv.y - 0.5, 2), 0.5) * 1.5;
	new_uv.x = (pi + atan2(uv.y - 0.5, uv.x - ratio*0.5)) / (pi*2);
	uv = new_uv;

//	if (uv.x*20 - int(uv.x*20) < 0.1 || uv.y*20 - int(uv.y*20) < 0.1)
//		return uv.x;
//	return 0.9;


//	float x_cent_coef = fmod(uv.x, 0.5);

//	float bg_whitness_coef = x_cent_coef + smoothstep(0.3, 0.8, x_cent_coef*uv.y+0.1);

	// background
	float3 col = lerp(0.8, 0.05, smoothstep(0.3, 0.8, uv.y-0.2));

//	float funcst = tent_move(1, time/2);

	for (int t = 0; t < 2; ++t) {
		// Tentacles
		for (int i = 0; i < tenatacle_count; ++i) {
			float func = 2*((1-uv.y)+0.0)*tent_move(uv.y, time/2)+tent_pos[i]/* - funcst*/;
			float coef = pow(uv.y-0.3, 1)+fft-0.8;
			if (coef < 0.06) coef = 0;
			float d = lerp(0, 0.02, coef);
			float dist = abs(uv.x-func);
		//	float d = 0.01;
		//	float dist;
		//	float end = fft/2.0;
		//	if (uv.y < end) {
		//		float funcin = 2*((1-end)+0.0)*tent_move(end, time/2)+0.5;
		//		dist = distance(uv, float2(funcin, end));
		//	} else {
		//		dist = abs(uv.x-func);
		//	}

			if (dist < d)
				col = lerp(col, 0.05, (d - dist)*300);// float3(0.0, 0.0, 0.0);
		}
		if (uv.x < 0.1)
			uv.x += 1;
		else if (uv.x > 0.9)
			uv.x -= 1;
		else break;
	}

	// Output to screen
	return float4(col,1.0);
}