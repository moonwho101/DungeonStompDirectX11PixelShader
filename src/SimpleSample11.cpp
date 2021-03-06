//--------------------------------------------------------------------------------------
// File: SimpleSample11.cpp
//
// Starting point for new Direct3D 11 samples.  For a more basic starting point, 
// use the EmptyProject11 sample instead.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

#include "LoadWorld.hpp"
#include "world.hpp"
#include "GlobalSettings.hpp"
#include "GameLogic.hpp"
#include <time.h>
#include <vector>
#include <DirectXMath.h>
#include "MathHelper.h"
#include "Missle.hpp"
#include "d3dutil.h"

int totalcount = 0;
extern int countdisplay;
extern int number_of_tex_aliases;

typedef struct Vertex {
    D3DXVECTOR3 position; // The position
    FLOAT nx, ny, nz;   // Normals
    FLOAT tu, tv;   // The texture coordinates
};

ID3D11ShaderResourceView* textures[400];
ID3D11ShaderResourceView* save_out_srv = NULL;

#define MAX_DX11_VERT 50000

Vertex v[MAX_DX11_VERT];

void SetDungeonText();
void DisplayHud();
void SetDiceTexture(bool showroll);
void DrawScene(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);
void ProcessLights11();
BOOL LoadRRTextures11(char* filename, ID3D11Device* pd3dDevice);

typedef struct Light
{
    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only
};

struct PassConstants
{
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light Lights[MaxLights];
};

Light mMainPassCB[MaxLights];

//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper* g_pTxtHelper = NULL;
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
CDXUTDialog                 g_HUDStats;                  // dialog for standard controls

// Direct3D 9 resources
extern ID3DXFont* g_pFont9;
extern ID3DXSprite* g_pSprite9;

// Direct3D 11 resources
ID3D11VertexShader* g_pVertexShader11 = NULL;
ID3D11PixelShader* g_pPixelShader11 = NULL;
ID3D11VertexShader* g_pVertexShaderTorch11 = NULL;
ID3D11PixelShader* g_pPixelShaderTorch11 = NULL;
ID3D11InputLayout* g_pLayout11 = NULL;
ID3D11SamplerState* g_pSamLinear = NULL;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct CB_VS_PER_OBJECT
{
    D3DXMATRIX  m_mWorldViewProjection;
    D3DXMATRIX  m_mWorld;
    D3DXVECTOR4 m_MaterialAmbientColor;
    D3DXVECTOR4 m_MaterialDiffuseColor;
};

struct CB_VS_PER_FRAME
{
    D3DXVECTOR3 m_vLightDir;
    float       m_fTime;
    D3DXVECTOR4 m_LightDiffuse;
};
#pragma pack(pop)

ID3D11Buffer* g_pcbVSPerObject11 = NULL;
ID3D11Buffer* g_pcbVSPerFrame11 = NULL;
ID3D11Buffer* g_pcbVSLight11 = NULL;
ID3D11Buffer* g_pcbBuffer = NULL;
ID3D11Buffer* g_pcbCaptionBuffer = NULL;

HRESULT FrameMove(double fTime, FLOAT fTimeKey);
void CheckAngle();
void PlayerJump(const FLOAT& fTimeKey, D3DVECTOR& result);
void PlayerAnimation();
void StrifePlayer(float& step_left_angy, float& r, const FLOAT& fTimeKey, bool addVel);
void MovePlayer(float& r, const FLOAT& fTimeKey, bool& addVel);
void FindDoors(const FLOAT& fTimeKey);
void GameTimers(const FLOAT& fTimeKey);
VOID UpdateControls();

int main2();
extern char gActionMessage[2048];

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TEST                4

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
    void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);

extern bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
    bool bWindowed, void* pUserContext);
extern HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice,
    const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
extern HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
    void* pUserContext);
extern void CALLBACK OnD3D9FrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
    void* pUserContext);
extern void CALLBACK OnD3D9LostDevice(void* pUserContext);
extern void CALLBACK OnD3D9DestroyDevice(void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo* AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo* DeviceInfo,
    DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
    void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
    const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
    float fElapsedTime, void* pUserContext);

void InitApp();
void RenderText();
HRESULT CreateDInput(HWND hWnd);


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc(MsgProc);
    //DXUTSetCallbackKeyboard(OnKeyboard);
    DXUTSetCallbackFrameMove(OnFrameMove);
    DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

    DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
    DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice);
    DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice);
    DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice);
    DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice);
    DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender);

    DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
    DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
    DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
    DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
    DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
    DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);

    InitApp();
    DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings(true, true);
    DXUTCreateWindow(L"SimpleSample11");

    srand(time(NULL));

    main2();

    // Only require 10-level hardware, change to D3D_FEATURE_LEVEL_11_0 to require 11-class hardware
    // Switch to D3D_FEATURE_LEVEL_9_x for 10level9 hardware
    DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 640, 480);

    HWND h = DXUTGetHWND();
    CreateDInput(h);
    InitDS();

    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_SettingsDlg.Init(&g_DialogResourceManager);
    g_HUD.Init(&g_DialogResourceManager);

    g_HUDStats.Init(&g_DialogResourceManager);
    g_SampleUI.Init(&g_DialogResourceManager);

    g_HUD.SetCallback(OnGUIEvent);
    int iY = 30;
    int iYo = 26;
    g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22);
    g_HUD.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3);
    g_HUD.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2);

    g_SampleUI.SetCallback(OnGUIEvent); iY = 10;
    g_HUDStats.SetCallback(OnGUIEvent);
    iY = 0;
    iYo = 26;
    g_HUDStats.AddButton(IDC_TEST, L"", 0, 80.0f, 250, 250);

    CDXUTButton* button = g_HUDStats.GetButton(IDC_TEST);
    CDXUTElement* elem = button->GetElement(0);  // ..or perhaps GetElement(0)
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos(5, 5);
    g_pTxtHelper->SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
    //g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
    g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(true));
    g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());
    g_pTxtHelper->End();
}


void RenderText(char* p)
{
    wchar_t wtext[120];
    mbstowcs(wtext, p, strlen(p) + 1);//Plus null

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos(5, 250);
    g_pTxtHelper->SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
    g_pTxtHelper->DrawTextLine(wtext);
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo* AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo* DeviceInfo,
    DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
    void* pUserContext)
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
    V_RETURN(g_SettingsDlg.OnD3D11CreateDevice(pd3dDevice));
    g_pTxtHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15);

    // Read the HLSL file
    WCHAR str[MAX_PATH];
    V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"../Content/Shaders/SimpleSample.hlsl"));

    // Compile the shaders - D3DCOMPILE_ENABLE_STRICTNESS
    DWORD dwShaderFlags = D3DCOMPILE_SKIP_VALIDATION;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    // You should use the lowest possible shader profile for your shader to enable various feature levels. These
    // shaders are simple enough to work well within the lowest possible profile, and will run on all feature levels
    ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN(D3DX11CompileFromFile(str, NULL, NULL, "RenderSceneVS", "vs_4_0_level_9_1", dwShaderFlags, 0, NULL,
        &pVertexShaderBuffer, NULL, NULL));

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN(D3DX11CompileFromFile(str, NULL, NULL, "RenderScenePS", "ps_4_0_level_9_1", dwShaderFlags, 0, NULL,
        &pPixelShaderBuffer, NULL, NULL));

    // Create the shaders
    V_RETURN(pd3dDevice->CreateVertexShader(pVertexShaderBuffer->GetBufferPointer(),
        pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader11));
    DXUT_SetDebugName(g_pVertexShader11, "RenderSceneVS");

    V_RETURN(pd3dDevice->CreatePixelShader(pPixelShaderBuffer->GetBufferPointer(),
        pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader11));
    DXUT_SetDebugName(g_pPixelShader11, "RenderScenePS");

    V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"../Content/Shaders/Torch.hlsl"));

    // You should use the lowest possible shader profile for your shader to enable various feature levels. These
    // shaders are simple enough to work well within the lowest possible profile, and will run on all feature levels
    ID3DBlob* pVertexShaderTorchBuffer = NULL;
    V_RETURN(D3DX11CompileFromFile(str, NULL, NULL, "RenderSceneVS", "vs_4_0_level_9_1", dwShaderFlags, 0, NULL,
        &pVertexShaderTorchBuffer, NULL, NULL));

    ID3DBlob* pPixelShaderTorchBuffer = NULL;
    V_RETURN(D3DX11CompileFromFile(str, NULL, NULL, "RenderScenePS", "ps_4_0_level_9_1", dwShaderFlags, 0, NULL,
        &pPixelShaderTorchBuffer, NULL, NULL));

    // Create the shaders
    V_RETURN(pd3dDevice->CreateVertexShader(pVertexShaderTorchBuffer->GetBufferPointer(),
        pVertexShaderTorchBuffer->GetBufferSize(), NULL, &g_pVertexShaderTorch11));
    DXUT_SetDebugName(g_pVertexShaderTorch11, "RenderTorchVS");

    V_RETURN(pd3dDevice->CreatePixelShader(pPixelShaderTorchBuffer->GetBufferPointer(),
        pPixelShaderTorchBuffer->GetBufferSize(), NULL, &g_pPixelShaderTorch11));
    DXUT_SetDebugName(g_pPixelShaderTorch11, "RenderTorchPS");

    // Create a layout for the object data
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    V_RETURN(pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVertexShaderBuffer->GetBufferPointer(),
        pVertexShaderBuffer->GetBufferSize(), &g_pLayout11));
    DXUT_SetDebugName(g_pLayout11, "Primary");

    // No longer need the shader blobs
    SAFE_RELEASE(pVertexShaderBuffer);
    SAFE_RELEASE(pPixelShaderBuffer);

    // Create state objects
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory(&samDesc, sizeof(samDesc));
    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.MaxAnisotropy = 1;
    samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN(pd3dDevice->CreateSamplerState(&samDesc, &g_pSamLinear));
    DXUT_SetDebugName(g_pSamLinear, "Linear");

    // Create constant buffers
    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(CB_VS_PER_OBJECT);
    V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbVSPerObject11));
    DXUT_SetDebugName(g_pcbVSPerObject11, "CB_VS_PER_OBJECT");

    cbDesc.ByteWidth = sizeof(CB_VS_PER_FRAME);
    V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbVSPerFrame11));
    DXUT_SetDebugName(g_pcbVSPerFrame11, "CB_VS_PER_FRAME");


    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(PassConstants);
    V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbVSLight11));
    DXUT_SetDebugName(g_pcbVSLight11, "CB_VS_LIGHTS");

    // Create other render resources here

    LoadRRTextures11("textures.dat", pd3dDevice);

    // Create t buffers
    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.ByteWidth = sizeof(Vertex) * MAX_DX11_VERT;
    V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbBuffer));
    DXUT_SetDebugName(g_pcbBuffer, "CB_VS_PER_FRAME");

    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.ByteWidth = sizeof(Vertex) * 5000;
    V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, NULL, &g_pcbCaptionBuffer));
    DXUT_SetDebugName(g_pcbCaptionBuffer, "CB_CAPTION_PER_FRAME");

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
    g_Camera.SetViewParams(&vecEye, &vecAt);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
    const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
    V_RETURN(g_SettingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;

    wWidth = pBackBufferSurfaceDesc->Width;
    wHeight = (FLOAT)pBackBufferSurfaceDesc->Height;

    //g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
    g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 50000.0f);

    g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
    g_Camera.SetButtonMasks(MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON);

    g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
    g_HUD.SetSize(170, 170);


    //g_HUDStats.SetLocation(pBackBufferSurfaceDesc->Width - 190, 0);
    g_HUDStats.SetLocation(-35, wHeight - 290.0f);
    g_HUDStats.SetSize(290, 160);

    //g_HUDStats.SetTexture(1, charToWChar("D:\\GitHub\\DungeonStomp9\\UI\\box2.dds"));

    g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
    g_SampleUI.SetSize(170, 300);

    return S_OK;
}




//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
    float fElapsedTime, void* pUserContext)
{
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if (g_SettingsDlg.IsActive())
    {
        g_SettingsDlg.OnRender(fElapsedTime);
        return;
    }

    // Setup the camera's view parameters
    //D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
    //D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);

    if (maingameloop) {
        CheckMidiMusic();
    }

    float adjust = 50.0f;

    D3DXVECTOR3 vEyePt(m_vEyePt.x, m_vEyePt.y + adjust, m_vEyePt.z);
    D3DXVECTOR3 vLookatPt(m_vLookatPt.x, m_vLookatPt.y + adjust, m_vLookatPt.z);

    player_list[trueplayernum].x = m_vEyePt.x;
    player_list[trueplayernum].y = m_vEyePt.y + adjust;
    player_list[trueplayernum].z = m_vEyePt.z;

    g_Camera.SetViewParams(&vEyePt, &vLookatPt);


    for (int j = 0; j < cnt; j++)
    {
        //D3DXVECTOR3 a = D3DXVECTOR3(src_v[j].x, src_v[j].y, src_v[j].z);

        v[j].position.x = src_v[j].x;
        v[j].position.y = src_v[j].y;
        v[j].position.z = src_v[j].z;

        v[j].nx = src_v[j].nx;
        v[j].ny = src_v[j].ny;
        v[j].nz = src_v[j].nz;

        v[j].tu = src_v[j].tu;
        v[j].tv = src_v[j].tv;
    }

    ProcessLights11();

    //Vertex Buffer
    D3D11_MAPPED_SUBRESOURCE resource;
    pd3dImmediateContext->Map(g_pcbBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    int s = sizeof(Vertex) * cnt;
    memcpy(resource.pData, v, s);
    pd3dImmediateContext->Unmap(g_pcbBuffer, 0);

    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView(pRTV, ClearColor);

    // Clear the depth stencil
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

    // Get the projection & view matrix from the camera class
    D3DXMATRIX mWorld = *g_Camera.GetWorldMatrix();
    D3DXMATRIX mView = *g_Camera.GetViewMatrix();
    D3DXMATRIX mProj = *g_Camera.GetProjMatrix();
    D3DXMATRIX mWorldViewProjection = mWorld * mView * mProj;

    HRESULT hr;

    //Map world data
    D3D11_MAPPED_SUBRESOURCE MappedResource3;
    V(pd3dImmediateContext->Map(g_pcbVSPerObject11, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource3));
    CB_VS_PER_OBJECT* pVSPerObject = (CB_VS_PER_OBJECT*)MappedResource3.pData;
    D3DXMatrixTranspose(&pVSPerObject->m_mWorldViewProjection, &mWorldViewProjection);
    D3DXMatrixTranspose(&pVSPerObject->m_mWorld, &mWorld);
    //pVSPerObject->m_MaterialAmbientColor = D3DXVECTOR4(0.3f, 0.3f, 0.3f, 1.0f);
    //pVSPerObject->m_MaterialDiffuseColor = D3DXVECTOR4(0.7f, 0.7f, 0.7f, 1.0f);
    pVSPerObject->m_MaterialAmbientColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
    pVSPerObject->m_MaterialDiffuseColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
    pd3dImmediateContext->Unmap(g_pcbVSPerObject11, 0);
    pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVSPerObject11);

    //do lights
    D3D11_MAPPED_SUBRESOURCE MappedResource2;
    V(pd3dImmediateContext->Map(g_pcbVSLight11, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource2));
    PassConstants* pVSPerFrameLight = (PassConstants*)MappedResource2.pData;
    for (int i = 0;i < MaxLights;i++) {
        pVSPerFrameLight->Lights[i].Direction = mMainPassCB[i].Direction;
        pVSPerFrameLight->Lights[i].Strength = mMainPassCB[i].Strength;
        pVSPerFrameLight->Lights[i].Position = mMainPassCB[i].Position;
        pVSPerFrameLight->Lights[i].FalloffEnd = mMainPassCB[i].FalloffEnd;
        pVSPerFrameLight->Lights[i].FalloffStart = mMainPassCB[i].FalloffStart;
        pVSPerFrameLight->Lights[i].SpotPower = mMainPassCB[i].SpotPower;
    }

    pd3dImmediateContext->Unmap(g_pcbVSLight11, 0);
    pd3dImmediateContext->VSSetConstantBuffers(2, 1, &g_pcbVSLight11);  //(b2)
    pd3dImmediateContext->PSSetConstantBuffers(2, 1, &g_pcbVSLight11);

    // Set the constant buffers
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map(g_pcbVSPerFrame11, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
    CB_VS_PER_FRAME* pVSPerFrame = (CB_VS_PER_FRAME*)MappedResource.pData;
    pVSPerFrame->m_vLightDir = D3DXVECTOR3(0, 0.707f, -0.707f);
    pVSPerFrame->m_fTime = (float)fTime;
    pVSPerFrame->m_LightDiffuse = D3DXVECTOR4(1.f, 1.f, 1.f, 1.f);
    pd3dImmediateContext->Unmap(g_pcbVSPerFrame11, 0);
    pd3dImmediateContext->VSSetConstantBuffers(1, 1, &g_pcbVSPerFrame11);
    pd3dImmediateContext->PSSetConstantBuffers(1, 1, &g_pcbVSPerFrame11);

    // Set render resources
    pd3dImmediateContext->IASetInputLayout(g_pLayout11);
    pd3dImmediateContext->VSSetShader(g_pVertexShader11, NULL, 0);
    pd3dImmediateContext->PSSetShader(g_pPixelShader11, NULL, 0);
    pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);

    // Render objects here...
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));
    pd3dImmediateContext->IASetVertexBuffers(0, 1, &g_pcbBuffer, &stride, &offset);

    //Finally, draw everything.
    DrawScene(pd3dDevice, pd3dImmediateContext);

    DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
    g_HUD.OnRender(fElapsedTime);
    g_HUDStats.OnRender(fElapsedTime);
    g_SampleUI.OnRender(fElapsedTime);

    RenderText();
    DisplayHud();
    ScanMod();
    SetDiceTexture(true);
    SetDungeonText();

    DXUT_EndPerfEvent();

    static DWORD dwTimefirst = GetTickCount();
    if (GetTickCount() - dwTimefirst > 5000)
    {
        //OutputDebugString(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
        OutputDebugString(DXUTGetFrameStats(true));
        OutputDebugString(L"\n");
        dwTimefirst = GetTickCount();
    }
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render(ID3D11DeviceContext* pd3dImmediateContext)
{
    pd3dImmediateContext->DrawIndexed(36, 0, 0);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE(g_pTxtHelper);

    SAFE_RELEASE(g_pVertexShader11);
    SAFE_RELEASE(g_pPixelShader11);
    SAFE_RELEASE(g_pVertexShaderTorch11);
    SAFE_RELEASE(g_pPixelShaderTorch11);

    SAFE_RELEASE(g_pLayout11);
    SAFE_RELEASE(g_pcbVSLight11);
    SAFE_RELEASE(g_pSamLinear);

    // Delete additional render resources here...

    SAFE_RELEASE(g_pcbVSPerObject11);
    SAFE_RELEASE(g_pcbVSPerFrame11);
    SAFE_RELEASE(g_pcbBuffer);
    SAFE_RELEASE(g_pcbCaptionBuffer);

    SAFE_RELEASE(save_out_srv);

    for (int i = 0;i < 400;i++) {

        SAFE_RELEASE(textures[i]);


    }

}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
    if (pDeviceSettings->ver == DXUT_D3D9_DEVICE)
    {
        IDirect3D9* pD3D = DXUTGetD3D9Object();
        D3DCAPS9 Caps;
        pD3D->GetDeviceCaps(pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps);

        // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
        // then switch to SWVP.
        if ((Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
        {
            pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // Debugging vertex shaders requires either REF or software vertex processing 
        // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
        if (pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF)
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
            pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
#endif
#ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    }

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if (s_bFirstTime)
    {
        s_bFirstTime = false;
        if ((DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF) ||
            (DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
                pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE))
        {
            DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
        }

    }

    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
    UpdateControls();
    FrameMove(fTime, fElapsedTime);
    UpdateWorld(fElapsedTime);

    // Update the camera's position based on user input 
    // g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
    void* pUserContext)
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    // Pass messages to settings dialog if its active
    if (g_SettingsDlg.IsActive())
    {
        g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);

    return 0;
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
    switch (nControlID)
    {
    case IDC_TOGGLEFULLSCREEN:
        DXUTToggleFullScreen();
        break;
    case IDC_TOGGLEREF:
        DXUTToggleREF();
        break;
    case IDC_CHANGEDEVICE:
        g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
        break;
    }
}
