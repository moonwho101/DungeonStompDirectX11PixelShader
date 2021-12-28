//--------------------------------------------------------------------------------------
// File: SimpleSample9.cpp
//
// Basic starting point for new Direct3D 9 samples
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
#include "d3dtypes.h""
#include <string.h>
#include "LoadWorld.hpp"
#include "world.hpp"
#include "GlobalSettings.hpp"
#include "GameLogic.hpp"
//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 
#pragma comment(lib, "legacy_stdio_definitions.lib")
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
extern CModelViewerCamera           g_Camera;               // A model viewing camera
extern CDXUTDialogResourceManager   g_DialogResourceManager; // manager for shared resources of dialogs
extern CD3DSettingsDlg              g_SettingsDlg;          // Device settings dialog
extern CDXUTTextHelper* g_pTxtHelper;
extern CDXUTDialog                  g_HUD;                  // dialog for standard controls
extern CDXUTDialog                  g_SampleUI;             // dialog for sample specific controls

// Direct3D 9 resources
ID3DXFont* g_pFont9 = NULL;
ID3DXSprite* g_pSprite9 = NULL;
ID3DXSprite* g_pDice9 = NULL;
ID3DXSprite* g_pHud9 = NULL;
ID3DXEffect* g_pEffect9 = NULL;
D3DXHANDLE                          g_hmWorldViewProjection;
D3DXHANDLE                          g_hmWorld;
D3DXHANDLE                          g_hfTime;
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // Our texture
extern LPDIRECT3DTEXTURE9      g_pTextureList[MAX_NUM_TEXTURESLIST];


CUSTOMVERTEXTEST* pVertices;
CUSTOMVERTEXTEST* pBoundingBox;
CUSTOMVERTEXTEST* pMonsterCaption;

extern D3DVECTOR       m_vLookatPt;
extern D3DVECTOR       m_vEyePt;

float g_AspectRation = 0.0f;
float wWidth = 0;
float wHeight = 0;

D3DXMATRIXA16 effectcamera;
D3DXMATRIXA16 effectidentity;
D3DXMATRIXA16 g_matView;

float FastDistance(float fx, float fy, float fz);
void AddWorldLight(int ob_type, int angle, int oblist_index, IDirect3DDevice9* pd3dDevice);
void DrawText(IDirect3DDevice9* pd3dDevice, char* s);

void DrawScene(IDirect3DDevice9* pd3dDevice);

extern VOID SetupMatrices(IDirect3DDevice9* pd3dDevice);
extern void RenderText();
extern void ObjectToD3DVertList(int ob_type, int angle, int oblist_index);

POLY_SORT ObjectsToDraw[200000];
int itemlistcount = 0;
int num_players2 = 0;
bool init = false;

void InitShaderApp(IDirect3DDevice9* pd3dDevice);
void RenderRoomUsingBlinnPhong(IDirect3DDevice9* pd3dDevice);
void ReleaseEffect();
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
extern LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
extern void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
extern void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
extern void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
extern bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);

bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
	bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
void CALLBACK OnD3D9FrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext);
void CALLBACK OnD3D9LostDevice(void* pUserContext);
void CALLBACK OnD3D9DestroyDevice(void* pUserContext);

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	// Skip backbuffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if (FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat)))
		return false;

	// No fallback defined by this app, so reject any device that 
	// doesn't support at least ps2.0
	if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	// Use D3DX to create a texture from a file based image
	if (FAILED(D3DXCreateTextureFromFile(pd3dDevice, L"banana.bmp", &g_pTexture)))
	{
		// If texture is not in current folder, try parent folder
		if (FAILED(D3DXCreateTextureFromFile(pd3dDevice, L"..\\banana.bmp", &g_pTexture)))
		{
			MessageBox(NULL, L"Could not find banana.bmp", L"Textures.exe", MB_OK);
			return E_FAIL;
		}
	}



	//pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	//pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));    // ambient light
	//pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);    // handle normals in scaling
	pd3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
	//pd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);


	CLoadWorld* pCWorld;
	pCWorld = new CLoadWorld();
	if (!pCWorld->LoadRRTextures("textures.dat", pd3dDevice))
	{
	}

	//Dungeon Stomp Main Init
	//if (!init) {
	//	InitDS();
	//	init = true;
	//}

	// Create the vertex buffer.
	if (FAILED(pd3dDevice->CreateVertexBuffer(220000 * (3 * sizeof(CUSTOMVERTEXTEST)),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// Create the vertex buffer.
	if (FAILED(pd3dDevice->CreateVertexBuffer(5000 * (3 * sizeof(CUSTOMVERTEXTEST)),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVBBoundingBox, NULL)))
	{
		return E_FAIL;
	}


	// Create the vertex buffer.
	if (FAILED(pd3dDevice->CreateVertexBuffer(5000 * (3 * sizeof(CUSTOMVERTEXTEST)),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVBMonsterCaption, NULL)))
	{
		return E_FAIL;
	}


	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(g_SettingsDlg.OnD3D9CreateDevice(pd3dDevice));

	V_RETURN(D3DXCreateFont(pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Cascadia Code", &g_pFont9));

	// Read the D3DX effect file
//	WCHAR str[MAX_PATH];
//	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXFX_LARGEADDRESSAWARE;
//#ifdef DEBUG_VS
//	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
//#endif
//#ifdef DEBUG_PS
//	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
//#endif
//	V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"SimpleSample.fx"));
//	V_RETURN(D3DXCreateEffectFromFile(pd3dDevice, str, NULL, NULL, dwShaderFlags,
//		NULL, &g_pEffect9, NULL));


	InitShaderApp(pd3dDevice);

	//g_hmWorldViewProjection = g_pEffect9->GetParameterByName(NULL, "g_mWorldViewProjection");
	//g_hmWorld = g_pEffect9->GetParameterByName(NULL, "g_mWorld");
	//g_hfTime = g_pEffect9->GetParameterByName(NULL, "g_fTime");

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
	g_Camera.SetViewParams(&vecEye, &vecAt);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9* pd3dDevice,
	const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D9ResetDevice());
	V_RETURN(g_SettingsDlg.OnD3D9ResetDevice());

	if (g_pFont9) V_RETURN(g_pFont9->OnResetDevice());
	if (g_pEffect9) V_RETURN(g_pEffect9->OnResetDevice());

	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_pDice9));
	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_pHud9));
	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_pSprite9));
	g_pTxtHelper = new CDXUTTextHelper(g_pFont9, g_pSprite9, 15);

	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;

	wWidth = pBackBufferSurfaceDesc->Width;
	wHeight = (FLOAT)pBackBufferSurfaceDesc->Height;

	g_AspectRation = fAspectRatio - 0.6f;
	g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
	g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_HUD.SetSize(170, 170);
	g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 350);
	g_SampleUI.SetSize(170, 300);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext)
{
	HRESULT hr;
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mWorldViewProjection;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if (g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.OnRender(fElapsedTime);
		return;
	}

	UpdateVertexBuffer(pd3dDevice);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	// Clear the render target and the zbuffer 
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0));
	//V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0));


	RenderRoomUsingBlinnPhong(pd3dDevice);


	// Render the scene
	if (SUCCEEDED(pd3dDevice->BeginScene()))
	{
		SetupMatrices(pd3dDevice);

		pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		//pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEXTEST));
		pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		pd3dDevice->SetTexture(0, g_pTexture); //set texture

		//Draw the entire world
		DrawScene(pd3dDevice);

		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats"); // These events are to help PIX identify what the code is doing
		RenderText();
		V(g_HUD.OnRender(fElapsedTime));
		V(g_SampleUI.OnRender(fElapsedTime));
		DXUT_EndPerfEvent();
		V(pd3dDevice->EndScene());
	}
}


//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mWorldViewProjection;


	// Set up world matrix
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	//D3DXMatrixRotationX(&matWorld, timeGetTime() / 7000.0f);
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
	
	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the
	// origin, and define "up" to be in the y-direction.

	float adjust = 50.0f;

	player_list[trueplayernum].x = m_vEyePt.x;
	player_list[trueplayernum].y = m_vEyePt.y + adjust;
	player_list[trueplayernum].z = m_vEyePt.z;

	D3DXVECTOR3 vEyePt(m_vEyePt.x, m_vEyePt.y + adjust, m_vEyePt.z);
	D3DXVECTOR3 vLookatPt(m_vLookatPt.x, m_vLookatPt.y + adjust, m_vLookatPt.z);

	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	
	pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
	
	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 2, g_AspectRation,0.1f, 50000.0f);
	
	pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	
	

	D3DXMATRIXA16 mTextureTransform;
	D3DXMATRIXA16 mTrans;
	D3DXMATRIXA16 mScale;

	// Get the projection & view matrix from the camera class
	mWorld = matWorld;
	mProj = matProj;
	

	mWorldViewProjection = matView * matProj;


	// Update the effect's variables.  Instead of using strings, it would 
	// be more efficient to cache a handle to the parameter by calling 
	// ID3DXEffect::GetParameterByName
	//V(g_pEffect9->SetMatrix(g_hmWorldViewProjection, &mWorldViewProjection));
	//V(g_pEffect9->SetMatrix(g_hmWorld, &mWorld));
	//V(g_pEffect9->SetFloat(g_hfTime, (float)0.0f));

	effectcamera = mWorldViewProjection;
	effectidentity = mWorld;
	g_matView = matView;


}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	
	if (bKeyDown)
	{
		switch (nChar)
		{
		case 88: //x
			player_list[trueplayernum].xp += 10;
			break;
		
		}
	}
	

}
//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D9LostDevice();
	g_SettingsDlg.OnD3D9LostDevice();
	if (g_pFont9) g_pFont9->OnLostDevice();
	if (g_pEffect9) g_pEffect9->OnLostDevice();
	SAFE_RELEASE(g_pSprite9);
	SAFE_RELEASE(g_pDice9);
	SAFE_RELEASE(g_pHud9);
	SAFE_DELETE(g_pTxtHelper);
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D9DestroyDevice();
	g_SettingsDlg.OnD3D9DestroyDevice();
	SAFE_RELEASE(g_pEffect9);
	SAFE_RELEASE(g_pFont9);
	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pVBBoundingBox);
	SAFE_RELEASE(g_pVBMonsterCaption);
	SAFE_RELEASE(g_pTexture);


	ReleaseEffect();


	g_pTexture = NULL;

	for (int i = 0;i < 400;i++) {
		SAFE_RELEASE(g_pTextureList[i]);
		g_pTextureList[i] = NULL;
	}
}
