#include <windows.h>

#include <xnamath.h>
#include <assert.h>
#include <dinput.h>

#include "D3DCompat.h"
#include "Terrain.h"

#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }



HINSTANCE   g_hInst = NULL;
HWND        g_hWnd = NULL;
ID3DDevice*   g_d3dDevice = NULL;
ID3DDeviceContext* g_context = NULL;
ID3DRenderTargetView* g_rtv = NULL;
ID3DDepthStencilView* g_dsv = NULL;
IDXGISwapChain* g_swapChain = NULL;


ID3DVertexShader* g_vs = NULL;
ID3DPixelShader* g_fs = NULL;
ID3DGeometryShader* g_gs = NULL;

ID3DBuffer* g_cb = NULL;
ID3DRasterizerState* g_rs = NULL;
ID3DDepthStencilState* g_ds = NULL;

LPDIRECTINPUT8 g_Input;
LPDIRECTINPUTDEVICE8  g_KB; 


RECT g_windowRect;
ID3D10Blob* g_vsBlob = NULL, *g_fsBlob = NULL, *g_gsBlob = NULL;



enum VertexType
{
	VT_POSITION,
	VT_BONE_WEIGHTS,
	VT_BONE_INDICES,
	VT_NORMAL,
	VT_DIFFUSE_COLOR,
	VT_TEXTURE_COORDINATE,
	VT_NTYPES
};

const char* gVertexNames[VT_NTYPES] = 
{
	"POSITION",
	"BLENDWEIGHT",
	"BLENDINDICES",
	"NORMAL",
	"COLOR",
	"TEXCOORD"
};


ID3DShaderResourceView* GetSRV(const char* name)
{
	ID3DResource* texture;
	HRESULT result = D3DXCreateTextureFromFile(g_d3dDevice, name, NULL, NULL, &texture, NULL);
	assert(result == S_OK);

	ID3DShaderResourceView* srv;
	result = g_d3dDevice->CreateShaderResourceView(texture, NULL, &srv);
	assert(result == S_OK);
	return srv;
}

class Model
{
public:
	Model(const char* filename) : mInputLayout(NULL)
	{
		FILE* f = fopen(filename, "rb");

		int numGroups, numElements, numVertices;
		fread(&numGroups, 4, 1, f);
		fread(&numElements, 4, 1, f);

		const int MAX_INPUT_ELEMENTS = 16;
		assert(numElements <= MAX_INPUT_ELEMENTS);
		D3D_INPUT_ELEMENT_DESC layout[MAX_INPUT_ELEMENTS];

		int vertexSize = 0;

		for(int i = 0; i < numElements; i++)
		{
			VertexType vertexType;
			DXGI_FORMAT format;
			byte usageIndex, byteOffset, sizeInBytes;
			fread(&vertexType, sizeof(vertexType), 1, f);
			fread(&format, sizeof(format), 1, f);
			fread(&usageIndex, sizeof(usageIndex), 1, f);
			fread(&sizeInBytes, sizeof(sizeInBytes), 1, f);
			fread(&byteOffset, sizeof(byteOffset), 1, f);
			
			layout[i].SemanticName = gVertexNames[vertexType];	
			layout[i].SemanticIndex = usageIndex;
			layout[i].Format = format;
			layout[i].InputSlot = 0;
			layout[i].AlignedByteOffset = byteOffset;
			layout[i].InputSlotClass = D3D_INPUT_PER_VERTEX_DATA;
			layout[i].InstanceDataStepRate = 0;

			vertexSize += sizeInBytes;
		}

		mVBStride = vertexSize;

		HRESULT hr;
		//HRESULT hr = g_d3dDevice->CreateInputLayout(layout, numElements, g_vsBlob->GetBufferPointer(), g_vsBlob->GetBufferSize(), &mInputLayout);
		
		struct AJMVertex
		{
			float x, y, z, nX, nY, nZ;
		};

		fread(&numVertices, 4, 1, f);


		char* vertices = new char[numVertices * vertexSize];
		AJMVertex* ajmVertices = (AJMVertex*)vertices;
		fread(vertices, vertexSize, numVertices, f);

		D3D_BUFFER_DESC vbDesc = CREATE_D3D_BUFFER_DESC(vertexSize * numVertices, D3D_USAGE_IMMUTABLE, D3D_BIND_VERTEX_BUFFER, 0, 0, 0);
		D3D_SUBRESOURCE_DATA vbInitData = { vertices, 0, 0};

		hr = g_d3dDevice->CreateBuffer(&vbDesc, &vbInitData, &mVB);
		delete[] vertices;

		assert(hr == S_OK);

		UINT stride = vertexSize;
		UINT offset = 0;
		
		g_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		int indexSize;
		fread(&indexSize, sizeof(indexSize), 1, f);
		fread(&mNumIndices, sizeof(mNumIndices), 1, f);

		int bytesPerIndex = indexSize / 8;
		char* indices = new char[bytesPerIndex * mNumIndices];

		fread(indices, bytesPerIndex, mNumIndices, f);

		D3D_BUFFER_DESC ibDesc = CREATE_D3D_BUFFER_DESC(bytesPerIndex * mNumIndices, D3D_USAGE_IMMUTABLE, D3D_BIND_INDEX_BUFFER, 0, 0, 0);
		D3D_SUBRESOURCE_DATA ibInitData = { indices, 0, 0 };

		hr = g_d3dDevice->CreateBuffer(&ibDesc, &ibInitData, &mIB);

		mIBFormat = (bytesPerIndex == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

		int numMaterials;
		fread(&numMaterials, sizeof(numMaterials), 1, f);
		assert(numMaterials == 1);

		for(int i = 0; i < numMaterials; i++)
		{
			int numMaps;
			fread(&numMaps, sizeof(numMaps), 1, f);
			assert(numMaterials == 1);

			for(int j = 0; j < numMaps; j++)
			{
				int stringLength;
				fread(&stringLength, sizeof(stringLength), 1, f);

				const int MAX_FILENAME = 128;
				char fileName[MAX_FILENAME];
				ZeroMemory(fileName, MAX_FILENAME);
				assert(stringLength < MAX_FILENAME);
				fread(fileName, 1, stringLength, f);

				for(int c = 0; c < stringLength; c++)
				{
					if(fileName[c] == '\\')
						fileName[c] = '/';
				}

				mSRV = GetSRV(fileName);
			}
		}

		fclose(f);

		delete[] indices;
	}

	~Model()
	{
		SAFE_RELEASE(mVB);
		SAFE_RELEASE(mIB);
		SAFE_RELEASE(mInputLayout);
	}

	void Update()
	{

	}

	void Draw()
	{
		UINT offset = 0;

		//g_context->IASetInputLayout(mInputLayout);
		g_context->IASetVertexBuffers(0, 1, &mVB, &mVBStride, &offset);
		g_context->IASetIndexBuffer(mIB, mIBFormat, 0);
		g_context->PSSetShaderResources(0, 1, &mSRV);
		g_context->VSSetShader(VS_SET_SHADER_ARGS(g_vs, NULL, 0));
		g_context->PSSetShader(PS_SET_SHADER_ARGS(g_fs, NULL, 0));
		//g_context->GSSetShader(GS_SET_SHADER_ARGS(g_gs, NULL, 0));

		//g_context->DrawIndexed(mNumIndices, 0, 0);
	}

private:
	ID3DBuffer* mVB;
	ID3DBuffer* mIB;
	ID3DShaderResourceView* mSRV;
	ID3DInputLayout* mInputLayout;
	UINT mNumIndices;
	UINT mVBStride;
	DXGI_FORMAT mIBFormat;
};

Model* model = NULL;
Terrain* terrain = NULL;

HRESULT RecreateViews()
{
	ID3DTexture2D* pBackBuffer;
	HRESULT hr = g_swapChain->GetBuffer(0, __uuidof(ID3DTexture2D), (void**)&pBackBuffer);

	if(FAILED(hr))
		return hr;

	hr = g_d3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_rtv);
	SAFE_RELEASE(pBackBuffer);

	if(FAILED(hr))
		return hr;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	g_swapChain->GetDesc(&swapChainDesc);
	
	ID3DTexture2D* depthTexture = NULL;
	D3D_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = swapChainDesc.BufferDesc.Width;
	depthDesc.Height = swapChainDesc.BufferDesc.Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 4;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	hr = g_d3dDevice->CreateTexture2D(&depthDesc, NULL, &depthTexture);
	
	if(FAILED(hr))
		return hr;

	D3D_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthDesc.Format;
#if !defined(D3D10_OVERRIDE)
	dsvDesc.Flags = 0;
#endif
	dsvDesc.ViewDimension = D3D_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = g_d3dDevice->CreateDepthStencilView(depthTexture, NULL, &g_dsv);

	SAFE_RELEASE(depthTexture);

	if(FAILED(hr))
		return hr;

	g_context->OMSetRenderTargets(1, &g_rtv, g_dsv);

	D3D_VIEWPORT viewport = { 0, 0, swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height, 0, 1 };
	g_context->RSSetViewports(1, &viewport);

	return S_OK;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

		case WM_SIZE:
			//Sleep(1000);
			{

			
			GetClientRect(g_hWnd, &g_windowRect);
			SAFE_RELEASE(g_rtv);
			SAFE_RELEASE(g_dsv);
			g_swapChain->ResizeBuffers(1, g_windowRect.right - g_windowRect.left, g_windowRect.bottom - g_windowRect.top, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
			RecreateViews();
			break;
			}

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDC_ARROW );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "SettlersWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDC_ARROW );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1600, 1000 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( "SettlersWindowClass", "Settlers 11",
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    

    return S_OK;
}



HRESULT InitDevice()
{
	OutputDebugString("HELLO!\n");

	HRESULT hr = S_OK;
	RECT rect;
	GetClientRect(g_hWnd, &rect);

	UINT width = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;

	//UINT createDeviceFlags = D3D_CREATE_DEVICE_DEBUG;
	UINT createDeviceFlags = 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = g_hWnd;
	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;

	IDXGIFactory* factory;
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

	uint nAdapter = 0;
	IDXGIAdapter* adapter = NULL, *selectedAdapter = NULL;
	D3D10_DRIVER_TYPE driverType = D3D10_DRIVER_TYPE_HARDWARE;

	while(factory->EnumAdapters(nAdapter, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		if(adapter)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			if(SUCCEEDED(adapter->GetDesc(&adapterDesc)))
			{
				bool isPerfHUD = wcscmp(adapterDesc.Description, L"NVIDIA PerfHUD") == 0;

				if(nAdapter == 0 || isPerfHUD)
				{
					selectedAdapter = adapter;
				}

				if(isPerfHUD)
					driverType = D3D10_DRIVER_TYPE_REFERENCE;
			}
		}

		nAdapter++;
	}

#if defined(D3D10_OVERRIDE)
	hr = D3D10CreateDeviceAndSwapChain(selectedAdapter, driverType, NULL, createDeviceFlags, D3D10_SDK_VERSION, &swapChainDesc, &g_swapChain, &g_d3dDevice);
	g_context = g_d3dDevice;
#else
	D3D_FEATURE_LEVEL featureLevelUsed;
	hr = D3D11CreateDeviceAndSwapChain(selectedAdapter, driverType, NULL, createDeviceFlags, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &g_swapChain, &g_d3dDevice, &featureLevelUsed, &g_context);
#endif
	


	assert(hr == S_OK);
	assert(g_swapChain != NULL);

	char str[256];
	sprintf_s(str, 256, "Device Creation: Error Code = %d\n", hr);
	OutputDebugString(str);

	if(FAILED(hr))
		return hr;

	RecreateViews();

	hr = LoadShader("Z:/S2C/Development/data/shaders/standard.vs", "vs_4_0", &g_vsBlob);
	if(FAILED(hr))
		return hr;
	hr = LoadShader("Z:/S2C/Development/data/shaders/standard.fs", "ps_4_0", &g_fsBlob);
	if(FAILED(hr))
		return hr;
	hr = LoadShader("Z:/S2C/Development/data/shaders/standard.gs", "gs_4_0", &g_gsBlob);
	if(FAILED(hr))
		return hr;

	hr = g_d3dDevice->CreateVertexShader(CREATE_VERTEX_SHADER_ARGS(g_vsBlob->GetBufferPointer(), g_vsBlob->GetBufferSize(), NULL, &g_vs));

	if(FAILED(hr))
		return hr;

	hr = g_d3dDevice->CreatePixelShader(CREATE_PIXEL_SHADER_ARGS(g_fsBlob->GetBufferPointer(), g_fsBlob->GetBufferSize(), NULL, &g_fs));

	if(FAILED(hr))
		return hr;

	hr = g_d3dDevice->CreateGeometryShader(CREATE_GEOMETRY_SHADER_ARGS(g_gsBlob->GetBufferPointer(), g_gsBlob->GetBufferSize(), NULL, &g_gs));

	if(FAILED(hr))
		return hr;


	model = new Model("Z:/S2C/Development/data/animals/elk.GR2.smd");
	terrain = new Terrain(g_d3dDevice, g_context, "Z:/S2C/Development/data/maps/freeGameMaps/Southshore11.s11map");
	



	D3D_BUFFER_DESC cbDesc = CREATE_D3D_BUFFER_DESC(sizeof(XMMATRIX) + 16, D3D_USAGE_DYNAMIC, D3D_BIND_CONSTANT_BUFFER, D3D_CPU_ACCESS_WRITE, 0, 0);
	hr = g_d3dDevice->CreateBuffer(&cbDesc, NULL, &g_cb);

	if(FAILED(hr))
		return hr;

	g_context->VSSetConstantBuffers(0, 1, &g_cb);
	g_context->PSSetConstantBuffers(0, 1, &g_cb);
	//g_context->GSSetConstantBuffers(0, 1, &g_cb);


	D3D_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D_FILL_SOLID;
	rasterizerDesc.CullMode = D3D_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = true;
	rasterizerDesc.AntialiasedLineEnable = false;

	g_d3dDevice->CreateRasterizerState(&rasterizerDesc, &g_rs);
	g_context->RSSetState(g_rs);

	D3D_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = false;

	hr = g_d3dDevice->CreateDepthStencilState(&depthStencilDesc, &g_ds);
	
	if(FAILED(hr))
		return hr;

	g_context->OMSetDepthStencilState(g_ds, 0);
	

	return hr;
}

void CleanupDevice()
{
	if(g_context)
	{
		g_context->ClearState();
		SAFE_RELEASE(g_context);
	}

	SAFE_RELEASE(g_vs);
	SAFE_RELEASE(g_fs);
	SAFE_RELEASE(g_gs);
	SAFE_RELEASE(g_rs);
	SAFE_RELEASE(g_cb);
	delete model;
	SAFE_RELEASE(g_rtv);
	SAFE_RELEASE(g_dsv);
	SAFE_RELEASE(g_swapChain);

#if !defined(D3D10_OVERRIDE)
	SAFE_RELEASE(g_d3dDevice);
#endif
}

float f = 3.1415;

float m = 1;

XMVECTOR cameraPosition = XMVectorSet(0, 0, 0, 1);
XMVECTOR cameraBaseForward = XMVectorSet(0, 0, 1, 0);
XMVECTOR cameraBaseUp = XMVectorSet(0, 1, 0, 0);
XMVECTOR cameraBaseRight = XMVectorSet(1, 0, 0, 0);

XMVECTOR rotatedForward;
XMVECTOR rotatedUp;
XMVECTOR rotatedRight;

float yaw = 0, pitch = 0, roll = 0;

void Render()
{
#define KEYDOWN(name, key) (name[key] & 0x80) 
	

	char buffer[256];
	ZeroMemory(buffer, sizeof(buffer));
	HRESULT hr = g_KB->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);

	if(hr != S_OK)
	{
		hr = g_KB->Acquire();
	}

	float strafeSpeed = 1.0f;
	float pitchSpeed = 0.01f;

	if(KEYDOWN(buffer, DIK_D))
		cameraPosition = XMVectorAdd(cameraPosition, XMVectorScale(rotatedRight, strafeSpeed));
	if(KEYDOWN(buffer, DIK_A))
		cameraPosition = XMVectorSubtract(cameraPosition, XMVectorScale(rotatedRight, strafeSpeed));
	if(KEYDOWN(buffer, DIK_Q))
		cameraPosition = XMVectorAdd(cameraPosition, XMVectorScale(rotatedUp, strafeSpeed));
	if(KEYDOWN(buffer, DIK_Z))
		cameraPosition = XMVectorSubtract(cameraPosition, XMVectorScale(rotatedUp, strafeSpeed));
	if(KEYDOWN(buffer, DIK_W))
		cameraPosition = XMVectorAdd(cameraPosition, XMVectorScale(rotatedForward, strafeSpeed));
	if(KEYDOWN(buffer, DIK_S))
		cameraPosition = XMVectorSubtract(cameraPosition, XMVectorScale(rotatedForward, strafeSpeed));
	if(KEYDOWN(buffer, DIK_UP))
		pitch += pitchSpeed;
	if(KEYDOWN(buffer, DIK_DOWN))
		pitch -= pitchSpeed;
	if(KEYDOWN(buffer, DIK_LEFT))
		yaw -= pitchSpeed;
	if(KEYDOWN(buffer, DIK_RIGHT))
		yaw += pitchSpeed;

	int windowWidth = g_windowRect.right - g_windowRect.left;
	int windowHeight = g_windowRect.bottom - g_windowRect.top;

	/*f += 0.25f;
	m -= 0.005f;

	if(m < 0.15f)
		m = 0.15f;

	XMMATRIX world = XMMatrixRotationRollPitchYaw(0, XMConvertToRadians(180), 0);
	XMMATRIX view = XMMatrixLookAtRH(XMVectorSet(-100 + f, 500*m, -300 * 1.2f, 1), XMVectorSet(-100 + f, 0, -300 * 1.33333f, 1), XMVectorSet(0, 1, 0, 0));
	XMMATRIX proj = XMMatrixPerspectiveFovRH(XMConvertToRadians(60), (float)windowWidth/(float)windowHeight, 2.0f, 1500.0f);*/

	f += 0.005f;

	float radius = 100;
	float height = 0;

	//XMVECTOR viewPoint = XMVectorSet(175, 0, 175, 1);
	
	XMVECTOR lightPosition = XMVectorSet(sin(f) * 400, 300, cos(f) * 400, 1) + XMVectorSet(500, 0, 500, 1);

	//float radius = 20;
	//float height = 20;
	//
	//XMVECTOR viewPoint = XMVectorSet(0, 0, 0, 1);
	//XMVECTOR cameraPosition = XMVectorSet(sin(f) * radius, height, cos(f) * radius, 1) + viewPoint;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	rotatedForward = XMVector3TransformNormal(cameraBaseForward, rotationMatrix);
	rotatedUp = XMVector3TransformNormal(cameraBaseUp, rotationMatrix);
	rotatedRight = XMVector3TransformNormal(cameraBaseRight, rotationMatrix);

	XMMATRIX world = XMMatrixRotationRollPitchYaw(0, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(cameraPosition, XMVectorAdd(cameraPosition, rotatedForward), rotatedUp);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(50), (float)windowWidth/(float)windowHeight, 1.0f, 5000.0f);

	XMMATRIX viewProj = XMMatrixMultiply(XMMatrixMultiply(world, view), proj);

	terrain->Update(*g_context, cameraPosition, world, view, proj);

#if defined(D3D10_OVERRIDE)
	void* mappedResource;
	hr = g_cb->Map(D3D_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource, &viewProj, sizeof(XMMATRIX));
	char* lightPosPtr = (char*)mappedResource + sizeof(XMMATRIX);
	float* lightPos = (float*)lightPosPtr;
	lightPos[0] = XMVectorGetX(lightPosition);
	lightPos[1] = XMVectorGetY(lightPosition);
	lightPos[2] = XMVectorGetZ(lightPosition);
	g_cb->Unmap();
#else
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = g_context->Map(g_cb, 0, D3D_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &viewProj, sizeof(XMMATRIX));
	g_context->Unmap(g_cb, 0);
#endif
	
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_context->ClearRenderTargetView(g_rtv, clearColor);
	g_context->ClearDepthStencilView(g_dsv, D3D_CLEAR_DEPTH, 1, 0);

	model->Draw();
	terrain->Draw(*g_context, viewProj);

	g_swapChain->Present(0, 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if(FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if(FAILED(InitDevice()))
	{
		CleanupDevice();
		return -1;
	}

	ShowWindow( g_hWnd, nCmdShow );

	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_Input, NULL);

	if(hr == S_OK)
	{
		hr = g_Input->CreateDevice(GUID_SysKeyboard, &g_KB, NULL);
		assert(hr == S_OK);
		hr = g_KB->SetDataFormat(&c_dfDIKeyboard);
		hr = g_KB->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

		if(g_KB)
			g_KB->Acquire();
	}

	MSG msg = {0};

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	CleanupDevice();

	return 0;
}