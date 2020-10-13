cbuffer mycBuffer : register(b0)
{
	int wnd_w;
	int wnd_h;
	float time;

	float bass_coef;

	float click_time;
	float click_id;

	float horn_time;
	float horn_id;

	float hats_time;
	float hats_id;

	float hatsin_time;

	float power300;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

#define S(a, b, t) smoothstep(a, b, t)
#define NUM_LAYERS 4.


float rand21(float2 p) {
	float3 a = frac(float3(p.xyx) * float3(213.897, 653.453, 253.098));
	a += dot(a, a.yzx + 79.76);
	return frac((a.x + a.y) * a.z);
}

float rand11(float n)
{
	return frac(sin(n)*43758.5453);
}

float2 GetPos(float2 id, float2 offs, float t) {
	float n = rand21(id+offs);
	float n1 = frac(n*10.);
	float n2 = frac(n*100.);
	float a = t+n;
	return offs + float2(sin(a*n1), cos(a*n2))*.4;
}

float GetT(float2 ro, float2 rd, float2 p) {
	return dot(p-ro, rd);
}

float LineDist(float3 a, float3 b, float3 p) {
	return length(cross(b-a, p-a))/length(p-a);
}

float df_line( in float2 a, in float2 b, in float2 p)
{
	float2 pa = p - a, ba = b - a;
	float h = clamp(dot(pa,ba) / dot(ba,ba), 0., 1.);
	return length(pa - ba * h);
}

float dline(float2 a, float2 b, float2 uv) {
	float r1 = .01;
	float r2 = .005;

	float d = df_line(a, b, uv);
	float d2 = length(a-b);
	float fade = 0.5 * S(1.5, 0.3, d2);

//	fade += S(.05, .02, abs(d2-0.1));
	return S(r1, r2, d)*fade;
}

float2 convert_id(float2 base_id, int s) {
	int x = (s % 3) - 1;
	int y = (s / 3) - 1;

	return base_id + float2(x, y);
}

float rangemul(float2 id, float d) {
	if (fmod(id.x+100.5, 3.) > 0.1 || (fmod(id.y+100.5, 3.)) > 0.1)
		return 1.;

	float wave_start = fmod(time, 2.);
	return (abs(wave_start - d)< 0.01) ? 6. : 1.;
}

float NetLayer(float2 st, float n, float t) {
	float2 id = floor(st)+n;

	st = frac(st)-.5;

	float2 p[9];
	int i=0;
	for(float y=-1.; y<=1.; y++) {
    	for(float x=-1.; x<=1.; x++) {
			p[i++] = GetPos(id, float2(x,y), t);
    	}
	}

	float m = 0.;
	float sparkle = 0.;

	float w_range = 1000.;

	float d0 = length(st-p[4]);

	for(int i=0; i<9; i++) {
		float d = length(st-p[i]);

		m += dline(p[4], p[i], st);


		float s = (.0005/(d*d));
		sparkle += s;
	}

	m += dline(p[1], p[3], st);
	m += dline(p[1], p[5], st);
	m += dline(p[7], p[5], st);
	m += dline(p[7], p[3], st);


	float sPhase = (sin(t+n)+sin(t*.1))*.25+.5;
	sPhase += pow(sin(t*.1)*.5+.5, 50.)*5.;
	m += sparkle;

	return m;
}

void bg_stars(inout float3 col, float2 uv) {
	float t = time*.1;

	float s, c;
	sincos(t, s, c);
	float2x2 rot = float2x2(c, -s, s, c);
	float2 st = mul(uv,rot);
//	M *= rot*2.;

	float m = 0.;
	for(float i=0.; i<1.; i+=1./NUM_LAYERS) {
		float z = frac(t+i);
		float size = lerp(6., 0.5, z);
		float fade = S(0., .6, z)*S(1., .8, z);

		m += fade * NetLayer(st*size-/*M**/z, i, time);
	}

	float click_loc = time - click_time;

	float2 click_pos = float2(rand11(click_id+100.123), rand11(click_id+1230.764)) * 0.7 - 0.35;

	float dist = length(uv - click_pos); // float2(0.3, 0.)
	float ccoef = clamp(1.3-(sqrt(dist)), 0., 1.) * smoothstep(0.8, 0., click_loc) / 2.;
	col += float3(0.7, 0.85, 0.9) * ccoef;

	float3 baseCol = lerp(float3(1., 1., 1.), float3(0.7, 0.85, 0.9), ccoef); // float3(s, cos(t*.4), -sin(t*.24))*.4+.6;
	col += baseCol*clamp(m, 0., 1.);
}

float sdEquilateralTriangle(float2 p)
{
    const float k = sqrt(3.0);
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0/k;
    if( p.x+k*p.y>0.0 ) p = float2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp( p.x, -2.0, 0.0 );
    return -length(p)*sign(p.y);
}

float sdCircle(float2 p, float r)
{
    return length(p) - r;
}

float smin( float a, float b, float k )
{
	float h = max(k-abs(a-b),0.0);
	return min(a, b) - h*h*0.25/k;
}

float sdTriCircle(float2 p, float r, float s)
{
	const float k = sqrt(3.0);

	float ds1 = sdCircle(p-float2(0., s), r);
	float ds2 = sdCircle(p-float2(k/2., -1./2.)*s, r);
	float ds3 = sdCircle(p-float2(-k/2., -1./2.)*s, r);

	float ds23 = smin(ds2, ds3, 0.1);
	float ds123 = smin(ds1, ds23, 0.1);

    return ds123;
}

float sdUnevenCapsule(float2 p, float r1, float r2, float h)
{
    p.x = abs(p.x);
    float b = (r1-r2)/h;
    float a = sqrt(1.0-b*b);
    float k = dot(p,float2(-b,a));
    if( k < 0.0 ) return length(p) - r1;
    if( k > a*h ) return length(p-float2(0.0,h)) - r2;
    return dot(p, float2(a,b) ) - r1;
}

float sdBox(float2 p, float2 b)
{
    float2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

float calc_figure(int f_id, float2 uv, float arg) {
	switch(f_id) {
	case 0: return sdEquilateralTriangle(uv);
	case 1: return sdCircle(uv, 1.);
	case 2: return sdTriCircle(uv, 0.6, 0.4 + arg);
	case 3: return sdUnevenCapsule(uv, 0.6, 0.4 + arg, 0.4);
	case 4: return sdBox(uv, float2(0.6, 1.f));
	}
	return 0;
}

void figures_morph(inout float3 col, float2 uv, float sz, bool shade) {
	float2 uv_loc = uv/sz * (1 + bass_coef/500.);

	float pi = 3.1415;
					// 0  1  2		3		4		5			6		7			8		9		10			11		12			13			14	15		16		17		18		19		20		21		22		23		24
	int figs[]		= {0, 2, 2,		2,		1,		1,			0,		0,			0,		2,		2,			2,		1,			1,			0,	1,		0,		3,		3,		4,		3,		3,		2,		0,		0,		};
	float turn[]	= {0, 0, 0,		0,		0.,		0.,			0.,		pi*2.5,		pi*5,	pi*5,	pi*5,		pi*5,	pi*5,		0.,			0.,	pi*2.,	pi*2.,	pi*2.,	pi*2.,	pi*1.,	pi*1.,	pi*1.,	pi*1.,	pi*1.,	pi*1.,	};
	float arg[]		= {0, 0, 0.5,	0,		0,		0,			0.,		0,			0,		0,		0.5,		0,		0,			0,			0.,	0.,		0.,		0.,		0.2,	0.,		0.0,	0.1,	0.3,	0,		0,		};

//	float time_start = 60.;

//	float time_loc = max(time - time_start, 0.);
	float time_loc = hats_id < 0 ? 0. : time - hats_time;
	int fig_sz = 25;

	float time_chane = 5.;

	int cur_id = min(int(time_loc / time_chane), fig_sz-2);
	int next_id = (cur_id + 1);

	float t_coef = clamp(fmod(time_loc, time_chane) / time_chane, 0., 1.);

	float ang_step = hats_id > 0 ? pi*1 : 0.;

	float s, c;
	float ang = lerp(turn[cur_id], turn[next_id], t_coef) + ang_step;
	sincos(ang, s, c);
//	float s = sin(t);
//	float c = cos(t);
	float2x2 rot = float2x2(c, -s, s, c);
	uv_loc = mul(uv_loc, rot);

	float d1, d2;

	if (figs[cur_id] == figs[next_id]) {
		float argt = lerp(arg[cur_id], arg[next_id], t_coef);
		d1 = d2 = calc_figure(figs[cur_id], uv_loc, argt);
	} else {
		d1 = calc_figure(figs[cur_id], uv_loc, arg[cur_id]);
		d2 = calc_figure(figs[next_id], uv_loc, arg[next_id]);
	}

	float d = lerp(d1, d2, t_coef);

	float disotr_time = time - 60.*3.;
	float distor_coef = clamp(disotr_time / 30., 0., 1.) * (1. - clamp((time - 330.) / 30., 0., 1.));
	float distor = (sin(uv.x*10. + time*1.4) + sin(uv.y*10. + time*1.7)) / 70.;
	d += distor_coef * distor;

	if (shade) {
		float shade_coef = smoothstep(0.2, 0., pow(abs(d*sz), 0.5));
		col = lerp(col, 0.0, shade_coef);
	}

	float coef = smoothstep(0.01, 0., d*sz);

	float3 light_color = 0.9;
//	float read_line_time = time - hatsin_time;
//	float red_coef = hats_id > 0 ? smoothstep(0.03, 0.01, abs(read_line_time*2. + d)) : 0.;
//	light_color = lerp(light_color, float3(0.9, 0.7, 0.7), red_coef);

	float3 figre_color = lerp(0.2, light_color, clamp((2.+cos(d*100)) / 2., 0., 1.));
	col = lerp(col, figre_color, coef);
}

void mul_figures_morph(inout float3 col, float2 uv) {
	float k = time / 2.;

	float3 old_col = col;

	float first_gorw_coef = smoothstep(0., 2., time - horn_time);
	if (horn_id != 3)
		first_gorw_coef = horn_id < 3 ? 0. : 1.;
//	float first_gorw_coef = smoothstep(55., 57., time);
	float biggest_size = 0.35 + 0.1*first_gorw_coef;

	float mul_fig_time = hats_id < 0 ? -1 : time - hats_time; //time - 60;

	float sin_hide_coef = clamp(1 - power300 * 3., 0., 1.);

	if (mul_fig_time < 0.) {
		figures_morph(col, uv, biggest_size, false);
	} else {
		for (int i = 0; i < 4; ++i) {
			if (i == 0 || sin(mul_fig_time/3.) > 1.1*sin_hide_coef + (i-1) * 0.1)
				figures_morph(col, uv, biggest_size - (i * 0.1), i != 0);
		}
	}

	float fade_coef = smoothstep(0., 3., time - horn_time);// smoothstep(8.5, 12., time);
	if (horn_id != 0)
		fade_coef = horn_id < 0 ? 0. : 1.;
	col = lerp(old_col, col, fade_coef);
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;

	float3 col = 0.;

	bg_stars(col, uv);
	mul_figures_morph(col, uv);

	const float fade_time = 6.;
	if (time < fade_time) {
		col = lerp(0., col, smoothstep(0., fade_time, time));
	}

	const float fade_out = 350;
	if (time > fade_out) {
		col = lerp(col, 0., smoothstep(fade_out, fade_out+25., time));
	}

	// Output to screen
	return float4(col,1.0);
}