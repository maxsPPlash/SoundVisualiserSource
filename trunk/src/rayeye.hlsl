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

float deye_cut(float3 p, float r, float shift, float blink, float z_bound)
{
	float lid_top = lerp(length(p.xy - float2(-0.0, -shift)) - r, +p.y - 0.002, blink);
	float lid_bottom = lerp(length(p.xy - float2(+0.0, shift)) - r, -p.y - 0.002, blink); 
	float cut_xy = smax(lid_top, lid_bottom, 0.01);

	float z_len = 0.; p.z > -z_bound ? 0. : p.z + z_bound;

	return cut_xy + z_len;
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

float dEye(float3 p, float size) {
	const float inner_hill_shift = -0.78;
	const float inner_hill_r = 0.25;

	float inner_hill = sdSphere(p - float3(-0.0, 0, inner_hill_shift*size), inner_hill_r*size);
	float eyeball = sdSphere(p, 1.*size);
	eyeball = smin(eyeball, inner_hill, 0.01);
	const float pupil_shift = inner_hill_shift - inner_hill_r;
	const float pupil_r = 0.02;
	float pupil = sdSphere(p - float3(-0.0, 0, pupil_shift*size), pupil_r*size);
	eyeball = smax(eyeball, -pupil, 0.0003);

	float blink = smoothstep(0.995, 1, sin(time)); // temp

	float shell_sphere = sdSphere(p, 1.02*size);// smin(, inner_hill - 0.01*size, 0.01);
	shell_sphere = smin(shell_sphere, inner_hill - 0.02*size, 0.01);

	float cut = deye_cut(p, 1.*size, 0.85*size, blink*0.99, 1.1 * size);
	float eye_shell = smax(shell_sphere, -cut, 0.01);

	float cut_t = deye_cut(p, 1.02*size, 0.85*size, 0., 1.1 * size);

	float eye = smin(eye_shell, eyeball, 0.01);
	float curve = sin(shell_sphere/size)/1.6/size;
	eye = smax(eye, cut_t+curve, 0.01);

	float beck_cut = sdSphere(p - float3(0.0, 0, pupil_shift*size), 1.*size);
	eye = smax(eye, beck_cut, 0.5);

	return eye;
}


//float dEye(float3 p, float size) {
////	float heye_r = 1.45;
////	float heye_shift = 1.1;
//
//	float inner_hill = sdSphere(p - float3(-0.0, 0, -0.68*size), 0.25*size);
//
//	float main_sphere = sdSphere(p, 1.*size);// smin(, inner_hill - 0.01*size, 0.01);
//
//	float s3 = smin(sdSphere(p - float3(-0.0, 0, 0), 0.92*size), inner_hill - 0.01*size, 0.01);
////	float s4 = dBox(p, float3(1.5, 0.1, 0.35)*size);
////	s3 = smin(s3, s4, 0.2);
//
//	float blink = smoothstep(0.995, 1, sin(time));
//
//	float cut = deye_cut(p, 1.1*size, 1.0*size, blink*0.99);
//
//	float eye = max(main_sphere, -cut);
//	float eyeball = sdSphere(p - float3(-0.0, 0, 0.0), 0.9*size);
//
//	return smin(eye, eyeball_full, 0.01)
//
//	return eye;
//
//	float eyeball = sdSphere(p - float3(-0.0, 0, 0.0), 0.9*size);
//
//	eyeball = smin(eyeball, inner_hill, 0.01);
//	float pupil = sdSphere(p - float3(-0.0, 0, -0.92*size), 0.03*size);
//
//	float eyeball_full = max(eyeball, -pupil);
//
//	float cut_t = deye_cut(p, 1.32*size, 1.1*size, 0.);
//	float curve = sin(s3*4/size)/5*size;
//
//	return smax(smin(eye, eyeball_full, 0.01), cut_t+curve, 0.01);
//}

float GetDist(float3 p) {
	float s1 = dEye(p-float3(0.3, 1.3, 0.6), 1.0);

	const float move_start = 1.;
	const float move_time = 1.;
	float dpos = time < move_start ? 0. : lerp(0., 0.8, clamp((time - move_start) / move_time, 0., 1.));
	float coef_merge = dpos * 3;

	float s2 = dEye(p-float3(0.3-dpos, 1.3-dpos, 0.6), 1.0);

	float2 res = s1; float2(smin(s1, s2, 0.001+0.3*coef_merge), 1.);

	return res;
}

float2 RayMarch(float3 ro, float3 rd) {
	float2 res = float2(0.0,-1.0);

    float tmin = 0.5;
    float tmax = 20.0;

#if 1
	// raytrace bounding plane
	float tp = (3.5-ro.y)/rd.y;
	if( tp>0.0 ) tmax = min( tmax, tp );
#endif

	// raymarch scene
	float t = tmin;
	for( int i=0; i<256 && t<tmax; i++ )
	{
		float2 h = GetDist(ro+rd*t);
		if(abs(h.x)<(0.0005*t))
		{
			res = float2(t,h.y);
			break;
		}
		t += h.x;
	}

	return res;
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

float rand(float n)
{
	return frac(sin(n)*43758.5453);
}

float rand2d(float2 v)
{
	float2 K1 = float2(23.14069263277926,2.665144142690225);
	return frac( cos( dot(v,K1) ) * 12345.6789 );
}

float rand3d(float3 v)
{
	float2 K1 = float3(23.14069263277926, 2.665144142690225, 7.5739974548463465);
	return frac( cos( dot(v,K1) ) * 12345.6789 );
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;
	uv.y *= -1.f;

	float3 sky_col = float3(0.0, 0.0, 0.0);
	float3 col = sky_col;

//	float light_coef = smoothstep(0.98, 1.0, sin(time));
//	float3 light_color = float3(1., 0.0, 0.0);

	float light_coef = 1.;
	float3 light_color = float3(1., 1.0, 1.0);

	col = clamp((1.-length(uv-float2(0., -0.5)) - 0.2*rand2d(uv+time)) * light_coef, 0, 1) * light_color;

//	float3 ro = float3(-3.f, 1.f, 0.6f);
//	float3 rd = normalize(float3(1, uv.y, uv.x));
	float3 ro = float3(-0.0f, 1.f, -2.0f);
	float3 rd = normalize(float3(uv.x+0., uv.y, 1));

	float2 d = RayMarch(ro, rd);

	float3 p = ro + rd * d.x;

	if (d.y > -0.5) {
		// moving light
		float3 lightPos = float3(0.2, 1.1, -0.9);
//		lightPos.xz += float2(sin(time/3), cos(time/3)) * 3.;
		float3 light_dir = lightPos - p;
		float3 l = normalize(light_dir) /* (pow(dot(light_dir, light_dir), 1)) * 3*/;
		float3 normal = GetNormal(p);

		float3 sky_l_dir = float3(0, 1, 0);

		// light coef
		float main_light = clamp(dot(normal, l) - rand3d(p/*+time*/)*0.05, 0, 1) * 0.5;
//		float sky_light = sqrt(clamp( 0.5+0.5*normal.y, 0.0, 1.0 ));

		col = float3(0.4, 0.4, 0.4);

		// shadow
//		float ds = RayMarch(p+normal*SURF_DIST*2.f, l);
//		float shadow_coef = 1.f;
//		if (ds < length(lightPos - p)) shadow_coef = 0.3;

		// lightning
		float3 lin = 0.;
		lin += 3 * light_coef * main_light * light_color;// * shadow_coef;
//		lin += sky_light * sky_col * 0.2;

		col = col * lin;

		// gama
		col = pow(col, 1./2.2);
	}


	// Output to screen
	return float4(col,1.0);
}