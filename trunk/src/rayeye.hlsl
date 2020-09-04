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

float SubstractRound(float a, float b, float r)
{
	float2 u = max(float2(r + a,r - b), 0);
	return min(-r, max(a, -b)) + length(u);
}

float deye_cut(float3 p, float r, float shift, float blink)
{
	float lid_top = lerp(length(p.xy - float2(-0.0, -shift)) - r, +p.y - 0.01, blink); // last one is blink
	float lid_bottom = lerp(length(p.xy - float2(+0.0, shift)) - r, -p.y - 0.01, blink);// last one is blink
	float cut = max(lid_top, lid_bottom);// SubstractRound(EyelidHi, -EyelidLo, 0.02);

	return cut;
}

float sdEllipsoid( float3 p, float3 r )
{
  float k0 = length(p/r);
  float k1 = length(p/(r*r));
  return k0*(k0-1.0)/k1;
}

float sdSphere( float3 p, float s )
{
  return length(p)-s;
}


float dEye(float3 p) {
	float heye_r = 1.45;
	float heye_shift = 1.1;

	float s3 = sdSphere(p - float3(-0.0, 0, 0), 0.95);
	float s4 = dBox(p, float3(1.5, 0.1, 0.35));
	s3 = smin(s3, s4, 0.2);

	float blink = smoothstep(0.995, 1, sin(time));

	float cut = deye_cut(p, 1.4, 1.1, blink);

	float eye1 = max(s3, -cut);

	float eyeball = sdSphere(p - float3(-0.0, 0, 0.0), 0.9);
	eyeball = smin(eyeball, sdSphere(p - float3(-0.0, 0, -0.55), 0.4), 0.01);
	float pupil = sdSphere(p - float3(-0.0, 0, -0.9), 0.06);

	float eyeball_full = max(eyeball, -pupil);

	float cut_t = deye_cut(p, 1.43, 1.1, 0.);
	float curve = sin(s3*4)/5;

	return smax(min(eye1, eyeball_full), cut_t+curve, 0.01);
}

float2 GetDist(float3 p) {
//	float dP = p.y;
//	float2 res = float2(dP, 0.);


	float s1 = dEye(p-float3(0, 1, 3));
	float2 res = float2(s1, 1.);

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

//	float3 sky_col = float3(0.5, 0.7, 0.9);
	float3 sky_col = float3(0.0, 0.0, 0.0);
	float3 col = sky_col;

//	float3 ro = float3(-2.f, 1.f, 3.f);
//	float3 rd = normalize(float3(1, uv.y, uv.x));
	float3 ro = float3(-0.f, 1.f, 0.4f);
	float3 rd = normalize(float3(uv.x, uv.y, 1));

	float2 d = RayMarch(ro, rd);

	float3 p = ro + rd * d.x;

	if (d.y > -0.5) {
		// moving light
		float3 lightPos = float3(0.2, 0.7, 0.5);
//		lightPos.xz += float2(sin(time/3), cos(time/3)) * 3.;
		float3 light_dir = lightPos - p;
		float3 l = normalize(light_dir) / (pow(dot(light_dir, light_dir), 2)) * 3;
		float3 normal = GetNormal(p);

		float3 sky_l_dir = float3(0, 1, 0);

		// light coef
		float main_light = clamp(dot(normal, l), 0 , 1) * 0.5;
		float sky_light = sqrt(clamp( 0.5+0.5*normal.y, 0.0, 1.0 ));

		// material
		if (d.y < 0.5)
			col = float3(0.6, 0.8, 0.6);
		else if (d.y < 1.5)
			col = float3(0.4, 0.4, 0.4);
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