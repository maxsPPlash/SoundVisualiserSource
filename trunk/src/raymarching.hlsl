cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;

	float _1;
	float _2;
	float _3;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

Texture2D objTexture1 : TEXTURE : register(t1);

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

float dBox(float3 p, float3 hsize) {
	return length(max(abs(p) - hsize, 0.f));
}

float smin( float a, float b, float k )
{
	float h = max(k-abs(a-b),0.0);
	return min(a, b) - h*h*0.25/k;
}

float smax( float a, float b, float k )
{
	float h = max(k-abs(a-b),0.0);
	return max(a, b) + h*h*0.25/k;
}



float2 GetDist(float3 p) {
	float4 sphere = float4(0.f, 1.f, 6.f, 1.f);
	float dS = length(p - sphere.xyz) - sphere.w;
	float dP = p.y;
	float2 res = float2(dP, 0.);

	float movement = sin(time/2) * 3;

	float cd = sdCapsule(p - float3(movement, 0, -0.9), float3(0, 1, 6), float3(1, 2, 6), .2);
//	float td = sdTorus(p-float3(0, 0.5, 6), float2(2.f, 0.1));
	float bd = dBox(p-float3(1, 1.5, 6), float3(0.3, 0.5, 1.0));

	if (bd < res.x)
		res = float2(bd, 1.);

	float d = smin(res.x, cd, 0.3);
	if (d < res.x)
		res = float2(d, 2.);

	return res;
}

float2 RayMarch(float3 ro, float3 rd) {
	float2 dO = float2(0.f, -1.);
	for (int i = 0; i < MAX_STEPS; ++i) {
		float3 p = ro + dO.x * rd;
		float2 dS = GetDist(p);
		dO.x += dS.x;
		if (dO.x > MAX_DIST) {
			break;
		}
		if (dS.x < SURF_DIST) {
			dO.y = dS.y;
			break;
		}
	}
	return dO;
}

float3 GetNormal(float3 p) {
	float d = GetDist(p);
	float2 e = float2(0.01, 0);

	float3 n = d - float3(
		GetDist(p - e.xyy).x,
		GetDist(p - e.yxy).x,
		GetDist(p - e.yyx).x);

	return normalize(n);
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;
	uv.y *= -1.f;

	float3 sky_col = float3(0.5, 0.7, 0.9);
	float3 col = sky_col;

	float3 ro = float3(0.f, 1.f, 0.f);
	float3 rd = normalize(float3(uv.x, uv.y, 1));

	float2 d = RayMarch(ro, rd);

	float3 p = ro + rd * d.x;

	if (d.y > -0.5) {
		// moving light
		float3 lightPos = float3(0, 5, 6);
		lightPos.xz += float2(sin(time/3), cos(time/3)) * 3.;
		float3 l = normalize(lightPos - p);
		float3 normal = GetNormal(p);

		float3 sky_l_dir = float3(0, 1, 0);

		// light coef
		float main_light = clamp(dot(normal, l), 0 , 1) * 0.5;
		float sky_light = sqrt(clamp( 0.5+0.5*normal.y, 0.0, 1.0 ));

		// material
		if (d.y < 0.5)
			col = float3(0.6, 0.8, 0.6);
		else if (d.y < 1.5)
			col = float3(1.0, 0.2, 0.1);
		else if (d.y < 2.5)
			col = float3(0.1, 0.2, 1.0);


		// shadow
		float ds = RayMarch(p+normal*SURF_DIST*2.f, l);
		float shadow_coef = 1.f;
		if (ds < length(lightPos - p)) shadow_coef = 0.3;

		// lightning
		float3 lin = 0.;
		lin += 3 * main_light * float3(0.8, 0.7, 0.6) * shadow_coef;
		lin += sky_light * sky_col * 0.2;

		col = col * lin;

		// gama
		col = pow(col, 1./2.2);
	}


	// Output to screen
	return float4(col,1.0);
}