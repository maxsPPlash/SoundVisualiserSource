#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include "Vertex.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBufferTypes.h"
#include <chrono>

#include <Xaudio2.h>
#include <Xaudio2fx.h>

#define PNG_STDIO_SUPPORTED
#include <png.h>

struct IMFSinkWriter;

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void RenderFrame();
private:
	bool InitializeDirectX(HWND hwnd, int width, int height);
	bool InitializeShaders();
	bool InitializeScene();
	bool InitializeAudio();
	int load_sound();

	void write_png_file(const char *filename, png_bytep *data);

//	HRESULT WriteFrame(IMFSinkWriter* pWriter, DWORD streamIndex, const LONGLONG& rtStart, const UINT32 uiWidth, const UINT32 uiHeight);

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

	VertexShader vertexshader;
	PixelShader pixelshader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indicesBuffer;


	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture1;

	IXAudio2				*_xaudio = 0;
	IXAudio2MasteringVoice	*_mastering_voice = 0;
	IXAudio2SourceVoice		*_source_voice = 0;
	WAVEFORMATEX			wfx;

	IMFSinkWriter *pSinkWriter;
	DWORD stream;
	LONGLONG rtStart;

	int wnd_w;
	int wnd_h;
	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> prev_time;
	float tent_len;
	float cam_pos;
	float time;

	int cur_img;
};