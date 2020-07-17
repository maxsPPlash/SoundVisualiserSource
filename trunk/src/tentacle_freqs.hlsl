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
	const float pi = 3.14159274101;
	const uint tenatacle_count = 28;
	float tent_pos[tenatacle_count] = {
		0.05, 0.11, 0.2, 0.25, 0.33, 0.4, 0.46, 0.5, 0.59, 0.68, 0.74, 0.81, 0.87, 0.96,
		1.05, 1.11, 1.2, 1.25, 1.33, 1.4, 1.46, 1.5, 1.59, 1.68, 1.74, 1.81, 1.87, 1.96
	};
	for (int i = 0; i < tenatacle_count; ++i) {
		tent_pos[i] /= 2.f;
	}
	const float4 tent_params[tenatacle_count] = {
		float4(2.06, 4.15, 1.71, 1.0/40.0),
		float4(3.75, 2.41, 1.51, 1.0/35.0),
		float4(2.96, 5.42, 1.47, 1.0/40.0),
		float4(5.47, 3.09, 2.80, 1.0/40.0),
		float4(5.57, 3.46, 0.80, 1.0/25.0),
		float4(2.80, 5.70, 3.67, 1.0/30.0),
		float4(3.55, 2.09, 1.04, 1.0/30.0),
		float4(3.28, 0.90, 1.81, 1.0/30.0),
		float4(3.74, 2.09, 4.80, 1.0/40.0),
		float4(2.51, 1.25, 3.50, 1.0/30.0),
		float4(4.62, 5.10, 1.80, 1.0/30.0),
		float4(1.67, 2.74, 4.66, 1.0/30.0),
		float4(3.83, 1.30, 2.06, 1.0/30.0),
		float4(4.92, 1.31, 5.37, 1.0/50.0),

		float4(2.06, 4.15, 1.71, 1.0/40.0),
		float4(3.75, 2.41, 1.51, 1.0/35.0),
		float4(2.96, 5.42, 1.47, 1.0/40.0),
		float4(5.47, 3.09, 2.80, 1.0/40.0),
		float4(5.57, 3.46, 0.80, 1.0/25.0),
		float4(2.80, 5.70, 3.67, 1.0/30.0),
		float4(3.55, 2.09, 1.04, 1.0/30.0),
		float4(3.28, 0.90, 1.81, 1.0/30.0),
		float4(3.74, 2.09, 4.80, 1.0/40.0),
		float4(2.51, 1.25, 3.50, 1.0/30.0),
		float4(4.62, 5.10, 1.80, 1.0/30.0),
		float4(1.67, 2.74, 4.66, 1.0/30.0),
		float4(3.83, 1.30, 2.06, 1.0/30.0),
		float4(4.92, 1.31, 5.37, 1.0/50.0),
	};

	float2 uv = input.inPosition.xy / float2(wnd_h, wnd_h);

	// Normalized pixel coordinates (from 0 to 1)
	float ratio = float(wnd_w) / float(wnd_h);

	float intro_time = 3.0;

	float intro_coef = time > intro_time ? 0.0 : 1 - (time / intro_time);

	// wrap to circle
	float2 crc_uv;
	crc_uv.y = pow(pow(uv.x - ratio * 0.5, 2) + pow(uv.y - 0.5, 2), 0.5) * 1.5;
	float ang = (pi + atan2(uv.y - 0.5, uv.x - ratio * 0.5));
	crc_uv.x = ang / (pi * 2);
//	crc_uv = uv;

	// background
	float dist = pow(pow(uv.x - ratio*0.5, 2) + pow(uv.y - 0.5, 2), 0.5)*1.85 * (0.4*smoothstep(0.0, 0.5, abs(uv.y - 0.5))+0.6);
	float corcle_coef = sin((crc_uv.x + time / 25.f)*pi * 8.0) * sin((crc_uv.x + time / 30.f)*pi * 4.0);
	corcle_coef = (corcle_coef + 0.5) / 20.f;
	dist *= 1.f + corcle_coef;
//	float dist = 8 * intro_coef *(abs(uv.y - 0.5)+0.1)+(pow(pow(uv.x - ratio*0.5, 2) + pow(uv.y - 0.5, 2), 0.5)*1.7 * (0.3*smoothstep(0.0, 0.5, abs(uv.y - 0.5))+0.7));
	float3 col = lerp(0.8, 0.05, smoothstep(0.3, 0.9, dist-0.2));

	float max_len = 1.3f;

	for (int t = 0; t < 2; ++t) {
		// Tentacles
		for (int i = 0; i < tenatacle_count; ++i) {
			float tex_coord = (1.f - (sin(tent_pos[i] * pi * 2) + 1) / 2) / 2;
			float local_tent_len = objTexture.Sample(objSamplerState, float2(tex_coord, 0)).x * max_len;// 0.0;
			float tent_len = local_tent_len + cam_pos;

			float4 tp = tent_params[i];
			float func = 2*tent_move(crc_uv.y, 0.5 + (tent_len+7.0)/2.0, tp.x, tp.y, tp.z, tp.w)+tent_pos[i];
			float coef = pow(crc_uv.y-0.3, 1)+local_tent_len-0.85+(rand(456+i)/30.0);
			if (coef < 0.04) coef = 0;
			float d = lerp(0, 0.02, coef);
			float dist_ten = abs(crc_uv.x-func);

			if (dist_ten < d)
				col = lerp(col, float3(0.05, 0.05, 0.05), clamp((d - dist_ten)*350, 0.0, 1.0));// float3(0.0, 0.0, 0.0);
		}
		if (crc_uv.x < 0.1)
			crc_uv.x += 1;
		else if (crc_uv.x > 0.9)
			crc_uv.x -= 1;
		else break;
	}

	// centre
	const float fade = 0.005;
	const float center_rad = 0.15;
	float shake_coef = smoothstep(0.4, 0.9, bass_coef);
	float shakex = (rand(time+100.0)-0.5) / 70.0 * shake_coef;
	float shakey = (rand(time+200.0)-0.5) / 70.0 * shake_coef;
	float shakesz = (rand(time+300.0)-0.5) / 70.0 * shake_coef;
	float dist_cent = pow(pow(uv.x - ratio*(0.5+shakex), 2) + pow(uv.y - (0.5+shakey), 2), 0.5);
	if (dist_cent < center_rad+shakesz) {
		float2 tex = (uv - float2(ratio*(0.5+shakex), (0.5+shakey)) + center_rad) / (center_rad*2);
		if (center_rad - dist_cent > fade)
			col = objTexture1.Sample(objSamplerState, tex);// 0.0;
		else
			col = lerp(col, objTexture1.Sample(objSamplerState, tex), (center_rad - dist_cent) / fade);
//		if (intro_coef)
//			col = lerp(col, 0.05, smoothstep(0.3, 0.8, dist-0.2));
	}

	if (intro_coef > 0.0)
		col = lerp(col, 0.05, intro_coef);
	float3 nz = (rand2d(uv*time)) * (0.1);
	col += nz;

	// Output to screen
	return float4(col,1.0);
}