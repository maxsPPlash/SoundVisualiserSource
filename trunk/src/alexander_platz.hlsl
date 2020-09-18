cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;

	float bass_coef;
	float smooth_bass_coef;

	float c1_time;
	float c2_time;

	float eye_time;
	float eye_id;
	float eye_stage;

	float time_end;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

Texture2D papertex : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

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

// make vert_segment
float dSegment(float2 p, float2 a, float2 b)
{
    float2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

float sdBox(float2 p, float2 b)
{
	float2 d = abs(p)-b;
	return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

float dCapsule(float2 p, float2 a, float2 b, float r)
{
    return dSegment(p, a, b) - r;
}

float dSpiral(float2 p, float dr, float t) {
	float th = atan2(p.x, p.y);
	float r = length(p);

	float coef = (th + 3.1415) / (3.1415 * 2.);

	float cr = (dr*coef);

	float q = r-dr*clamp(round((r-cr)/dr),0,t);

	float d = abs(q - (dr*coef));

	return d;
}

float dColumn(float2 uv, float2 hsize) {
	uv.x = abs(uv.x);

	float d = dSegment(uv, float2(hsize.x, hsize.y), float2(hsize.x, -hsize.y));

	// make 1 check by x?
	float inner_h = hsize.y * 0.93;
	float x_sz = hsize.x * 0.225;
	float x_offs = x_sz;
	d = min(d, abs(dCapsule(uv, float2(x_offs, inner_h), float2(x_offs, -inner_h), x_sz)));
	x_offs += x_sz; // 0.45
	x_sz = hsize.x * 0.175;
	x_offs += x_sz;
	d = min(d, abs(dCapsule(uv, float2(x_offs, inner_h), float2(x_offs, -inner_h), x_sz)));
	x_offs += x_sz; // 0.45+0.35
	x_sz = hsize.x * 0.1;
	x_offs += x_sz;
	d = min(d, abs(dCapsule(uv, float2(x_offs, inner_h), float2(x_offs, -inner_h), x_sz)));

	d = min(d, dSegment(uv, float2(-hsize.x, -hsize.y*0.99), float2(hsize.x, -hsize.y*0.99)));

	float spir_r = hsize.x * 0.15;
	d = min(d, dSpiral(uv - float2(hsize.x*1.1, -hsize.y - spir_r*3), spir_r, 2));
	d = min(d, dSegment(uv, float2(-hsize.x*1.1, -hsize.y - spir_r*6), float2(hsize.x*1.1, -hsize.y - spir_r*6)));

	float2 bsz = float2(hsize.x*1.25, hsize.y*0.05);
	d = min(d, abs(sdBox(uv - float2(0., hsize.y + bsz.y), bsz)));

	return d;
}

void road(inout float3 col, float2 uv) {
	float2 loc_uv = uv;
	loc_uv.x = abs(loc_uv.x);

	float distor_fade = clamp(time/10, 0., 1.);
	float distor = sin((loc_uv.x+time)*100) * sin((uv.y+time)*100) / 7000. * bass_coef * distor_fade;

	// road lines
	float rd = dSegment(loc_uv, float2(0.05, 0.), float2(0.6, 0.7)) + distor;
	float depth_fade = clamp(loc_uv.y*4, 0.0001, 1.);
	float coef = (1.-smoothstep(0.001, 0.005 + 0.01*depth_fade , rd));
	col = lerp(col, float3(0.9, 0.9, 1), depth_fade * coef);

	// columns
	float3 col_col = 1.;
	float new_col_time = 2.2;
	float cadd = 0;

	float cmn_t = min(c1_time, c2_time);
	float cmx_t = max(c1_time, c2_time);

	float loc_c_time = time - cmx_t;

	if (loc_uv.x > 0.205 && loc_c_time < 0.5 * new_col_time)
		loc_c_time += cmx_t - cmn_t;

	float move = pow(loc_c_time, 3)/35. + 0.005;
	float2 csize = float2(0.05, 0.3) * move * 4;
	float2 shift = float2(0.07 + 1.27 * move, 1 * move - csize.y);

//	float trd = dSegment(loc_uv, float2(0., -0.064), float2(0.7, 0.7));
//	col = lerp(float3(1.0, 0.0, 0.), col, smoothstep(0.001, 0.005, trd));

	float cdepth_fade = clamp(csize*10, 0.0001, 1.);
	float d = dColumn(loc_uv - shift, csize);// + bass_coef / 100000.;

//	float d = dColumn(loc_uv, float2(0.05, 0.3));

	float bc = lerp(0.03, 0.0005 * smooth_bass_coef, clamp(time - 112., 0., 1.)) * csize * 10;

	coef = 1.-smoothstep(0.004 + bc / 2, 0.005 + bc, d);
	col = lerp(col, col_col, coef*cdepth_fade);

	coef = 1.-smoothstep(0.002, 0.004, d);
	col = lerp(col, 0., coef*cdepth_fade);

//	float3 inner_col = float3(rand(time), rand(time+3.), rand(time+20.));
//	inner_col = normalize(inner_col) / 3;

//	coef = 1.-smoothstep(-0.001, 0.0035 * clamp(bass_coef / 1000., 0, 1.), d);
//	col = lerp(col, inner_col, coef*cdepth_fade);
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

float sdSphere( float3 p, float s )
{
  return length(p)-s;
}

float dEye(float3 p, float3 lookat, float distor_mul) {
	const float inner_hill_shift = -0.78;
	const float inner_hill_r = 0.25;

	float3 look_dir = normalize(lookat);

	float inner_hill = sdSphere(p - (look_dir*inner_hill_shift), inner_hill_r);
	float eyeball = sdSphere(p, 1.);
	eyeball = smin(eyeball, inner_hill, 0.01);
	const float pupil_shift = inner_hill_shift - inner_hill_r;
	const float pupil_r = 0.02;
	float pupil = sdSphere(p - (look_dir * pupil_shift), pupil_r);
	eyeball = smax(eyeball, -pupil, 0.0003);

	if (distor_mul > 0.) {
		float tm_mul = time;
		float distor = sin(10*p.x + tm_mul)*sin(10*p.y + tm_mul)*sin(10*p.z + tm_mul) * (1+sin(time*10)) / 40.;
		eyeball += distor_mul * distor;
	}

	float blink = smoothstep(0.995, 1, sin(time)); // temp

	float shell_sphere = sdSphere(p, 1.02);// smin(, inner_hill - 0.01, 0.01);
	shell_sphere = smin(shell_sphere, inner_hill - 0.02, 0.01);

	float cut = deye_cut(p, 1., 0.85, blink*0.99, 1.1);
	float eye_shell = smax(shell_sphere, -cut, 0.01);

	float cut_t = deye_cut(p, 1.02, 0.85, 0., 1.1);

	float eye = smin(eye_shell, eyeball, 0.01);
	float curve = sin(shell_sphere)/2.0;
	eye = smax(eye, cut_t+curve, 0.01);

	float beck_cut = sdSphere(p - float3(0.0, 0, pupil_shift), 1.);
	eye = smax(eye, beck_cut, 0.5);

	return eye;
}

float GetDist(float3 p, float eye_light_coef) {
	float3 lookat = float3(0., 0., 1);
	float shake_tm = round(time / 0.2);
	lookat.xy += (float2(rand(shake_tm), rand(shake_tm+3.)) - 0.5) / 100;

	// stage 3
	float distor_mul = eye_stage > 2.5 && eye_stage < 3.5 ? eye_id / 5. : 0.;

	// stage 1
	// random lookat
	float2 lookat_range = float2(0.2, 0.07);
	float lokat_chage = 0.75 - (0.4 * distor_mul);

	float lookat_time = floor(time / lokat_chage);
	float2 prev_lookat = float2(rand(lookat_time), rand(lookat_time+5.5)) * lookat_range * 2.;
	lookat_time += 1.;
	float2 next_lookat = float2(rand(lookat_time), rand(lookat_time+5.5)) * lookat_range * 2.;
	float lokat_chage_coef = smoothstep(lokat_chage - 0.2, lokat_chage, fmod(time, lokat_chage));

	// stage2
	float look_coef = eye_stage > 1.5 && eye_stage < 2.5  ? (eye_id > 3.5 ? eye_light_coef : 1.-eye_light_coef) : 1.;

	// lokat
	lookat.xy += look_coef * (-lookat_range + lerp(prev_lookat, next_lookat, lokat_chage_coef));

	// 1st eye
	float s1 = dEye(p-float3(0.0, 1.3, 0.6), lookat, distor_mul);

	const float move_start = 1.;
	const float move_time = 1.;
	float dpos = time < move_start ? 0. : clamp((time - move_start) / move_time, 0., 1.);
	float coef_merge = dpos * 2;

	float s2 = dEye(p-float3(0.3-(1.*dpos), 1.3-(0.4*dpos), 0.6), lookat, 0.);

	float res = s1; smin(s1, s2, 0.001+0.3*coef_merge);

	return res;
}

float RayMarch(float3 ro, float3 rd, float eye_light_coef) {
	float res = -1.0;

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
		float h = GetDist(ro+rd*t, eye_light_coef);
		if(abs(h.x)<(0.0005*t))
		{
			res = t;
			break;
		}
		t += h;
	}

	return res;
}

float3 GetNormal(float3 p, float eye_light_coef) {
	float d = GetDist(p, eye_light_coef);
	float2 e = float2(0.01, 0);

	float3 n = d - float3(
		GetDist(p - e.xyy, eye_light_coef).x,
		GetDist(p - e.yxy, eye_light_coef).x,
		GetDist(p - e.yyx, eye_light_coef).x);

	return normalize(n);
}

void rm_eye(inout float3 col, float2 uv)
{
	uv.y *= -1.f;

	float loc_t = time - eye_time;

	const float fade_in = 0.1;
	const float tm = fade_in + 1.0;
	const float fade_out = tm + 0.5;
	float fi_c = smoothstep(0., fade_in, loc_t);
	float fo_c = smoothstep(fade_out, tm, loc_t);

	float eye_light_coef = fi_c * fo_c;

	if (eye_light_coef < 0.00001)
		return;
//	float3 light_color = float3(1.0, 1.0, 1.0);
	float3 light_color = float3(1.0, 0.0, 0.0);

//	col = clamp((1.-length(uv-float2(0., -0.5)) - 0.2*rand2d(uv+time)) * light_coef, 0, 1) * light_color;

	float3 ro = float3(-0.0f, 1.f, -2.0f);
	float3 rd = normalize(float3(uv.x+0., uv.y, 1));

	float d = RayMarch(ro, rd, eye_light_coef);

	float3 p = ro + rd * d.x;

	float l_col = float3(0.4, 0.4, 0.4);

	if (d > -0.5) {
		// moving light
		float3 lightPos = float3(0.8, 1.95, -1.9);
		float3 light_dir = lightPos - p;
		float3 l = normalize(light_dir) /* (pow(dot(light_dir, light_dir), 1)) * 3*/;
		float3 normal = GetNormal(p, eye_light_coef);

		// light coef
		float main_light = clamp(dot(normal, l) + rand3d(p/*+time*/)*0.05, 0, 1) * 0.5;

		// lightning
		float3 lin = 0.;
		lin += 3 * eye_light_coef * main_light * light_color;

		col = lerp(col, l_col, lin);
	}
}
/*
float dTrapezoid(float2 p, float r1, float r2, float he)
{
	float2 k1 = float2(r2,he);
	float2 k2 = float2(r2-r1,2.0*he);
	p.x = abs(p.x);
	float2 ca = float2(p.x-min(p.x,(p.y<0.0)?r1:r2), abs(p.y)-he);
	float2 cb = p - k1 + k2*clamp( dot(k1-p,k2)/dot(k2, k2), 0.0, 1.0 );
	float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
	return s*sqrt( min(dot(ca, ca),dot(cb, cb)) );
}

void plaz_tower(inout float3 col, float2 uv) {
	float2 loc_uv = uv;
	float size = 1;
	float h = 0.2;

	float d = dTrapezoid(loc_uv + float2(0., h), 0.004*size, 0.01*size, h*size);

	float coef = smoothstep(0.001, 0.004, d);
	col = lerp(1.f, col, coef);
}
*/

void bg(inout float3 col, float2 uv) {
	float2 origin = uv - float2(0, 0.5);
	float lsq = dot(origin, origin);

	col = 0.1 * papertex.Sample(objSamplerState, (uv+float2(1., 0.5)) / 2 ).x;// lerp(0.2, 0.01, lsq);
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;

	uv += (papertex.Sample(objSamplerState, input.inPosition.xy/resolution).xy-0.5) * 0.004;

	float3 col = 0;

	bg(col, uv);
	rm_eye(col, uv);
//	plaz_tower(col, uv);
	road(col, uv);

	if (time < 1)
		col = lerp(0., col, time);
	if (time > time_end - 4.)
		col = lerp(0., col, clamp(time_end - time, 0, 1));

	// Output to screen
	return float4(col,1.0);
}