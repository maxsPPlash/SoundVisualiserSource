#include "Graphics.h"
#include <ctime>
#include <algorithm>

#include<fftw3.h>

#pragma comment(lib, "libpng")

#pragma comment(lib,"xaudio2.lib")

const int imgs_count = 30 * (60 * 4 + 18); // 4 min 18 sec

int sample_count = 0;

template <class T> void SafeRelease(T **ppT){

    if(*ppT){
        (*ppT)->Release();
        *ppT = NULL;
    }
}

bool Graphics::InitializeAudio() {
	HRESULT					hr;
	if (!_xaudio) {
		UINT32 flags		= 0;

#ifdef XAUDIO2_DEBUG
		flags				|= XAUDIO2_DEBUG_ENGINE;
#endif

		if (FAILED(hr = XAudio2Create(&_xaudio, flags))) {
			return			false;
		}

//		_xaudio->RegisterForCallbacks(this);
	}

#if defined _WINPC && !defined _UWP
	XAUDIO2_DEVICE_DETAILS	details;
	string1024				display_name;

	u32						dev_count, dev_idx = 0;
	if (FAILED(hr = _xaudio->GetDeviceCount(&dev_count))) {
		rlog				("![SND] XAudio: Failed to enumerate XAudio2 devices: %#X [%s].", hr, ::debug::error2string(hr).c_str());
		return				false;
	}

	if (0 == dev_count)
		return				false;

	dev_idx					= u32(-1);

	rlog					("* [sound] XAudio: Available devices:");
	for (u32 k = 0; k < dev_count; ++k) {
		hr					= _xaudio->GetDeviceDetails(k, &details);
		if (FAILED(hr))		rlog(" %d - failed", k);
		else {

			if ((device_guid && wsz_cmp(details.DeviceID, device_guid) == 0) || (dev_idx == u32(-1) && details.Role & DefaultGameDevice))
				dev_idx		= k;

			sz_wc2mb		(display_name, sizeof(display_name), details.DisplayName);
			rlog			(" %d: '%s' [Ch:%d, SR:%d] - OK", k, display_name, details.OutputFormat.Format.nChannels, details.OutputFormat.Format.nSamplesPerSec);
		}
	}

	if (dev_idx == u32(-1)) {
		SAFE_RELEASE		(_xaudio);
		return				false;
	}

	if (!SUCCEEDED(hr = _xaudio->GetDeviceDetails(dev_idx, &details)))
		return				false;

	sz_wc2mb				(display_name, sizeof(display_name), details.DisplayName);
	if (details.OutputFormat.Format.nChannels < 2) {
		rlog				("![SND] XAudio: Unsupported channels count: %d.", details.OutputFormat.Format.nChannels);
		return				false;
	}
#endif // _WINPC && !_UWP

//	output_channels			= 8;
//	output_channels			= details.OutputFormat.Format.nChannels;

#ifdef _DURANGO
	if (FAILED(hr = _xaudio->CreateMasteringVoice(&_mastering_voice, 0, 0, 0, NULL, NULL))) {
#elif _UWP
	if (FAILED(hr = _xaudio->CreateMasteringVoice(&_mastering_voice))) {
#else
	if (FAILED(hr = _xaudio->CreateMasteringVoice(&_mastering_voice, 0, 0, 0, NULL, NULL))) {
#endif
//		SAFE_RELEASE		(_xaudio);
//		rlog				("![SND]  XAudio: Failed to create mastering voice: %#X [%s].", hr, ::debug::error2string(hr).c_str());
		return				false;
	}

//	WAVEFORMATEX			wfx;
//	wfx.cbSize				= 0;
//	wfx.nBlockAlign			= (WORD)(output_channels * 32);
//	wfx.nChannels			= (WORD)output_channels;
//	wfx.nSamplesPerSec		= sdef_ogg_samples_per_sec;
//	wfx.wBitsPerSample		= sdef_xa_bits_per_sample;
//	wfx.nAvgBytesPerSec		= wfx.nSamplesPerSec * wfx.nBlockAlign;
//	wfx.wFormatTag			= WAVE_FORMAT_PCM;

	XAUDIO2_SEND_DESCRIPTOR voices[] = { { 0, _mastering_voice } };
	const XAUDIO2_VOICE_SENDS sendList = { 1, voices };
	if (FAILED(hr = _xaudio->CreateSourceVoice(&_source_voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, &sendList))) {
//		SAFE_DESTROY_VOICE	(_mastering_voice);
//		SAFE_RELEASE		(_xaudio);
//		rlog				("![SND]  XAudio: Failed to create source voice: %#X [%s].", hr, ::debug::error2string(hr).c_str());
		return				false;
	}

	_source_voice->SetFrequencyRatio(1.f);

	_source_voice->Start	(0, 0);

	return					true;
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	if (!InitializeDirectX(hwnd, width, height))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	if (InitializeAudio())
		return false;

	return true;
}

DWORD rgba2argb(DWORD val) {
//	return val >> 8 | 0xFF000000;

	DWORD r = val & 0x000000FF;
	DWORD b = (val & 0x00FF0000) >> 16;
	DWORD res = val & 0xFF00FF00;
	res |= b;
	res |= r << 16;
	return res;
}

float *buf;
int soffset = 0;

/* https://www.youtube.com/watch?v=GDKFSAXZwtc
Natural
f(x)=x*Fit_factor

Exponential
f(x)=log(x*Fit_factor2)*Fit_factor

Multi Peak Scale
f(x,i)=x/Peak[i]*Fit_factor

Max Peak Scale
f(x)=x/Global_Peak*Fit_factor
*/

float clamp(float n, float lower, float upper) {
  return max(lower, min(n, upper));
}

float smoothstep(float edge0, float edge1, float x) {
  // Scale, bias and saturate x to 0..1 range
  x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  // Evaluate polynomial
  return x * x * (3 - 2 * x);
}


void Graphics::write_png_file(const char *filename, png_bytep *data) {
  int y;

  FILE *fp;
  fopen_s(&fp, filename, "wb");
  if(!fp) abort();

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR(
    png,
    info,
    wnd_w, wnd_h,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_bytep *row_pointers = data;

  if (!row_pointers) abort();

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

//  for(int y = 0; y < wnd_h; y++) {
//    free(row_pointers[y]);
//  }
//  free(row_pointers);

  fclose(fp);

  png_destroy_write_struct(&png, &info);
}


void Graphics::RenderFrame()
{
	int num_items = 2048;

	bool samples_left = soffset + 2048 < sample_count;

	float fft_bass = 0.f;
	const float bass_samples_cnt = 32;

	unsigned char tdata[512];
	if (samples_left) {
		fftwf_complex *in, *out;
		fftwf_plan p;
		/*Do fft to wav data*/
		in = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * num_items);
		out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * num_items);
		for (int i = 0; i < num_items; i++) {
			int id = (i + soffset) * 2;
			in[i][0] = buf[id];
			in[i][1] = 0;
		}

		p = fftwf_plan_dft_1d(num_items, in, out, FFTW_FORWARD, FFTW_MEASURE);  // FFTW_ESTIMATE    //1D Complex DFT, fftwf_FORWARD & BACKWARD just give direction and have particular values
		fftwf_execute(p);

		for (int i = 0; i < 512; ++i) {
			float r1 = sqrt (sqrt( pow(out[i][0],2) + pow(out[i][1],2)));
	//		float r2 = sqrt (sqrt( pow(out[i*2+1][0],2) + pow(out[i*2+1][1], 2)));
			float val = min(sqrt(r1) / 3.f, 1.f);
			tdata[i] = 255 * val;        //2 sqrt since np.sqrt( np.abs() )
			if (i < bass_samples_cnt)
				fft_bass += val;
	//		tdata[i] = 255;
		}
		fft_bass /= bass_samples_cnt;

		fftwf_free(in); fftwf_free(out);
		fftwf_destroy_plan(p);
	}

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), bgcolor);
	this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	this->deviceContext->IASetInputLayout(this->vertexshader.GetInputLayout());
	this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceContext->RSSetState(this->rasterizerState.Get());
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
	this->deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);

	D3D11_MAPPED_SUBRESOURCE mappedtResource;
	HRESULT hr = this->deviceContext->Map(pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedtResource);
	CopyMemory(mappedtResource.pData, tdata, sizeof(unsigned char) * 512);
	this->deviceContext->Unmap(pTexture.Get(), 0);
//	this->deviceContext->PSSetConstantBuffers(0, 1, pTexture.GetAddressOf());

	UINT offset = 0;

	//Update Constant Buffer
	CB_VS_vertexshader data;
	data.width = wnd_w;
	data.height = wnd_h;
	std::chrono::time_point<std::chrono::steady_clock> cur_time = std::chrono::steady_clock::now();

	// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	// FOR REALTIME
	std::chrono::duration<float> cdt = cur_time - prev_time;
	std::chrono::duration<float> diff = cur_time - start_time;
	data.time = diff.count();
	float dt = cdt.count();
	time = diff.count();

	data.time = time;
	prev_time = cur_time;
	tent_len += (0.05+smoothstep(0.4f, 0.9f, fft_bass)) * dt * 2;
	data.tent_len = tent_len;
	const float min_tent_len = 0.5f;
	if (cam_pos + min_tent_len < tent_len) {
		cam_pos += (tent_len - min_tent_len - cam_pos)* dt * 4.f;
	}
	data.cam_pos = cam_pos;
	data.bass_coef = fft_bass;

	if (samples_left) {
		int new_soffset = (44100u * time);
		if (new_soffset - soffset > 2048)
			soffset += 2048;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = this->deviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CopyMemory(mappedResource.pData, &data, sizeof(CB_VS_vertexshader));
	this->deviceContext->Unmap(constantBuffer.Get(), 0);
	this->deviceContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	//Square
	this->deviceContext->PSSetShaderResources(0, 1, this->myTexture.GetAddressOf());
	this->deviceContext->PSSetShaderResources(1, 1, this->myTexture1.GetAddressOf());
	this->deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(indicesBuffer.BufferSize(), 0, 0);

	if (false && samples_left) {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = swapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<LPVOID*>( backBuffer.GetAddressOf() ) );

		D3D11_TEXTURE2D_DESC txtDesc;
		backBuffer->GetDesc(&txtDesc);
		txtDesc.Usage = D3D11_USAGE_STAGING;
		txtDesc.BindFlags = 0;
		txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		// txtDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM

		ID3D11Texture2D *pBackBufferStaging;
		device->CreateTexture2D(&txtDesc, nullptr, &pBackBufferStaging);
		deviceContext->CopyResource(pBackBufferStaging, backBuffer.Get());

		const LONG cbWidth = 4 * wnd_w;
		const DWORD cbBuffer = cbWidth * wnd_h;

		unsigned int *data = (unsigned int *)malloc(cbBuffer);

		D3D11_MAPPED_SUBRESOURCE mappedtResource;
		hr = this->deviceContext->Map(pBackBufferStaging, 0, D3D11_MAP_READ, 0, &mappedtResource);
		CopyMemory(data, mappedtResource.pData, cbBuffer);
		this->deviceContext->Unmap(pBackBufferStaging, 0);
		pBackBufferStaging->Release();
		pBackBufferStaging = NULL;

		{
			png_bytep *png_data = (png_bytep*)malloc(sizeof(png_bytep) * wnd_h);

			for (int i = 0; i < wnd_h; ++i) {
				png_data[i] = (unsigned char*)data + i*cbWidth;
			}
			char fn[250];
			sprintf_s(fn, "imgs\\img_%d.png", cur_img);
			write_png_file(fn, (png_bytep*)png_data);

			free(png_data);
		}
		cur_img++;
	}

/*	if (data.time > 2.f && rtStart / VIDEO_FRAME_DURATION < VIDEO_FRAME_COUNT) {
		WriteFrame(pSinkWriter, stream, rtStart, wnd_w, wnd_h);
		rtStart += VIDEO_FRAME_DURATION;
		if (rtStart / VIDEO_FRAME_DURATION >= VIDEO_FRAME_COUNT) {
			hr = pSinkWriter->Finalize();
			SafeRelease(&pSinkWriter);
		}
	}*/

	this->swapchain->Present(1, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd, int width, int height)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		Log("No IDXGI Adapters found.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	wnd_w = width;
	wnd_h = height;
	start_time = std::chrono::steady_clock::now();
	prev_time = std::chrono::steady_clock::now();
	tent_len = 0.f;
	cam_pos = 0.f;
	cur_img = 0;
	time = 0.f;

	scd.BufferDesc.Width = width;
	scd.BufferDesc.Height = height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(	adapters[0].pAdapter, //IDXGI Adapter
										D3D_DRIVER_TYPE_UNKNOWN,
										NULL, //FOR SOFTWARE DRIVER TYPE
										D3D11_CREATE_DEVICE_DEBUG, //FLAGS FOR RUNTIME LAYERS
										NULL, //FEATURE LEVELS ARRAY
										0, //# OF FEATURE LEVELS IN ARRAY
										D3D11_SDK_VERSION,
										&scd, //Swapchain description
										this->swapchain.GetAddressOf(), //Swapchain Address
										this->device.GetAddressOf(), //Device Address
										NULL, //Supported feature level
										this->deviceContext.GetAddressOf()); //Device Context Address

	if (FAILED(hr))
	{
		Log(hr, "Failed to create device and swapchain.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr)) //If error occurred
	{
		Log(hr, "GetBuffer Failed.");
		return false;
	}

	hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		Log(hr, "Failed to create render target view.");
		return false;
	}

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		Log(hr, "Failed to create depth stencil buffer.");
		return false;
	}

	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		Log(hr, "Failed to create depth stencil view.");
		return false;
	}

	this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		Log(hr, "Failed to create depth stencil state.");
		return false;
	}

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Set the Viewport
	this->deviceContext->RSSetViewports(1, &viewport);

	//Create Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
	if (FAILED(hr))
	{
		Log(hr, "Failed to create rasterizer state.");
		return false;
	}

	spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceContext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

	//Create sampler description for sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf()); //Create sampler state
	if (FAILED(hr))
	{
		Log(hr, "Failed to create sampler state.");
		return false;
	}

	return true;
}

bool Graphics::InitializeShaders()
{

	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug Mode
	#ifdef _WIN64 //x64
			shaderfolder = L"x64\\Debug\\";
	#else  //x86 (Win32)
			shaderfolder = L"Debug\\";
	#endif
	#else //Release Mode
	#ifdef _WIN64 //x64
			shaderfolder = L"x64\\Release\\";
	#else  //x86 (Win32)
			shaderfolder = L"Release\\";
	#endif
#endif
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layout);

	if (!vertexshader.Initialize(this->device, shaderfolder + L"vertexshader.cso", layout, numElements))
		return false;

	if (!pixelshader.Initialize(this->device, shaderfolder + L"pixelshader.cso"))
		return false;


	return true;
}

#include<sndfile.h>

#include<fstream>
#include<vector>
#include<math.h>
#include <algorithm>
#include<iostream>
using namespace std;

#define file_path "E:/Projects/SoundVisualiser_v1_stable/DirectX-11-Engine-VS2017-Tutorial_24/DirectX 11 Engine VS2017/DirectX 11 Engine VS2017/Summer.wav"
//#define file_path "D:/Tests/DirectX-11-Engine-VS2017-Tutorial_24/DirectX 11 Engine VS2017/DirectX 11 Engine VS2017/Summer.wav"

int Graphics::load_sound() {
	char        *infilename;
	SNDFILE     *file = NULL;

	SF_INFO     sfinfo;
	int num_channels;
	int num, num_items;
//	double *buf;
	int samplerate, ch;
	int i, j;

	FILE        *outfile = NULL;

	//Read the file, into buffer
	file = sf_open(file_path, SFM_READ, &sfinfo);


	/* Print some of the info, and figure out how much data to read. */
	sample_count = sfinfo.frames;
	samplerate = sfinfo.samplerate;
	ch = sfinfo.channels;
	printf("frames=%d\n", sample_count);
	printf("samplerate=%d\n", samplerate);
	printf("channels=%d\n", ch);
	num_items = sample_count * ch;
	printf("num_items=%d\n", num_items);

	//Allocate space for the data to be read, then read it
	buf = (float *)malloc(num_items * sizeof(float));
	num = sf_read_float(file, buf, num_items);

	sf_close(file);
	printf("Read %d items\n", num);

	wfx.cbSize				= 0;
	wfx.nBlockAlign			= (WORD)(8 * 16);
	wfx.nChannels			= (WORD)8;
	wfx.nSamplesPerSec		= samplerate;
	wfx.wBitsPerSample		= 16;
	wfx.nAvgBytesPerSec		= wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.wFormatTag			= WAVE_FORMAT_PCM;

	return 0;
}

bool Graphics::InitializeScene()
{
	load_sound();

	//Textured Square
	Vertex v[] =
	{
		Vertex(-1.0f,  -1.0f, 1.0f, 0.0f, 1.0f), //Bottom Left   - [0]
		Vertex(-1.0f,   1.0f, 1.0f, 0.0f, 0.0f), //Top Left      - [1]
		Vertex( 1.0f,   1.0f, 1.0f, 1.0f, 0.0f), //Top Right     - [2]
		Vertex( 1.0f,  -1.0f, 1.0f, 1.0f, 1.0f), //Bottom Right   - [3]
	};

	//Load Vertex Data
	HRESULT hr = this->vertexBuffer.Initialize(this->device.Get(), v, ARRAYSIZE(v));
	if (FAILED(hr))
	{
		Log(hr, "Failed to create vertex buffer.");
		return false;
	}

	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	//Load Index Data
	
	hr = this->indicesBuffer.Initialize(this->device.Get(), indices, ARRAYSIZE(indices));
	if (FAILED(hr))
	{
		Log(hr, "Failed to create indices buffer.");
		return hr;
	}

	//Load Texture
	D3D11_TEXTURE2D_DESC tdesc;
	tdesc.Width = 512;
	tdesc.Height = 1;
	tdesc.MipLevels = tdesc.ArraySize = 1;
	tdesc.Format = DXGI_FORMAT_R8_UNORM;
	tdesc.SampleDesc.Count = 1;
	tdesc.SampleDesc.Quality = 0;
	tdesc.Usage = D3D11_USAGE_DYNAMIC;
	tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tdesc.MiscFlags = 0;

//	pTexture = NULL;
    hr = device->CreateTexture2D(&tdesc, NULL, &pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	// Setup the shader resource view description.
	srvDesc.Format = tdesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hr = device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &myTexture);
	if(FAILED(hr))
	{
		return false;
	}

	hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\piano.png", nullptr, myTexture1.GetAddressOf());
	if (FAILED(hr))
	{
		Log(hr, "Failed to create wic texture from file.");
		return false;
	}

	//Initialize Constant Buffer(s)
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = static_cast<UINT>(sizeof(CB_VS_vertexshader) + (16 - (sizeof(CB_VS_vertexshader) % 16)));
	desc.StructureByteStride = 0;

	hr = device->CreateBuffer(&desc, 0, constantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		Log(hr, "Failed to initialize constant buffer.");
		return false;
	}

	return true;
}
