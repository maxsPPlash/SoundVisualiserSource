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
Texture2D objBassTexture : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

float sdSegment( in float2 p, in float2 a, in float2 b )
{
	float2 pa = p-a, ba = b-a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length(pa - ba*h);
}

float3 Intersect(float3 planeP, float3 planeN, float3 rayP, float3 rayD)
{
    float3 d = dot(planeP, -planeN);
    float t = -(d + rayP.z * planeN.z + rayP.y * planeN.y + rayP.x * planeN.x) / (rayD.z * planeN.z + rayD.y * planeN.y + rayD.x * planeN.x);
    return rayP + t * rayD;
}

float3 door_at_the_end(float2 uv, float bass) {
	float3 col = 0;

	float cam_z = sin(time);

	float ang = sin(-cam_z/10);//time/10;
	float2x2 rot = float2x2(cos(ang), -sin(ang), sin(ang), cos(ang));
	uv = mul(uv, rot);

	float size_mul = 1 + time/100;

	float d = sdSegment(uv, float2(0.0, -0.03*size_mul), float2(0.0, 0.03*size_mul));

	float3 out_color = float3(0.4, 0.0, 0.0);

	float gate_size = 0.045*size_mul;
	float light_size = 0.05 + 0.05*bass/100;

	if (d < gate_size) {
		col = out_color + float3(0.6, 0.0, 0.0) * (1.-smoothstep(gate_size - 0.01, gate_size, d));
	} else {
		float coef = (1.-smoothstep(gate_size, gate_size + light_size, d));
		col = coef * out_color;
	}

	if (uv.y < 0.0)
		return col;

	float3 ro = float3(cam_z * 15, 15.f, 0.);
	float3 rd = normalize(float3(uv.x, uv.y, 1));

	float3 pl_p = Intersect(0., float3(0., 1., 0.), ro, rd);

	col = smoothstep(-0.1, 0.1, sin(pl_p.x) + cos(pl_p.z-time*10));

//	col *= 1 - smoothstep(0, 150, length(pl_p.xz));

	return col;
}

float3 croses(float2 uv) {
	float3 col = 0;

	float2 cells = uv * 100;

	float m = 0.1;

	float delta = abs(fmod(uv.x, m)-m/2) + abs(fmod(uv.y, m) - m/2) - m;

	if (abs(delta) < 0.01)
		col = float3(1., 1., 0.5);

	return col;
}

#define MAX_STEPS 100
#define MAX_DIST 100.f
#define SURF_DIST .01f


float dBox(float3 p, float3 hsize) {
	return length(max(abs(p) - hsize, 0.f));
}

float sdSphere( float3 p, float s )
{
  return length(p)-s;
}

float RepSphere(float3 p, float3 c, float l, float r)
{
	float3 id = clamp(round(p/c),-l,l);
	float rnd = sin(id.x*5484.5664564 + id.y * 5744.123 + id.z * 434.6789);

	float loc_r;
	if (rnd < 0.5)
		loc_r = 0;
	else
		loc_r = r * (rnd + 1.)/2.;

    float3 q = p-c*clamp(round(p/c),-l,l);
    return sdSphere( q, loc_r );
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

	float mul = 1 + (sin(time) + 1.) / 2.;

	float sd = RepSphere(p-float3(1, 1.5, 6), mul*0.3, 20., 0.1);
	float bd = dBox(p-float3(1, 1.5, 6), mul*float3(0.3, 0.5, 1.0));

//	if (sd < res.x)
//		res = float2(sd, 1.);

	float d = sd;// max(sd, bd);
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

float3 destruction(float2 uv) {
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

	return col;
}

const static float soft_eps = 0.005;

void dumbbell_circle(inout float3 col, float2 uv, float2 pos, float r, float inner_r) {
	float2 vec = uv-pos;
	float d = length(vec);

	float3 out_col = 0.6;
	float3 in_col = float3(0.6, 0.4, 0.1);

	if (d < r) {
		float out_coef = smoothstep(inner_r - soft_eps, inner_r, d);
		float coef = smoothstep(r - soft_eps, r, d);

		float3 c_col = (out_col * out_coef) + (in_col * (1.-out_coef));

		col = (coef * col) + (c_col * (1.-coef));
	}
}

void dumbbell(inout float3 col, float2 uv, float h_size, float r, float twist) {
	float sz_mul = twist > 0 ? cos(twist) : 1.;
	h_size *= sz_mul;

	// line
	float3 line_col = 0.6;//float3(0.8, 0.5, 0.2);

	float x_coef = 1.-clamp((abs(uv.x)-abs(h_size))/soft_eps, 0., 1.);
	float line_coef = (1.-smoothstep(0., soft_eps, abs(uv.y))) * x_coef;
	col = ((1.-line_coef) * col) + (line_col * (line_coef));

	float back_sg = sign(sin(twist));
	if (back_sg == 0)
		back_sg = -1;

	// circle
	float bck_r = r * (0.8 + 0.2*abs(sz_mul));
	dumbbell_circle(col, uv, float2(back_sg*h_size, 0.), bck_r, bck_r - soft_eps);
	float fwd_r = r * (1 + 0.2*(1-abs(sz_mul)));
	dumbbell_circle(col, uv, float2(-back_sg*h_size, 0.), fwd_r, fwd_r - soft_eps);
}

#define hardcoded_time 14.0

void dumbbells(inout float3 col, float2 uv) {
	float hsz = 0.4;
	float x_stride = 0.6;

	float tex_step = 1/512.;

	uv.x = abs(uv.x);

	float line_sz = 0.05;
	float c = 0.05;

	float i = c*round(uv.y/ c);
	if (abs(i) > hsz) return;

	float tex_coord = tex_step + (hsz + i) * tex_step*5;
	float tex = objBassTexture.Sample(objSamplerState, float2(tex_coord, 0.5)).x;
	dumbbell(col, uv - float2(x_stride, i), 0.05 + 0.05*tex, 0.02, time + i*8 - hsz - hardcoded_time);
}

float dcircle(inout float3 col, float2 uv, float r, float4 c_color) {
	return length(uv) - r;
}

float smin( float a, float b, float k )
{
	float h = max(k-abs(a-b),0.0);
	return min(a, b) - h*h*0.25/k;
}

float rand2d(float2 v)
{
	float2 K1 = float2(23.14069263277926,2.665144142690225);
	return frac( cos( dot(v,K1) ) * 12345.6789 );
}

void bg_circle(inout float3 col, float2 uv) {
	float c1 = dcircle(col, uv, 0.2, 0);
	float xpos = sin(time/3);
	float c2 = dcircle(col, uv-float2(xpos, 0.17), 0.05, 0);

	float3 nz = (rand2d(uv*(1.+time))) * (0.15);

	float dist = smin(c1, c2, 0.1);

	if (dist < 0)
		col += nz;

	if (abs(smin(c1, c2, 0.1))< 0.002)
		col = 1;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;

	float3 col = 0;

	bg_circle(col, uv);
	dumbbells(col, uv);

	// Output to screen
	return float4(col,1.0);
}