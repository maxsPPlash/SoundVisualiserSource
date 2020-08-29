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

SamplerState objSamplerState : SAMPLER : register(s0);

float3 sin_circles(float2 pos, float amp, float freq, float c_dist, float max_r, float time_mul, float z) {
	float3 col = 0;

	float c = fmod(pos.x+(100*c_dist), c_dist);
	c = pos.x - c + c_dist / 2.;
	float c_id = floor(pos.x / c_dist);
	float rmul = (max_r - 0.005) / 2;
	float r = (sin(time * time_mul + c_id / 2.) + 1) * rmul;
	float2 circle;
	circle.x = c;
	circle.y = sin(circle.x*freq)*amp;

	float3 c_color = 1;
	float2 c_vec = pos - circle;
	float dist = length(c_vec);

	float bure = 0.005 + (smoothstep(0., 1., z)) * 0.03;
	if (dist < r)
		col = c_color * smoothstep(r, r-bure, dist);

	return col;
}

float rand(float st) {
	return frac(sin(st*30.)*43758.5453123);
}

float3 rand3(float st) {
	return frac(sin(float3(1.f,2.f,4.f) * st)*43758.5453123);
}

float3 rand3s(float st) {
	return (sin(float3(1.f,1.5f,2.f) * st)+1.)/2;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;

	float r = length(uv);
	float a = atan2(uv.y, uv.x);

//	// distort
//	r = pow(r*2, 0.8); // bulge

	// polar to cartesian coordinates
//	uv = r * float2(cos(a)*0.5, sin(a)*0.5);

	float ang = 0;//time/10;
	float2x2 rot = float2x2(cos(ang), -sin(ang), sin(ang), cos(ang));

	float3 col = 1;
	float2 offset = 0.;//sin(time * float2(0.1, 0.15)) * 1.;

	float zoom = (sin(time/3)+3) / 2.f;
	float2 local_uv =mul(uv*zoom / 2.f + offset, rot);

	float sector_sz = 0.1;
	float sector = floor(local_uv.x/sector_sz);

	for (int i = -3; i <= 3; ++i) {
		float cur_sec = sector+i;
		float sec_z = rand(cur_sec*10);
		float sector_pivot = (cur_sec+0.5)*sector_sz;

		float2 local_uv_ = mul(uv*zoom*(1+sec_z*0.5) / 2.f + offset, rot);

		float2 cs_uv = local_uv_.yx - float2(rand(cur_sec*10)*10, sector_pivot);
		float amp = 0.05 + rand(cur_sec) * 0.05;
		float freq = 5 + rand(cur_sec+1) * 5;
		float max_r = 0.01 + rand(cur_sec+3) * 0.02;
		float c_dist = max_r + 0.05 + rand(cur_sec+2) * 0.05;
		col -= sin_circles(cs_uv * (1+sec_z ), amp, freq, c_dist, max_r, sign(sin(cur_sec*12)) * (1 + rand(cur_sec*12)), sec_z) * normalize(rand3s(time/3 + sin(local_uv.x) + sin(local_uv.y) + 3 * sec_z));
	}

//	col = pow(col, 3);
//	col *= normalize(rand3s(time/3 + sin(local_uv.x) + sin(local_uv.y)));

	// Output to screen
	return float4(col,1.0);
}