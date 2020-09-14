cbuffer mycBuffer : register(b0)
{
    int wnd_w;
    int wnd_h;
	float time;

	float bass_coef;

	float c1_time;
	float c2_time;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

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

	float move = pow(loc_c_time, 3)/40. + 0.005;
	float2 csize = float2(0.05, 0.3) * move * 4;
	float2 shift = float2(0.07 + 1.27 * move, 1 * move - csize.y);

//	float trd = dSegment(loc_uv, float2(0., -0.064), float2(0.7, 0.7));
//	col = lerp(float3(1.0, 0.0, 0.), col, smoothstep(0.001, 0.005, trd));

	float cdepth_fade = clamp(csize*10, 0.0001, 1.);
	float d = dColumn(loc_uv - shift, csize);// - bass_coef / 10000.;

//	float d = dColumn(loc_uv, float2(0.05, 0.3));

	coef = 1.-smoothstep(0.001, 0.004, d);
	col = lerp(col, col_col, coef*cdepth_fade);
}

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

float bg(inout float3 col, float2 uv) {
	float2 origin = uv - float2(0, 0.5);
	float lsq = dot(origin, origin);

	col = lerp(0.2, 0.01, lsq);

	return col;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);
	float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;

	float3 col = 0;

	bg(col, uv);
//	plaz_tower(col, uv);
	road(col, uv);

	if (time < 1)
		col = lerp(0., col, time);

	// Output to screen
	return float4(col,1.0);
}