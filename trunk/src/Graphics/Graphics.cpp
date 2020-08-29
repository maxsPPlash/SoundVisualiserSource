#include "Graphics.h"
#include <ctime>
#include <algorithm>
#include "../IVideoStream.h"

bool Graphics::Initialize(HWND hwnd, int wnd_width, int wnd_height, IRecorder *frame_recorder, const std::wstring &shader_name, IConstBufferData *cbuf, const std::vector<IDynamicTexture*> &dyn_texs, const std::vector<IStaticTexture*> &stat_texs)
{
	shader_data = cbuf;
	dyn_textures = dyn_texs;
	stat_textures = stat_texs;
	recorder = frame_recorder;

	if (!InitializeDirectX(hwnd, wnd_width, wnd_height))
		return false;

	if (!InitializeShaders(shader_name))
		return false;

	if (!InitializeScene())
		return false;

	return true;
}

//void Graphics::vdata(IVideoStream *val) {
//	video = val;
//}

//DWORD rgba2argb(DWORD val) {
////	return val >> 8 | 0xFF000000;
//
//	DWORD r = val & 0x000000FF;
//	DWORD b = (val & 0x00FF0000) >> 16;
//	DWORD res = val & 0xFF00FF00;
//	res |= b;
//	res |= r << 16;
//	return res;
//}

void Graphics::RenderFrame()
{
	HRESULT hr;

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

	if (shader_data) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hr = this->deviceContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CopyMemory(mappedResource.pData, shader_data->Data(), shader_data->Size());
		this->deviceContext->Unmap(constantBuffer.Get(), 0);
		this->deviceContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
	}


	for (int i = 0, size = dyn_textures.size(); i < size; ++i) {
		DXDynTexture &dx_t = dx_dyn_textures[i];

		D3D11_MAPPED_SUBRESOURCE mappedTex;
		hr = this->deviceContext->Map(dx_t.Texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedTex);
		if (FAILED(hr))
		{
			throw std::runtime_error("surface mapping failed!");
		}

		dyn_textures[i]->GetData(mappedTex.pData, mappedTex.RowPitch);

		this->deviceContext->Unmap(dx_t.Texture.Get(), 0);
	}

	//Square
	UINT offset = 0;
	for (int i = 0, size = dyn_textures.size(); i < size; ++i) {
		this->deviceContext->PSSetShaderResources(dyn_textures[i]->RegisterId(), 1, dx_dyn_textures[i].TextureResource.GetAddressOf());
	}
	for (int i = 0, size = stat_textures.size(); i < size; ++i) {
		this->deviceContext->PSSetShaderResources(stat_textures[i]->RegisterId(), 1, dx_stat_textures[i].GetAddressOf());
	}
	this->deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(indicesBuffer.BufferSize(), 0, 0);


	if (recorder/* && recorder->Finished()*/) {
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

		const DWORD buf_size = 4 * width * height;

		unsigned int *data = (unsigned int *)malloc(buf_size);

		D3D11_MAPPED_SUBRESOURCE mappedtResource;
		hr = this->deviceContext->Map(pBackBufferStaging, 0, D3D11_MAP_READ, 0, &mappedtResource);
		CopyMemory(data, mappedtResource.pData, buf_size);
		this->deviceContext->Unmap(pBackBufferStaging, 0);
		pBackBufferStaging->Release();
		pBackBufferStaging = NULL;

		recorder->SaveNewFrame(data);
	}

	this->swapchain->Present(1, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd, int wnd_width, int wnd_height)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		Log("No IDXGI Adapters found.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	width = wnd_width;
	height = wnd_height;

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

//	spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceContext.Get());
//	spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

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

bool Graphics::InitializeShaders(const std::wstring &shader_name)
{

	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug Mode
	#ifdef _WIN64 //x64
			shaderfolder = L"..\\bin\\";//L"x64\\Debug\\";
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

	if (!pixelshader.Initialize(this->device, shaderfolder + shader_name + L".cso"/*L"pixelshader.cso"*/))
		return false;


	return true;
}

DXGI_FORMAT TextureType2DX(DynTextureType t) {
	switch (t)
	{
	case DT_U8RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case DT_U16R:
		return DXGI_FORMAT_R16_UNORM;
		break;
	case DT_U8R:
		return DXGI_FORMAT_R8_UNORM;
		break;
	default:
		// NO!
		break;
	}

	return DXGI_FORMAT_R8G8B8A8_UNORM;
}

bool Graphics::InitializeScene()
{
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

	for(IStaticTexture *dt : stat_textures) {
		dx_stat_textures.emplace_back();
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> &dx_t = dx_stat_textures.back();

		hr = DirectX::CreateWICTextureFromFile(this->device.Get(), dt->Path().c_str(), nullptr, dx_t.GetAddressOf());
		if (FAILED(hr)) {
			Log(hr, "Failed to create wic texture from file.");
			return false;
		}
	}

	for(IDynamicTexture *dt : dyn_textures) {
		dx_dyn_textures.emplace_back();
		DXDynTexture &dx_t = dx_dyn_textures.back();

		D3D11_TEXTURE2D_DESC desc_rgba;
		desc_rgba.Width              = dt->Width(); // 352
		desc_rgba.Height             = dt->Height(); // 288
		desc_rgba.MipLevels          = 1;
		desc_rgba.ArraySize          = 1;
		desc_rgba.Format             = TextureType2DX(dt->Type());
		desc_rgba.SampleDesc.Count   = 1;
		desc_rgba.SampleDesc.Quality = 0;
		desc_rgba.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		desc_rgba.Usage              = D3D11_USAGE_DYNAMIC;
		desc_rgba.CPUAccessFlags     = D3D11_CPU_ACCESS_WRITE;
		desc_rgba.MiscFlags          = 0;

		hr = device->CreateTexture2D(&desc_rgba, 0, &dx_t.Texture);
		if(FAILED(hr))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		// Setup the shader resource view description.
		srvDesc.Format = desc_rgba.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		// Create the shader resource view for the texture.
		hr = device->CreateShaderResourceView(dx_t.Texture.Get(), &srvDesc, &dx_t.TextureResource);
		if(FAILED(hr))
		{
			return false;
		}
	}

	if (shader_data) {
		//Initialize Constant Buffer(s)
		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		int sz = shader_data->Size();
		desc.ByteWidth = static_cast<UINT>(sz + (16 - (sz % 16)));
		desc.StructureByteStride = 0;

		hr = device->CreateBuffer(&desc, 0, constantBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			Log(hr, "Failed to initialize constant buffer.");
			return false;
		}
	}

	return true;
}
