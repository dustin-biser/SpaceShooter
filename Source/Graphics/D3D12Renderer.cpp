//
// D3D12Renderer.cpp
//
#include "pch.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <string>
#include <cstring>

#include "Core/AssetLoader.hpp"

#include "Graphics/D3D12Renderer.hpp"
#include "Graphics/d3dx12.h"

internal void waitForGpuFence (
	ID3D12Fence * fence,
	uint64 completionValue,
	HANDLE fenceEvent
);


//---------------------------------------------------------------------------------------
D3D12Renderer::D3D12Renderer ()
	: m_frameIndex (0),
	  m_fenceValue {0}
{

}

//---------------------------------------------------------------------------------------
D3D12Renderer::~D3D12Renderer ()
{
	// Signal command-queue 
	m_directCmdQueue->Signal (m_frameFence[m_frameIndex].Get (), m_currentFenceValue);
	m_fenceValue[m_frameIndex] = m_currentFenceValue;
	++m_currentFenceValue;

	// Wait for command queue to finish processing all buffered frames.
	for (int i (0); i < NUM_BUFFERED_FRAMES; ++i) {
		::waitForGpuFence (m_frameFence[i].Get (), m_fenceValue[i], m_frameFenceEvent[i]);
	}

	// Clean up event handles
	for (auto event : m_frameFenceEvent) {
		CloseHandle (event);
	}

	// Uninitialize COM library.
	CoUninitialize ();
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::initialize (
	HWND hWindow
) {
#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	ComPtr<ID3D12Debug> debugController;
	if ( SUCCEEDED (D3D12GetDebugInterface (IID_PPV_ARGS (&debugController))) ) {
		LOG_INFO ("D3D12 Debug Layer Enabled.");
		debugController->EnableDebugLayer ();
	}
#endif

	// Check for DirectXMath support.
	if (!DirectX::XMVerifyCPUSupport ()) {
		ForceBreak ("No support for DirectXMath.");
	}

	// Initialize COM library.
	CHECK_WIN_RESULT (
		CoInitializeEx (nullptr, COINIT_MULTITHREADED)
	);

	// Calculate framebuffer dimensions from window rect.
	RECT windowRect = {};
	GetWindowRect (hWindow, &windowRect);
	m_framebufferWidth = windowRect.right - windowRect.left;
	m_framebufferHeight = windowRect.bottom - windowRect.top;

	createDeviceAndSwapChain (hWindow, m_framebufferWidth, m_framebufferHeight);

	createDepthStencilBuffer (m_framebufferWidth, m_framebufferHeight);

	createRenderTargetViews ();

	createFenceObjects ();

	createDrawCommandLists ();

	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
	createCommandList (uploadCmdList, cmdAllocator);

	// Temporary resource buffer to aid in uploading data to default heap.
	// Resource will be released at end of scope.
	ComPtr<ID3D12Resource> uploadBuffer;

	loadAssets (uploadCmdList.Get (), uploadBuffer);

	// Close upload command list and execute it on the direct command queue. 
	{
		CHECK_D3D_RESULT (
			uploadCmdList->Close ()
		);
		ID3D12CommandList * commandLists[] = {uploadCmdList.Get ()};
		m_directCmdQueue->ExecuteCommandLists (_countof (commandLists), commandLists);

		waitForGpuCompletion (m_directCmdQueue.Get ());
	}
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::render ()
{
	// Wait until GPU has processed the previous frame before building new one.
	::waitForGpuFence (
		m_frameFence[m_frameIndex].Get (),
		m_fenceValue[m_frameIndex],
		m_frameFenceEvent[m_frameIndex]
	);

	if (m_vsyncEnabled) {
		// Wait until swap chain has finished presenting all queued frames before building
		// command lists and rendering next frame.  This will reduce latency for the next
		// rendered frame.
		WaitForSingleObject (m_frameLatencyWaitableObject, INFINITE);
	}

	// Acquire commandList and command allocator for current frame.
	auto drawCmdList = m_drawCmdList[m_frameIndex].Get ();
	auto commandAllocator = m_directCmdAllocator[m_frameIndex].Get ();

	prepareRender (commandAllocator, drawCmdList);

	recordDrawingCommands (drawCmdList);

	finalizeRender (drawCmdList, m_directCmdQueue.Get ());
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::prepareRender (
	ID3D12CommandAllocator * commandAllocator,
	ID3D12GraphicsCommandList * drawCmdList
) {
	CHECK_D3D_RESULT (
		commandAllocator->Reset ()
	);

	CHECK_D3D_RESULT (
		// Forgo setting pipeline-state for now.
		drawCmdList->Reset (commandAllocator, nullptr)
	);

	// Set viewport to size of full window.
	D3D12_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(m_framebufferWidth);
	viewport.Height = static_cast<float>(m_framebufferHeight);
	viewport.MaxDepth = 1.0f;

	// Set scissor rect to size of full window.
	D3D12_RECT scissorRect = {};
	scissorRect.right = m_framebufferWidth;
	scissorRect.bottom = m_framebufferHeight;

	drawCmdList->RSSetViewports (1, &viewport);
	drawCmdList->RSSetScissorRects (1, &scissorRect);

	// Indicate that the back buffer for the current frame will be used as a render target.
	drawCmdList->ResourceBarrier (1,
		&CD3DX12_RESOURCE_BARRIER::Transition (
			m_renderTarget[m_frameIndex].resource,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

	// Acquire handle to Depth-Stencil View.
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle (m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart ());

	// Acquire handle to Render Target View.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle (m_renderTarget[m_frameIndex].rtvHandle);

	drawCmdList->OMSetRenderTargets (1, &rtvHandle, FALSE, &dsvHandle);

	// Clear render target.
	const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
	drawCmdList->ClearRenderTargetView (rtvHandle, clearColor, 0, nullptr);

	// Clear the depth/stencil buffer.
	drawCmdList->ClearDepthStencilView (dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr
	);
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::finalizeRender (
	ID3D12GraphicsCommandList * drawCmdList,
	ID3D12CommandQueue * commandQueue
) {
	// Indicate that the back buffer will now be used to present.
	drawCmdList->ResourceBarrier (1,
		&CD3DX12_RESOURCE_BARRIER::Transition (
			m_renderTarget[m_frameIndex].resource,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		)
	);

	CHECK_D3D_RESULT (
		drawCmdList->Close ()
	);

	// Execute the command list.
	ID3D12CommandList* commandLists[] = {drawCmdList};
	commandQueue->ExecuteCommandLists (_countof (commandLists), commandLists);
}



//---------------------------------------------------------------------------------------
void D3D12Renderer::recordDrawingCommands (
	ID3D12GraphicsCommandList * drawCmdList
) {
	// Set necessary state.
	drawCmdList->SetPipelineState (m_pipelineState.Get ());
	drawCmdList->SetGraphicsRootSignature (m_rootSignature.Get ());

	drawCmdList->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	drawCmdList->IASetVertexBuffers (0, 1, &m_vertexBufferView);
	drawCmdList->IASetIndexBuffer (&m_indexBufferView);

	drawCmdList->DrawIndexedInstanced (m_indexCount, 1, 0, 0, 0);
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::present ()
{
	if (m_vsyncEnabled) {
		presentAndIncrementFrame ();
	}
	else if (swapChainWaitableObjectIsSignaled ()) {
		// Swap-chain is available to queue up another present, so do that now.
		presentAndIncrementFrame ();
	}
	else {
		// Start over and rebuild the frame again, rendering to the same indexed back buffer.
		m_directCmdQueue->Signal (m_frameFence[m_frameIndex].Get (), m_currentFenceValue);
		m_fenceValue[m_frameIndex] = m_currentFenceValue;
		++m_currentFenceValue;
	}
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::presentAndIncrementFrame ()
{
	const uint syncInterval = m_vsyncEnabled;
	CHECK_D3D_RESULT (
		m_swapChain->Present (syncInterval, 0)
	);

	m_directCmdQueue->Signal (m_frameFence[m_frameIndex].Get (), m_currentFenceValue);
	m_fenceValue[m_frameIndex] = m_currentFenceValue;
	++m_currentFenceValue;

	m_frameIndex = (m_frameIndex + 1) % NUM_BUFFERED_FRAMES;
}

//---------------------------------------------------------------------------------------
__forceinline bool D3D12Renderer::swapChainWaitableObjectIsSignaled ()
{
	return WAIT_OBJECT_0 == WaitForSingleObjectEx (m_frameLatencyWaitableObject, 0, true);
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createDeviceAndSwapChain (
	HWND hWindow,
	uint framebufferWidth,
	uint framebufferHeight
) {
	ComPtr<IDXGIFactory4> dxgiFactory;

	CHECK_D3D_RESULT (
		::CreateDXGIFactory1 (IID_PPV_ARGS (&dxgiFactory))
	);

	//TODO (Dustin) - Prevent full screen transitions for now, but want to allow later.
	CHECK_D3D_RESULT (
		dxgiFactory->MakeWindowAssociation (hWindow, DXGI_MWA_NO_ALT_ENTER)
	);

	createHardwareDevice (dxgiFactory.Get());

	// Describe and create the direct command queue.
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		CHECK_D3D_RESULT (
			m_device->CreateCommandQueue (&queueDesc, IID_PPV_ARGS (&m_directCmdQueue))
		);
		SET_D3D_DEBUG_NAME (m_directCmdQueue);
	}


	// Describe and create the swap chain.
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = NUM_BUFFERED_FRAMES;
		swapChainDesc.Width = framebufferWidth;
		swapChainDesc.Height = framebufferHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		ComPtr<IDXGISwapChain1> swapChain1;
		CHECK_D3D_RESULT (
			dxgiFactory->CreateSwapChainForHwnd (
				m_directCmdQueue.Get (),
				hWindow,
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain1
			)
		);

		ComPtr<IDXGISwapChain2> swapChain2;
		CHECK_D3D_RESULT (
			swapChain1.As (&swapChain2)
		);
		swapChain2->SetMaximumFrameLatency (NUM_BUFFERED_FRAMES);

		// Acquire handle to frame latency waitable object.
		m_frameLatencyWaitableObject = swapChain2->GetFrameLatencyWaitableObject ();

		// Assign interface object to m_swapChain so it persists past current scope.
		CHECK_D3D_RESULT (
			swapChain1.As (&m_swapChain)
		);
	}

	// Set the current frame index to correspond with the current back buffer index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex ();
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createHardwareDevice (
	IDXGIFactory4 * dxgiFactory
) {
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	uint adapterIndex (0);
	while (DXGI_ERROR_NOT_FOUND != 
		dxgiFactory->EnumAdapters1 (adapterIndex, &hardwareAdapter))
	{
		DXGI_ADAPTER_DESC1 desc;
		hardwareAdapter->GetDesc1 (&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED (
				::D3D12CreateDevice (hardwareAdapter.Get (),
				featureLevel,
				IID_PPV_ARGS(&m_device)))) 
		{
			SET_D3D_DEBUG_NAME (m_device);
			break;
		}

		++adapterIndex;
	}

	if (!hardwareAdapter) {
		ForceBreak ("No hardware adapter found that supports D3D12.");
	}

	// Display hardware adapter name.
	DXGI_ADAPTER_DESC1 adapterDesc = {};
	hardwareAdapter->GetDesc1 (&adapterDesc);
	LOG_INFO ("Adapter: %ls", adapterDesc.Description);
}

//---------------------------------------------------------------------------------------
// Helper function for acquiring the first available hardware adapter that supports
// the given feature level. If no such adapter can be found, *ppAdapter will be set to
// nullptr.
void D3D12Renderer::getHardwareAdapter (
	IDXGIFactory2 * pFactory,
	D3D_FEATURE_LEVEL featureLevel,
	IDXGIAdapter1 ** ppAdapter
) {
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	uint adapterIndex (0);
	while (DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1 (adapterIndex, &adapter)) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1 (&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED (D3D12CreateDevice (adapter.Get (),
			featureLevel, _uuidof (ID3D12Device), nullptr))) {
			break;
		}

		++adapterIndex;
	}

	// Return first hardware adapter found, that supports the specified D3D feature set.
	// Call Detach() so that the ComPtr does not destroy the interface object when exiting
	// the current scope.
	*ppAdapter = adapter.Detach ();
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createDepthStencilBuffer (
	uint bufferWidth,
	uint bufferHeight
) {
	// Create a depth stencil descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDescriptor = {};
	dsvHeapDescriptor.NumDescriptors = 1;
	dsvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CHECK_D3D_RESULT (
		m_device->CreateDescriptorHeap (&dsvHeapDescriptor, IID_PPV_ARGS (&m_dsvDescHeap))
	);
	SET_D3D_DEBUG_NAME (m_dsvDescHeap);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	// Create Depth-Stencil Buffer resource.
	CHECK_D3D_RESULT (
		m_device->CreateCommittedResource (
			&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D (DXGI_FORMAT_D32_FLOAT, 
				bufferWidth, bufferHeight,
				1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS (&m_depthStencilBuffer.resource)
		)
	);
	SET_D3D_DEBUG_NAME (m_depthStencilBuffer.resource);

	m_device->CreateDepthStencilView (
		m_depthStencilBuffer.resource,
		&depthStencilDesc,
		m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart ()
	);

}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createRenderTargetViews ()
{
	//-- Describe and create the RTV Descriptor Heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDescriptor = {};

		// The RTV Descriptor Heap will hold a RTV Descriptor for each swap chain buffer.
		rtvDescHeapDescriptor.NumDescriptors = NUM_BUFFERED_FRAMES;
		rtvDescHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvDescHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_D3D_RESULT (
			m_device->CreateDescriptorHeap (&rtvDescHeapDescriptor, IID_PPV_ARGS (&m_rtvDescHeap))
		);
	}

	//-- Create a RTV for each swap-chain buffer.
	{
		// Specify a RTV with sRGB format to support gamma correction.
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		// Get increment size between descriptors in RTV Descriptor Heap.
		uint handleIncrementSize = 
			m_device->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Create a render target view for each frame.
		for (uint i (0); i < NUM_BUFFERED_FRAMES; ++i) {
			// Acquire buffer resource from swap chain.
			CHECK_D3D_RESULT (
				m_swapChain->GetBuffer (i, IID_PPV_ARGS (&m_renderTarget[i].resource))
			);

			// Compute handle location within RTV descriptor heap.
			const auto heapStart = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart ();
			m_renderTarget[i].rtvHandle.ptr = heapStart.ptr + i * handleIncrementSize;

			// Create RTV and store its descriptor at the heap location reference by
			// the descriptor handle.
			m_device->CreateRenderTargetView (
				m_renderTarget[i].resource, &rtvDesc, m_renderTarget[i].rtvHandle
			);
			SET_D3D_DEBUG_NAME (m_renderTarget[i].resource);
		}
	}

}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createFenceObjects ()
{
	// Create synchronization primitives.
	for (int i (0); i < NUM_BUFFERED_FRAMES; ++i) {
		const uint64 initialFenceValue (0);
		CHECK_D3D_RESULT (
			m_device->CreateFence (initialFenceValue, D3D12_FENCE_FLAG_NONE, 
				IID_PPV_ARGS (&m_frameFence[i]))
		);
		m_fenceValue[i] = initialFenceValue;

		// Create an event handle to use for frame synchronization.
		// Initially event will be non-signaled.
		m_frameFenceEvent[i] = CreateEvent (nullptr, false, false, nullptr);
		if (m_frameFenceEvent[i] == nullptr) {
			CHECK_D3D_RESULT (
				HRESULT_FROM_WIN32 (GetLastError ())
			);
		}
	}

	// Initialize incremental fence value.
	m_currentFenceValue = 1;
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createDrawCommandLists ()
{
	// Create a draw command list and backing command allocator for each buffered frame.
	for (int i (0); i < NUM_BUFFERED_FRAMES; ++i) {
		createCommandList (m_drawCmdList[i], m_directCmdAllocator[i]);

		// Close command list for now.  Will reset before recording new drawing commands.
		m_drawCmdList[i]->Close ();
	}
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createCommandList (
	ComPtr<ID3D12GraphicsCommandList> & commandList,
	ComPtr<ID3D12CommandAllocator> & cmdAllocator
) {
	// Create command allocator.
	CHECK_D3D_RESULT (
		m_device->CreateCommandAllocator (
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS (&cmdAllocator)
		)
	);

	// Create command list with associated allocator.
	CHECK_D3D_RESULT (
		m_device->CreateCommandList (
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			cmdAllocator.Get (),
			nullptr,
			IID_PPV_ARGS (&commandList)
		)
	);
}

//---------------------------------------------------------------------------------------
#include "Core/AssetLoader.hpp"

void D3D12Renderer::loadAssets (
	ID3D12GraphicsCommandList * uploadCmdList,
	ComPtr<ID3D12Resource> & uploadBuffer
) {
	// Create an empty root signature.
	createRootSignature ();

	// Load shader bytecode.
	AssetLoader::load ("VertexShader.cso", &m_shaderGroup.vertexShader);
	AssetLoader::load ("PixelShader.cso", &m_shaderGroup.pixelShader);


	createPipelineState (m_shaderGroup);

	uploadVertexDataToDefaultHeap (uploadCmdList, uploadBuffer);
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::createPipelineState (
	const ShaderGroup & shaderGroup
) {
	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescriptor[2];

	// Positions
	inputElementDescriptor[0].SemanticName = "POSITION";
	inputElementDescriptor[0].SemanticIndex = 0;
	inputElementDescriptor[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescriptor[0].InputSlot = 0;
	inputElementDescriptor[0].AlignedByteOffset = 0;
	inputElementDescriptor[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescriptor[0].InstanceDataStepRate = 0;

	// Colors
	inputElementDescriptor[1].SemanticName = "COLOR";
	inputElementDescriptor[1].SemanticIndex = 0;
	inputElementDescriptor[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescriptor[1].InputSlot = 0;
	inputElementDescriptor[1].AlignedByteOffset = sizeof (float) * 3;
	inputElementDescriptor[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescriptor[1].InstanceDataStepRate = 0;

	// Describe the rasterizer state
	CD3DX12_RASTERIZER_DESC rasterizerState (D3D12_DEFAULT);
	rasterizerState.FrontCounterClockwise = TRUE;
	rasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerState.DepthClipEnable = FALSE;

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = {inputElementDescriptor, _countof (inputElementDescriptor)};
	psoDesc.pRootSignature = m_rootSignature.Get ();
	psoDesc.VS = shaderGroup.vertexShader->byteCode;
	psoDesc.PS = shaderGroup.pixelShader->byteCode;
	psoDesc.RasterizerState = rasterizerState;
	psoDesc.BlendState = CD3DX12_BLEND_DESC (D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC (D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.SampleDesc.Count = 1;

	// Create the Pipeline State Object.
	CHECK_D3D_RESULT (
		m_device->CreateGraphicsPipelineState ( &psoDesc, IID_PPV_ARGS (&m_pipelineState))
	);
	SET_D3D_DEBUG_NAME (m_pipelineState);
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::uploadVertexDataToDefaultHeap (
	ID3D12GraphicsCommandList * uploadCmdList,
	ComPtr<ID3D12Resource> & uploadBuffer
) {
	const float aspectRatio = static_cast<float>(m_framebufferWidth) / m_framebufferHeight;

	// Define vertices for a square.
	const Vertex vertices[] =
	{
		// Positions                          Colors
		{{0.25f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.25f, -0.25f * aspectRatio, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{-0.25f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}}
	};

	const ushort indices[] = {2,1,0, 2,0,3};
	m_indexCount = _countof (indices);

	const int uploadBufferSize = sizeof (vertices) + sizeof (indices);
	const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD);
	const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (uploadBufferSize);

	// Create upload buffer.
	m_device->CreateCommittedResource (
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS (&uploadBuffer)
	);

	// Allocate vertex and index buffers within the default heap
	{
		const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT);

		const auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (sizeof (vertices));
		m_device->CreateCommittedResource (
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS (&m_vertexBuffer)
		);
		SET_D3D_DEBUG_NAME (m_vertexBuffer);

		const auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (sizeof (indices));
		m_device->CreateCommittedResource (
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS (&m_indexBuffer)
		);
		SET_D3D_DEBUG_NAME (m_indexBuffer);
	}

	// Create buffer views
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress ();
	m_vertexBufferView.SizeInBytes = sizeof (vertices);
	m_vertexBufferView.StrideInBytes = sizeof (Vertex);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress ();
	m_indexBufferView.SizeInBytes = sizeof (indices);
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// Copy data in CPU memory to upload buffer.
	{
		void * p;
		// Set read range to zero, since we are only going to upload data to upload buffer
		// rather than read data from it.
		D3D12_RANGE readRange = {0, 0};
		uploadBuffer->Map (0, &readRange, &p);
		::memcpy (p, vertices, sizeof (vertices));
		::memcpy (static_cast<byte *>(p) + sizeof (vertices), indices, sizeof (indices));

		// Finished uploading data to uploadBuffer, so unmap it.
		D3D12_RANGE writtenRange = {0, sizeof (vertices) + sizeof (indices)};
		uploadBuffer->Unmap (0, &writtenRange);
	}

	// Copy data from upload buffer on CPU into the index/vertex buffer on 
	// the GPU.
	{
		uint64 dstOffset = 0;
		uint64 srcOffset = 0;
		uploadCmdList->CopyBufferRegion (
			m_vertexBuffer.Get (), dstOffset, uploadBuffer.Get (), srcOffset, sizeof (vertices)
		);

		dstOffset = 0;
		srcOffset = sizeof (vertices);
		uploadCmdList->CopyBufferRegion (
			m_indexBuffer.Get (), dstOffset, uploadBuffer.Get (), srcOffset, sizeof (indices)
		);
	}

	// Batch resource barriers marking state transitions.
	{
		const CD3DX12_RESOURCE_BARRIER barriers[2] = {

			CD3DX12_RESOURCE_BARRIER::Transition (
				m_vertexBuffer.Get (),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			),

			CD3DX12_RESOURCE_BARRIER::Transition (
				m_indexBuffer.Get (),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_INDEX_BUFFER
			)
		};

		uploadCmdList->ResourceBarrier (2, barriers);
	}
}


//---------------------------------------------------------------------------------------
void D3D12Renderer::createRootSignature ()
{
	//TODO (Dustin) - Creating bare root signature now.  Add actual params to root signature later.

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init (
		0, nullptr, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	CHECK_D3D_RESULT (
		D3D12SerializeRootSignature (
			&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signature,
			&error
		)
	);

	CHECK_D3D_RESULT (
		m_device->CreateRootSignature (
			0,
			signature->GetBufferPointer (),
			signature->GetBufferSize (),
			IID_PPV_ARGS (&m_rootSignature)
		)
	);
}

//---------------------------------------------------------------------------------------
internal void waitForGpuFence (
	ID3D12Fence * fence,
	uint64 completionValue,
	HANDLE fenceEvent
)
{
	if (fence->GetCompletedValue () < completionValue) {
		CHECK_D3D_RESULT (
			fence->SetEventOnCompletion (completionValue, fenceEvent)
		);
		// fenceEvent will be signaled once GPU updates fence object to completionValue.
		::WaitForSingleObject (fenceEvent, INFINITE);
	}
}

//---------------------------------------------------------------------------------------
void D3D12Renderer::waitForGpuCompletion (
	ID3D12CommandQueue * commandQueue
)
{
	CHECK_D3D_RESULT (
		commandQueue->Signal (m_frameFence[m_frameIndex].Get (), m_currentFenceValue)
	);
	m_fenceValue[m_frameIndex] = m_currentFenceValue;
	++m_currentFenceValue;

	::waitForGpuFence (m_frameFence[m_frameIndex].Get (),
		m_fenceValue[m_frameIndex], m_frameFenceEvent[m_frameIndex]);
}

