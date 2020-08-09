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
#include "../IRecorder.h"
#include "../IConstBufferData.h"
#include "../IDynamicTexture.h"
#include "../IStaticTexture.h"

class IVideoStream;

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height, IRecorder *frame_recorder, const std::wstring &shader_name, IConstBufferData *cbuf, const std::vector<IDynamicTexture*> &dyn_texs, const std::vector<IStaticTexture*> &stat_texs);
	void RenderFrame();

//	CB_VS_vertexshader &data() { return shader_data; }
//	void tdata(unsigned short *val) { texture_data = val; }
//	void vdata(IVideoStream *val);
private:
	bool InitializeDirectX(HWND hwnd, int width, int height);
	bool InitializeShaders(const std::wstring &shader_name);
	bool InitializeScene();

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
//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;
//	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
//
//	Microsoft::WRL::ComPtr<ID3D11Texture2D> pVideoTexture;
//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myVideoTexture;
//
//	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture1;

	IConstBufferData *shader_data;
	std::vector<IDynamicTexture*> dyn_textures;
	std::vector<IStaticTexture*> stat_textures;
	struct DXDynTexture {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureResource;
	};
	std::vector<DXDynTexture> dx_dyn_textures;
	typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DXSatTexture;
	std::vector<DXSatTexture> dx_stat_textures;

	unsigned short *texture_data;
	IVideoStream *video;

	float width;
	float height;

	IRecorder *recorder;
};