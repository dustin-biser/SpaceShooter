//
// D3D12Renderer.hpp
//
#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>

#include "Core/Common.hpp"
#include "Graphics/IRenderer.hpp"
#include "Graphics/ShaderUtils.hpp"


class D3D12Renderer : public IRenderer {
public:
	D3D12Renderer ();
	~D3D12Renderer ();

	void initialize (
		HWND hWindow
	) override;

	void render () override;

	void present () override;


private:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool m_vsyncEnabled = true;

	// Number of rendered frames to pre-flight for execution on the GPU.
	static const uint NUM_BUFFERED_FRAMES = 3;
	uint m_frameIndex;

	uint m_framebufferWidth;
	uint m_framebufferHeight;

	ComPtr<ID3D12Device> m_device;

	// Direct Command Queue Related
	ComPtr<ID3D12CommandQueue> m_directCmdQueue;
	ComPtr<ID3D12CommandAllocator> m_directCmdAllocator[NUM_BUFFERED_FRAMES];
	ComPtr<ID3D12GraphicsCommandList> m_drawCmdList[NUM_BUFFERED_FRAMES];


	ComPtr<IDXGISwapChain3> m_swapChain;
	HANDLE m_frameLatencyWaitableObject;

	// Vertex data
	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ushort m_indexCount;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	ShaderGroup m_shaderGroup;


	// Resources that are referenced by descriptor handles (a.k.a. resource views).
	struct HandledResource {
		union {
			D3D12_CPU_DESCRIPTOR_HANDLE descHandle;
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		};

		ID3D12Resource * resource = nullptr;

		~HandledResource () { if (resource) resource->Release (); }
	};

	// Depth-Stencil Resource 
	ComPtr<ID3D12DescriptorHeap> m_dsvDescHeap;
	HandledResource m_depthStencilBuffer;

	// Render Target Resources
	ComPtr<ID3D12DescriptorHeap> m_rtvDescHeap;
	HandledResource m_renderTarget[NUM_BUFFERED_FRAMES];


	// Synchronization objects.
	HANDLE m_frameFenceEvent[NUM_BUFFERED_FRAMES];
	ComPtr<ID3D12Fence> m_frameFence[NUM_BUFFERED_FRAMES];
	uint64 m_currentFenceValue;
	uint64 m_fenceValue[NUM_BUFFERED_FRAMES];

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	void createDeviceAndSwapChain (
		HWND hWindow,
		uint framebufferWidth,
		uint framebufferHeight
	);

	void createHardwareDevice (
		IDXGIFactory4 * dxgiFactory
	);

	void getHardwareAdapter (
		IDXGIFactory2 * pFactory,
		D3D_FEATURE_LEVEL featureLevel,
		IDXGIAdapter1 ** ppAdapter
	);

	void createDepthStencilBuffer (
		uint bufferWidth,
		uint bufferHeight
	);

	void createRenderTargetViews ();

	void createFenceObjects ();

	void createDrawCommandLists ();

	void createCommandList (
		ComPtr<ID3D12GraphicsCommandList> & commandList,
		ComPtr<ID3D12CommandAllocator> & cmdAllocator
	);

	void createPipelineState (
		const ShaderGroup & shaderGroup
	);

	void createRootSignature ();

	void finalizeRender (
		ID3D12GraphicsCommandList * drawCmdList,
		ID3D12CommandQueue * commandQueue
	);

	void initViewportAndScissorRect ();


	void loadAssets (
		ID3D12GraphicsCommandList * uploadCmdList,
		ComPtr<ID3D12Resource> & uploadBuffer
	);

	void prepareRender (
		ID3D12CommandAllocator * commandAllocator,
		ID3D12GraphicsCommandList * drawCmdList
	);

	void presentAndIncrementFrame ();

	void recordDrawingCommands (
		ID3D12GraphicsCommandList * drawCmdList
	);

	bool swapChainWaitableObjectIsSignaled ();

	void uploadVertexDataToDefaultHeap (
		ID3D12GraphicsCommandList * uploadCmdList,
		ComPtr<ID3D12Resource> & uploadBuffer
	);

	/// Blocks current thread until GPU signals fence on command queue.
	void D3D12Renderer::waitForGpuCompletion (
		ID3D12CommandQueue * commandQueue
	);

	//virtual void Update () = 0;

	//virtual void Render (
	//	ID3D12GraphicsCommandList * drawCmdList
	//) = 0;

	//void PrepareRender (
	//	ID3D12CommandAllocator * commandAllocator,
	//	ID3D12GraphicsCommandList * drawCmdList
	//);

	//void FinalizeRender (
	//	ID3D12GraphicsCommandList * drawCmdList,
	//	ID3D12CommandQueue * commandQueue
	//);

	//void Present ();

	//__forceinline bool SwapChainWaitableObjectIsSignaled ();


	///// Helper function for resolving the full path of assets.
	//std::wstring GetAssetPath (
	//	const wchar_t * assetName
	//);

	///// Helper function for resolving the full path of assets.
	///// Works with basic multi-byte strings.
	//std::string GetAssetPath (
	//	const char * assetName
	//);

	//void SetCustomWindowText (
	//	LPCWSTR text
	//);

	///// Path of the demo's current working directory.
	//std::wstring m_workingDirPath;

	///// The shared solution asset path.
	//std::wstring m_sharedAssetPath;

	//// Window title.
	//std::wstring m_windowTitle;
};
	

