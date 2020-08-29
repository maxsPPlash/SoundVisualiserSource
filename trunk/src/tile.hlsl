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

SamplerState objSamplerState : SAMPLER : register(s0);


#define MAX_STEPS 100
#define MAX_DIST 100.f
#define SURF_DIST .01f

float dBox(float3 p, float3 hsize) {
	return length(max(abs(p) - hsize, 0.f));
}

float2 dBoxR(float3 p, float3 hsize) {
	float dist_betw = 0.1;
	float dist = hsize + dist_betw / 2;

	int2 id = (p.xz + dist) / dist / 2;

	p.xz = fmod(p.xz - dist, dist * 2.) + dist;

	float smooth_size = 0.1;

	return float2(length(max(abs(p) - hsize + smooth_size, 0.f)) - smooth_size, (-22*id.x)-id.y);
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

	float movement = sin(time/2) * 3;

	float2 bd = dBoxR(p-float3(150, -0.45, 150), float3(1., 0.5, 1.));

	float2 res = bd;

	float d = smin(res.x, dP, 0.01);
	if (d < res.x)
		res = float2(d, 0.);

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

#define AA 4

float4 main(PS_INPUT input) : SV_TARGET
{
	float2 resolution = float2(wnd_w, wnd_h);

	float3 tot = 0.0;
#if AA>1
	for( int m=0; m<AA; m++ )
	for( int n=0; n<AA; n++ )
	{
		// pixel coordinates
		float2 o = float2(float(m),float(n)) / float(AA) - 0.5;
		float2 uv = (2.0*(input.inPosition.xy+o)-resolution)/resolution.y;
#else
		float2 uv = (input.inPosition.xy-.5*resolution) / resolution.y;
#endif

		float3 sky_col = float3(0.6, 0.7, 0.8);
		float3 col = sky_col;

		float height = 5. + smoothstep(-0.5, 0.5, sin(time / 15.)) * 15;
		float move_time = (time * 1. + 10*(-cos(time/15.))) / 10;

		float3 ro = float3(0.f, height, 0.f);

		ro.x = 20 * sin(move_time/3) + 8 * sin(move_time);
		ro.z = 16 * sin(move_time/3.5) + 9 * sin(move_time*1.3);

		float3 rd = normalize(float3(uv.x, -1, uv.y));

		float2 d = RayMarch(ro, rd);

		float3 p = ro + rd * d.x;

		if (d.y > -0.5) {
			// moving light
			float3 lightPos = float3(0, 30, 6);
			lightPos.xz += float2(sin(time/3), cos(time/3)) * 10.;
			float3 lig = normalize(lightPos - p);
			float3 normal = GetNormal(p);

			float3 sky_l_dir = float3(0, 1, 0);

			float spe;
			{
				float3 hal = normalize( lig-rd );
				float dif = clamp( dot( normal, lig ), 0.0, 1.0 );
					  spe = pow( clamp( dot( normal, hal ), 0.0, 1.0 ),16.0);
					  spe *= dif;
					  spe *= 0.04+0.96*pow(clamp(1.0-dot(hal,lig),0.0,1.0),5.0);
			}

			// light coef
			float main_light = clamp(dot(normal, lig), 0 , 1) * 0.5;
			float sky_light = sqrt(clamp( 0.5+0.5*normal.y, 0.0, 1.0 ));

			// material
			if (d.y < 0.5)
				col = float3(0.6, 0.6, 0.6);
			else {
//				if ((sin(d.y * 43758.5453123) + 1) / 2 > 1 - bass_coef) {
//					col = 0.;
//				} else 
				{
					col = (0.5+sin( d.y* 30 * float3(493.1231,770.4230,377.247650) ) * 0.5);
				}
			}

			// shadow
	//		float ds = RayMarch(p+normal*SURF_DIST*2.f, lig);
			float shadow_coef = 1.f;
	//		if (ds < length(lightPos - p)) shadow_coef = 0.3;

			// lightning
			float3 lin = 0.;
//			lin += 3 * main_light * float3(0.8, 0.8, 0.8) * shadow_coef;
			lin += sky_light * sky_col * 0.2;
			lin += 80*spe*float3(1.30,1.00,0.70);

			col = col * lin;

			// gama
			col = pow(col, 1./2.2);
		}

		tot += col;
#if AA>1
    }
    tot /= float(AA*AA);
#endif

	// Output to screen
	return float4(tot,1.0);
}