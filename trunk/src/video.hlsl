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

#define MAX_STEPS 100
#define MAX_DIST 100.f
#define SURF_DIST .01f

float sdCapsule(float3 p, float3 a, float3 b, float r) {
	float3 ab = b - a;
	float3 ap = p - a;

	float t = dot(ab, ap) / dot(ab, ab);
	t = clamp(t, 0, 1);

	float3 c = a + t*ab;
	return length(p-c) - r;
}

float sdTorus(float3 p, float2 r) {
	float x = length(p.xz) - r.x;
	return length(float2(x, p.y)) - r.y;
}

float GetDist(float3 p) {
	float4 sphere = float4(0.f, 1.f, 6.f, 1.f);
	float dS = length(p - sphere.xyz) - sphere.w;
	float dP = p.y;

	float cd = sdCapsule(p, float3(0, 1, 6), float3(1, 2, 6), .2);
	float td = sdTorus(p-float3(0, 0.5, 6), float2(2.f, 0.1));
	float d = min(td, dP);
	d = min(d, cd);
	return d;
}

float RayMarch(float3 ro, float3 rd) {
	float dO = 0.f;
	for (int i = 0; i < MAX_STEPS; ++i) {
		float3 p = ro + dO * rd;
		float dS = GetDist(p);
		dO += dS;
		if (dS < SURF_DIST || dO > MAX_DIST) break;
	}
	return dO;
}

float3 GetNormal(float3 p) {
	float d = GetDist(p);
	float2 e = float2(0.01, 0);

	float3 n = d - float3(
		GetDist(p - e.xyy),
		GetDist(p - e.yxy),
		GetDist(p - e.yyx));

	return normalize(n);
}

float GetLight(float3 p) {
	float3 lightPos = float3(0, 5, 6);
	lightPos.xz += float2(sin(time), cos(time)) * 3.;

	float3 l = normalize(lightPos - p);
	float3 n = GetNormal(p);

	float dif = clamp(dot(n, l), 0 , 1);
	float d = RayMarch(p+n*SURF_DIST*2.f, l);
	if (d < length(lightPos - p)) dif *= .1;

	return dif;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy) / resolution;
//	uv.y *= -1.f;

	float3 col = 0.f;

//	float3 text_color = (1, 0, 0);//objTexture2.Sample(objSamplerState, uv);
//	float3 inv_color = 1. - text_color;
//
//	col = text_color * (1 - bass_coef) + inv_color * bass_coef;

	float2 offset = float2(.01,.0) * bass_coef;
	col.r = objTexture.Sample(objSamplerState, uv+offset.xy).r;
	col.g = objTexture.Sample(objSamplerState, uv          ).g;
	col.b = objTexture.Sample(objSamplerState, uv+offset.yx).b;

	// Output to screen
	return float4(col,1.0);
}