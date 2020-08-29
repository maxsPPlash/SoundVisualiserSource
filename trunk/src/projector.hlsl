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



float2 GetDist(float3 p) {
	float dP = 7 - p.z;
	dP += sin(p.x+time)*sin(p.y+time)/5;
	float2 res = float2(dP, 0.);

	float movement = sin(time/2) * 3;

	float bd = dBox(p-float3(movement, 1.5, 6), float3(0.3, 0.5, 1.0));
//	bd += sin(20*p.x)*sin(20*p.y)*sin(20*p.z)/20;

	if (bd < res.x)
		res = float2(bd, 1.);

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

float rand(float st) {
	return frac(sin(st*30.)*43758.5453123);
}

float3 rand3s(float st) {
	return (sin(float3(1.f,1.5f,2.f) * st)+1.)/2;
}

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

float3 circles_projection(float2 uv)
{
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
		float3 lightPos = float3(0, 5, 3);
		lightPos.xz += float2(sin(time/3), cos(time/3)) * 3.;
		float3 l = normalize(lightPos - p);
		float3 normal = GetNormal(p);

		float3 sky_l_dir = float3(0, 1, 0);

		// light coef
		float main_light = clamp(dot(normal, l), 0 , 1) * 0.5;
		float sky_light = sqrt(clamp( 0.5+0.5*normal.y, 0.0, 1.0 ));

		// material
		if (d.y < 0.5)
			col = circles_projection(p.xy); //float3(0.6, 0.8, 0.6);
		else if (d.y < 1.5)
			col = circles_projection(p.xy) - float3(1.0, 0.2, 0.1);
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