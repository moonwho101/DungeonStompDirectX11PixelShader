#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"
#include "d3dtypes.h""
#include "LoadWorld.hpp"
#include "world.hpp"
#include "GlobalSettings.hpp"
#include <string.h>
#include "GameLogic.hpp"
#include "Missle.hpp"
#include <string>

#include <d3d9.h>
#include <d3dx9.h>
#include "d3dutil.h"

void SetDungeonText();
extern LPDIRECT3DTEXTURE9      g_pTextureList[MAX_NUM_TEXTURESLIST];
void SetMaterialBright(IDirect3DDevice9* pd3dDevice);
void SetMaterialDull(IDirect3DDevice9* pd3dDevice);
void DisplayHud();
void DrawSceneAlpha(IDirect3DDevice9* pd3dDevice, bool torch);
void UpdateLights(float elapsedTimeSec);
void UpdateEffects(IDirect3DDevice9* pd3dDevice);
void DrawBoundingBox(IDirect3DDevice9* pd3dDevice);
void ShowScreenOverlays(IDirect3DDevice9* pd3dDevice);
void ProcessLights();
void SetDiceTexture(bool showroll);
void DisplayPlayerCaption(IDirect3DDevice9* pd3dDevice);
void statusbardisplay(float x, float length, int type);
void SmoothNormals(int start_cnt);

extern int textcounter;
extern gametext gtext[200];
int countdisplay = 0;
extern char statusbar[255];

D3DVERTEX bubble[600];
D3DTLVERTEX m_BackgroundMesh[2000];


D3DXHANDLE m_WVPMatrixHandle;//World-Observation-Projection Matrix Handle
 //- ---------- Effect Framework -----------------
ID3DXEffect* m_NLightEffect = 0;
//------------- ---- Handle ------------------------
D3DXHANDLE m_TecniMulLightsHandle = 0;//Technical handle
D3DXHANDLE m_AmbAmountHandle = 0;//Ambient light coefficient handle
 //Various light source handles
D3DXHANDLE m_PointLHandle = 0;//Point light source handle
D3DXHANDLE m_DirectLightHandle = 0;//Direction light handle
D3DXHANDLE m_SpotLightHandle = 0;//Spotlight handle
 //Various light source quantity handles
D3DXHANDLE m_NumPointLightHandle = 0;//Point light source
D3DXHANDLE m_NumDirectLightHandle = 0;//Direction light number handle
D3DXHANDLE m_NumSpotLightHandle = 0;//Spot light number handle
D3DXHANDLE m_ViewPosHandle = 0;//Viewpoint handle
D3DXHANDLE m_WorldMatrixHandle = 0;//World matrix handle

typedef D3DXVECTOR4 float4;//Here typedef becomes float4, it can be consistent with the shader, no need to change the code!

struct PointLight
{
	float4 Diffuse;//The color of diffuse reflection
	float4 Specular;//Color of specular highlight
	float4 Position;//Light source position	
	float4 RangeAttenuation;//Range, constant attenuation, mirror light intensity, secondary attenuation
};

struct DirectLight
{
	float4 Diffuse;//The color of diffuse reflection
	float4 Specular;//Color of specular highlight
	float4 DirectionPow;//direction x, y, z, highlight
};

struct SpotLight
{
	float4 Diffuse;//The color of diffuse reflection
	float4 Specular;//Color of specular highlight
	float4 Position;//Light source position	
	float4 Direction;//Direction
	float4 RangeAttenuation;//Range, constant attenuation, mirror light intensity, secondary attenuation
	float4 FalloffThetaPhi;//Intensity attenuation from inner cone to outer cone, NULL, radian of inner cone, radian of outer cone
};

struct Material
{
	float ambient[4];
	float diffuse[4];
	float emissive[4];
	float specular[4];
	float shininess;
};
float                        g_sceneAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

Material g_dullMaterial =
{
	0.2f, 0.2f, 0.2f, 1.0f,
	0.8f, 0.8f, 0.8f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f
};

Material g_shinyMaterial =
{
	0.2f, 0.2f, 0.2f, 1.0f,
	0.8f, 0.8f, 0.8f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	32.0f
};


PointLight m_PointLight[10];
DirectLight m_DirectLight[10];
SpotLight m_SpotLight[10];

ID3DXEffect* g_pBlinnPhongEffectSM20;
ID3DXEffect* g_pBlinnPhongEffectSM30;
//ID3DXEffect* g_pBlinnPhongEffect;
ID3DXEffect* g_pAmbientEffect;

int g_numLights;
const int MAX_LIGHTS_SM20 = 2;
const int MAX_LIGHTS_SM30 = 8;
bool g_enableMultipassLighting = 0;

extern char gfinaltext[2048];

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

//
// and the inverse
//

#define FAILED(hr) (((HRESULT)(hr)) < 0)
bool                         g_supportsShaderModel30;



bool LoadShader(const char* pszFilename, LPD3DXEFFECT& pEffect, IDirect3DDevice9* pd3dDevice)
{
	ID3DXBuffer* pCompilationErrors = 0;
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER;

	// Both vertex and pixel shaders can be debugged. To enable shader
	// debugging add the following flag to the dwShaderFlags variable:
	//      dwShaderFlags |= D3DXSHADER_DEBUG;
	//
	// Vertex shaders can be debugged with either the REF device or a device
	// created for software vertex processing (i.e., the IDirect3DDevice9
	// object must be created with the D3DCREATE_SOFTWARE_VERTEXPROCESSING
	// behavior). Pixel shaders can be debugged only using the REF device.
	//
	// To enable vertex shader debugging add the following flag to the
	// dwShaderFlags variable:
	//     dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
	//
	// To enable pixel shader debugging add the following flag to the
	// dwShaderFlags variable:
	//     dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;

	HRESULT hr = D3DXCreateEffectFromFile(pd3dDevice, charToWChar(pszFilename), 0, 0,
		dwShaderFlags, 0, &pEffect, &pCompilationErrors);

	if (FAILED(hr))
	{
		if (pCompilationErrors)
		{
			std::string compilationErrors(static_cast<const char*>(
				pCompilationErrors->GetBufferPointer()));

			pCompilationErrors->Release();
			//throw std::runtime_error(compilationErrors);
		}
	}

	if (pCompilationErrors)
		pCompilationErrors->Release();

	return pEffect != 0;
}



void DrawScene(IDirect3DDevice9* pd3dDevice) {

	int currentObject = 0;


	int arr[200];
	int n, i, j;

	char junk[255];

	ScanMod();
	SetDiceTexture(false);


	SetDungeonText();




	if (maingameloop) {
		CheckMidiMusic();
	}

	//for (i = 0; i < 100; i++)
		//pd3dDevice->LightEnable((DWORD)i, FALSE);


	//FlashLight(pd3dDevice);

	ProcessLights();

	//UpdateLights(0.0f);
	UpdateEffects(pd3dDevice);


	//sprintf(junk, "%f", g_AspectRation);
	//RenderText(junk);

	/*
	for (int q = 0; q < oblist_length; q++)
	{
		int angle = (int)oblist[q].rot_angle;
		int ob_type = oblist[q].type;

		float	qdist = FastDistance(m_vEyePt.x - oblist[q].x,
			m_vEyePt.y - oblist[q].y,
			m_vEyePt.z - oblist[q].z);


		if (ob_type == 6 && qdist < 900)
		{
			AddWorldLight(ob_type, angle, q, pd3dDevice);
		}
	}
	*/


	//D3DMATERIAL9 material;
	//ZeroMemory(&material, sizeof(D3DMATERIAL9));
	//material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	//material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	//pd3dDevice->SetMaterial(&material);

	//SetMaterialDull(pd3dDevice);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	static UINT totalPasses;
	if (SUCCEEDED(g_pBlinnPhongEffectSM30->Begin(&totalPasses, 0)))
	{
		for (currentObject = 0; currentObject < number_of_polys_per_frame; currentObject++)
			//for (lsort = 0; lsort < 1800; lsort++)
		{
			int i = ObjectsToDraw[currentObject].vert_index;
			int vert_index = ObjectsToDraw[currentObject].srcstart;
			int fperpoly = (float)ObjectsToDraw[currentObject].srcfstart;
			int face_index = ObjectsToDraw[currentObject].srcfstart;

			int texture_alias_number = texture_list_buffer[i];
			int texture_number = TexMap[texture_alias_number].texture;

			pd3dDevice->SetTexture(0, g_pTextureList[texture_number]); //set texture



			if (dp_command_index_mode[i] == 1 && TexMap[texture_alias_number].is_alpha_texture == FALSE) {  //USE_NON_INDEXED_DP
				int primitive = 0;
				HRESULT hr = g_pBlinnPhongEffectSM30->SetTexture((UINT_PTR)"colorMapTexture", g_pTextureList[texture_number]);

				if (dp_commands[currentObject] == D3DPT_TRIANGLEFAN || dp_commands[currentObject] == D3DPT_TRIANGLESTRIP)
				{
					primitive = (verts_per_poly[currentObject] - 2);
				}
				else if (dp_commands[currentObject] == D3DPT_TRIANGLELIST)
				{
					primitive = verts_per_poly[currentObject] / 3;
				}

				for (UINT pass = 0; pass < totalPasses; ++pass)
				{
					if (SUCCEEDED(g_pBlinnPhongEffectSM30->BeginPass(pass)))
					{
						pd3dDevice->DrawPrimitive(dp_commands[currentObject], vert_index, primitive);
						g_pBlinnPhongEffectSM30->EndPass();
					}
				} // end for i
			}
			else {
				//DrawIndexedItems(pd3dDevice, currentObject, vert_index);
			}
		}
	}

	DrawSceneAlpha(pd3dDevice, false);
	g_pBlinnPhongEffectSM30->End();
	DrawSceneAlpha(pd3dDevice, true);
	//DrawBoundingBox(pd3dDevice);
	SetMaterialBright(pd3dDevice);
	DisplayPlayerCaption(pd3dDevice);
	ShowScreenOverlays(pd3dDevice);
	DisplayHud();
}

void SetDungeonText()
{

	for (int q = 0; q < oblist_length; q++)
	{
		int angle = (int)oblist[q].rot_angle;
		int ob_type = oblist[q].type;
		if (ob_type == 120)
		{
			float	qdist = FastDistance(m_vEyePt.x - oblist[q].x, m_vEyePt.y - oblist[q].y, m_vEyePt.z - oblist[q].z);
			if (qdist < 500.0f) {
				if (strstr(oblist[q].name, "text") != NULL)
				{
					for (int il = 0; il < textcounter; il++)
					{
						if (gtext[il].textnum == q)
						{
							if (gtext[il].type == 0)
							{
								strcpy_s(gfinaltext, gtext[il].text);
							}
							else if (gtext[il].type == 1 || gtext[il].type == 2)
							{
								if (qdist < 200.0f)
								{

									DisplayDialogText(gtext[il].text, 0.0f);
								}
							}
						}
					}
				}
			}
		}
	}
}


void DrawSceneAlpha(IDirect3DDevice9* pd3dDevice, bool torch)
{


	if (torch) {
		SetMaterialBright(pd3dDevice);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	else {
		SetMaterialDull(pd3dDevice);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	}

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);


	//pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
 //   pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000081);
 //   pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);



	int currentObject = 0;
	for (currentObject = 0; currentObject < number_of_polys_per_frame; currentObject++)
		//for (lsort = 0; lsort < 1800; lsort++)
	{
		int i = ObjectsToDraw[currentObject].vert_index;
		int vert_index = ObjectsToDraw[currentObject].srcstart;
		int fperpoly = (float)ObjectsToDraw[currentObject].srcfstart;
		int face_index = ObjectsToDraw[currentObject].srcfstart;

		int texture_alias_number = texture_list_buffer[i];
		int texture_number = TexMap[texture_alias_number].texture;

		bool draw = true;
		//94-101 (95-102 in file) - Make the torch light bright
		//289-296 - spell explode
		//279-288 - spell shine
		//206 -210 spell flare

		if (torch) {

			if (texture_number >= 94 && texture_number <= 101 ||
				texture_number >= 289 - 1 && texture_number <= 296 - 1 ||
				texture_number >= 279 - 1 && texture_number <= 288 - 1 ||
				texture_number >= 206 - 1 && texture_number <= 210 - 1) {
				draw = true;
			}
			else {
				draw = false;
			}
		}

		if (draw) {
			pd3dDevice->SetTexture(0, g_pTextureList[texture_number]); //set texture

			if (dp_command_index_mode[i] == 1 && TexMap[texture_alias_number].is_alpha_texture == TRUE) {  //USE_NON_INDEXED_DP
				int primitive = 0;

				if (dp_commands[currentObject] == D3DPT_TRIANGLEFAN || dp_commands[currentObject] == D3DPT_TRIANGLESTRIP)
				{
					primitive = (verts_per_poly[currentObject] - 2);
				}
				else if (dp_commands[currentObject] == D3DPT_TRIANGLELIST)
				{
					primitive = verts_per_poly[currentObject] / 3;
				}

				pd3dDevice->DrawPrimitive(dp_commands[currentObject], vert_index, primitive);
			}
			else {
				//DrawIndexedItems(pd3dDevice, currentObject, vert_index);
			}
		}

	} // end for i
}


void DrawIndexedItems(int fakel, int vert_index)
{
	D3DPRIMITIVETYPE command;
	int face_index = 0;
	D3DXVECTOR3 vw1, vw2, vw3;
	float workx, worky, workz;

	if (dp_command_index_mode[fakel] == 0) //USE_INDEXED_DP
	{
		int dwIndexCount = ObjectsToDraw[fakel].facesperpoly * 3;
		int dwVertexCount = ObjectsToDraw[fakel].vertsperpoly;
		command = dp_commands[fakel];
		face_index = ObjectsToDraw[fakel].srcfstart;

		int prim = ObjectsToDraw[fakel].facesperpoly;

		dp_command_index_mode[fakel] = 1;
		verts_per_poly[fakel] = dwIndexCount;
		ObjectsToDraw[fakel].srcstart = cnt;
		ObjectsToDraw[fakel].vertsperpoly = dwIndexCount;

		for (int t = 0; t < (int)dwIndexCount; t++)
		{
			int f_index = src_f[face_index + t];
			memset(&temp_v[t], 0, sizeof(D3DVERTEX));
			memcpy(&temp_v[t], &src_v[vert_index + f_index],
				sizeof(D3DVERTEX));
		}

		int counttri = 0;

		for (int t = 0; t < (int)dwIndexCount; t++)
		{
			if (counttri == 0)
			{

				vw1.x = temp_v[t].x;
				vw1.y = temp_v[t].y;
				vw1.z = temp_v[t].z;

				vw2.x = temp_v[t + 1].x;
				vw2.y = temp_v[t + 1].y;
				vw2.z = temp_v[t + 1].z;

				vw3.x = temp_v[t + 2].x;
				vw3.y = temp_v[t + 2].y;
				vw3.z = temp_v[t + 2].z;

				// calculate the NORMAL for the road using the CrossProduct <-important!

				D3DXVECTOR3 vDiff = vw1 - vw2;
				D3DXVECTOR3 vDiff2 = vw3 - vw2;

				D3DXVECTOR3 vCross, final;

				D3DXVec3Cross(&vCross, &vDiff, &vDiff2);
				D3DXVec3Normalize(&final, &vCross);

				workx = -final.x;
				worky = -final.y;
				workz = -final.z;
			}

			counttri++;
			if (counttri > 2)
				counttri = 0;

			temp_v[t].nx = workx;
			temp_v[t].ny = worky;
			temp_v[t].nz = workz;

		}

		int start_cnt = cnt;

		for (int j = 0; j < dwIndexCount; j++)
		{
			D3DXVECTOR3 a = D3DXVECTOR3(temp_v[j].x, temp_v[j].y, temp_v[j].z);
			src_v[cnt].x = temp_v[j].x;
			src_v[cnt].y = temp_v[j].y;
			src_v[cnt].z = temp_v[j].z;
			src_v[cnt].tu = temp_v[j].tu;
			src_v[cnt].tv = temp_v[j].tv;
			src_v[cnt].nx = temp_v[j].nx;
			src_v[cnt].ny = temp_v[j].ny;
			src_v[cnt].nz = temp_v[j].nz;
			cnt++;
		}

		SmoothNormals(start_cnt);

	}
}



void DisplayHud() {

	char junk[255];

	float adjust = 170.0f;

	sprintf_s(junk, "Dungeon Stomp 1.90");
	display_message(5.0f, (FLOAT)wHeight - adjust - 14.0f, junk, 255, 255, 0, 12.5, 16, 0);

	sprintf_s(junk, "AREA: ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 10.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%s", gfinaltext);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 10.0f, junk, 0, 245, 255, 12.5, 16, 0);

	//statusbardisplay((float)player_list[trueplayernum].hp, (float)player_list[trueplayernum].hp, 1);

	sprintf_s(junk, "HP  : ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 24.0f, junk, 255, 255, 0, 12.5, 16, 0);

	sprintf_s(junk, "%d/%d", player_list[trueplayernum].health, player_list[trueplayernum].hp);
	display_message(0.0f + 110.0f, (FLOAT)wHeight - adjust + 24.0f, junk, 255, 255, 255, 12.5, 16, 0);


	//sprintf_s(junk, "%s %d/%d", statusbar, player_list[trueplayernum].health, player_list[trueplayernum].hp);
	//display_message(0.0f + 65.0f, (FLOAT)vp.dwHeight - adjust + 24.0f, junk, vp, 255, 255, 255, 12.5, 16, 0);

	statusbardisplay((float)player_list[trueplayernum].health, (float)player_list[trueplayernum].hp, 0);
	sprintf_s(junk, "%s", statusbar);

	//for (int i = 0;i < strlen(junk);i++) {
		//if (junk[i] == '|')
			//junk[i] = char(260);
	//}

	//display_message(0.0f + 61.0f, (FLOAT)wHeight - adjust + 24.0f, junk, 255, 0, 0, 12.5, 16, 0);

	//if (strlen(junk) < 8) {
	//	int i = 0;
	//	int c = 8 - strlen(junk);

	//	for (i = 0;i < c;i++) {
	//		junk[i] = char(260);
	//	}
	//	junk[i] = '\0';

	//	display_message(0.0f + 94.0f - ((FLOAT) c * 5.0f), (FLOAT)wHeight - adjust + 24.0f, junk, 255, 255, 255, 12.5, 16, 0);
	//}

	int c = strlen(junk);
	char jj[255];
	sprintf(jj, "%c", char(260));

	

	for (int i = 0;i < 8;i++) {

		if (i < c) {
			display_message(0.0f + 58.0f + ((FLOAT)i * 5.0f), (FLOAT)wHeight - adjust + 24.0f, jj, 255, 0, 0, 12.5, 16, 0);
		}
		else {
			display_message(0.0f + 58.0f + ((FLOAT)i * 5.0f), (FLOAT)wHeight - adjust + 24.0f, jj, 255, 255, 255, 12.5, 16, 0);
		}
	}




	sprintf_s(junk, "WPN : ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 38.0f, junk, 255, 255, 0, 12.5, 16, 0);

	char junk3[255];
	if (strstr(your_gun[current_gun].gunname, "SCROLL-MAGICMISSLE") != NULL)
	{
		strcpy_s(junk3, "MAGIC MISSLE");
		sprintf_s(junk, "%s: %d", junk3, (int)your_gun[current_gun].x_offset);
	}
	else if (strstr(your_gun[current_gun].gunname, "SCROLL-FIREBALL") != NULL)
	{
		strcpy_s(junk3, "FIREBALL");
		sprintf_s(junk, "%s: %d", junk3, (int)your_gun[current_gun].x_offset);
	}
	else if (strstr(your_gun[current_gun].gunname, "SCROLL-LIGHTNING") != NULL)
	{
		strcpy_s(junk3, "LIGHTNING");
		sprintf_s(junk, "%s: %d", junk3, (int)your_gun[current_gun].x_offset);
	}
	else if (strstr(your_gun[current_gun].gunname, "SCROLL-HEALING") != NULL)
	{
		strcpy_s(junk3, "HEALING");
		sprintf_s(junk, "%s: %d", junk3, (int)your_gun[current_gun].x_offset);
	}

	else
	{
		sprintf_s(junk, "%s", your_gun[current_gun].gunname);
	}
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 38.0f, junk, 0, 245, 255, 12.5, 16, 0);

	sprintf_s(junk, "DMG :");
	display_message(0.0f, (FLOAT)wHeight - adjust + 52.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%dD%d", player_list[trueplayernum].damage1, player_list[trueplayernum].damage2);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 52.0f, junk, 0, 245, 255, 12.5, 16, 0);


	int attackbonus = your_gun[current_gun].sattack;
	int damagebonus = your_gun[current_gun].sdamage;


	sprintf_s(junk, "BNS : ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 66.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "+%d/%+d", attackbonus, damagebonus);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 66.0f, junk, 0, 245, 255, 12.5, 16, 0);

	int nextlevelxp = LevelUpXPNeeded(player_list[trueplayernum].xp) + 1;

	sprintf_s(junk, "XP  : ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 80.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%d", player_list[trueplayernum].xp);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 80.0f, junk, 0, 245, 255, 12.5, 16, 0);

	sprintf_s(junk, "LVL : ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 94.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%d (%d)", player_list[trueplayernum].hd, nextlevelxp);
	//sprintf_s(junk, "%d (%d)", player_list[trueplayernum].hd, 0);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 94.0f, junk, 0, 245, 255, 12.5, 16, 0);

	sprintf_s(junk, "ARMR: ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 108.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%d", player_list[trueplayernum].ac);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 108.0f, junk, 0, 245, 255, 12.5, 16, 0);

	sprintf_s(junk, "THAC: ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 122.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%d", player_list[trueplayernum].thaco);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 122.0f, junk, 0, 245, 255, 12.5, 16, 0);

	sprintf_s(junk, "GOLD: ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 136.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%d", player_list[trueplayernum].gold);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 136.0f, junk, 0, 245, 255, 12.5, 16, 0);

	sprintf_s(junk, "KEYS: ");
	display_message(0.0f, (FLOAT)wHeight - adjust + 150.0f, junk, 255, 255, 0, 12.5, 16, 0);
	sprintf_s(junk, "%d", player_list[trueplayernum].keys);
	display_message(0.0f + 60.0f, (FLOAT)wHeight - adjust + 150.0f, junk, 0, 245, 255, 12.5, 16, 0);

	int flag = 1;
	float scrollmessage1 = 60;
	int count = 0;
	int scount = 0;
	char junk2[2048];
	scrolllistnum = 6;

	scount = sliststart;
	scrollmessage1 = 14.0f * (scrolllistnum + 2);


	while (flag)
	{
		sprintf_s(junk2, "%s", scrolllist1[scount].text);
		display_message(0.0f, scrollmessage1, junk2, scrolllist1[scount].r, scrolllist1[scount].g, scrolllist1[scount].b, 12.5, 16, 0);
		scrollmessage1 -= 14.0f;

		count++;
		scount--;

		if (scount < 0)
			scount = scrolllistnum - 1;

		if (count >= scrolllistnum)
			flag = 0;
	}

}




void SetMaterialBright(IDirect3DDevice9* pd3dDevice)
{
	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));

	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Emissive.r = 1.0f;
	material.Emissive.g = 1.0f;
	material.Emissive.b = 1.0f;

	pd3dDevice->SetMaterial(&material);
}

void SetMaterialDull(IDirect3DDevice9* pd3dDevice)
{
	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));

	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Emissive.r = 0.0f;
	material.Emissive.g = 0.0f;
	material.Emissive.b = 0.0f;

	material.Specular.r = 0.3f;
	material.Specular.g = 0.3f;
	material.Specular.b = 0.3f;
	material.Specular.a = 0.0f;

	material.Power = 40.0f;

	pd3dDevice->SetMaterial(&material);
}


static wchar_t* charToWChar(const char* text)
{
	const size_t size = strlen(text) + 1;
	wchar_t* wText = new wchar_t[size];
	mbstowcs(wText, text, size);
	return wText;
}

void ReleaseEffect() {

	SAFE_RELEASE(g_pAmbientEffect);
	SAFE_RELEASE(g_pBlinnPhongEffectSM20);
	SAFE_RELEASE(g_pBlinnPhongEffectSM30);

}

void RenderRoomUsingBlinnPhong(IDirect3DDevice9* pd3dDevice)
{
	static UINT totalPasses;
	static D3DXHANDLE hTechnique;


	//hTechnique = g_pBlinnPhongEffect->GetTechniqueByName("MulLights");


	//if (FAILED(g_pBlinnPhongEffect->SetTechnique(hTechnique)))
		//return;


	m_TecniMulLightsHandle = g_pBlinnPhongEffectSM30->GetTechniqueByName("MulLights");
	//Number of various light sources
	m_NumPointLightHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_NumPLs");
	m_NumDirectLightHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_NumDLs");
	m_NumSpotLightHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_NumSLs");
	//Get the handle of the light source type
	m_AmbAmountHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "AmbAmount");//Ambient light factor handle	
	m_PointLHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_PLs");//Point light handle
	m_DirectLightHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_DLs");//Direction light handle
	m_SpotLightHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_SLs");//Spotlight handle
		//Get observation point, matrix handle
	m_ViewPosHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_ViewPos");
	m_WorldMatrixHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "g_WorldMatrix");
	m_WVPMatrixHandle = g_pBlinnPhongEffectSM30->GetParameterByName(NULL, "WVPMatrix");//World observation projection matrix

	//Set the constant handle
		//pass in the light source data
		// 
	g_pBlinnPhongEffectSM30->SetRawValue(m_PointLHandle, m_PointLight, 0, sizeof(PointLight) * 10);//Transmit point light
	//g_pBlinnPhongEffectSM30->SetRawValue(m_PointLHandle, m_PointLight, 0, sizeof(PointLight) * 2);//Transmit point light
	//g_pBlinnPhongEffectSM30->SetRawValue(m_DirectLightHandle, m_DirectLight, 0, sizeof(DirectLight) * 1);//Direction light
	g_pBlinnPhongEffectSM30->SetRawValue(m_SpotLightHandle, m_SpotLight, 0, sizeof(SpotLight) * 8);//Spotlight

	g_pBlinnPhongEffectSM30->SetFloat(m_AmbAmountHandle, 0.0f);
	g_pBlinnPhongEffectSM30->SetInt(m_NumPointLightHandle, 10);//Number of point light sources: 2 (Adjusting the number can facilitate debugging)
	g_pBlinnPhongEffectSM30->SetInt(m_NumDirectLightHandle, 0);//Number of directional lights: 1
	g_pBlinnPhongEffectSM30->SetInt(m_NumSpotLightHandle, 8);//Number of spotlights: 1



}



extern D3DXMATRIXA16 effectcamera;
extern D3DXMATRIXA16 effectidentity;
extern D3DXMATRIXA16 effectidentity;
extern D3DXMATRIXA16 g_matView;

void UpdateEffects(IDirect3DDevice9* pd3dDevice)
{

	//m_NLightEffect->SetVector(m_ViewPosHandle, &effectcamera);
	//m_NLightEffect->SetVector(m_ViewPosHandle, &effectcamera);
	//m_NLightEffect->SetVector(m_ViewPosHandle, &effectcamera);



	g_pBlinnPhongEffectSM30->SetMatrix(m_WorldMatrixHandle, &effectidentity); //mWorld
	g_pBlinnPhongEffectSM30->SetMatrix(m_WVPMatrixHandle, &effectcamera);
	g_pBlinnPhongEffectSM30->SetMatrix(m_ViewPosHandle, &g_matView);
	g_pBlinnPhongEffectSM30->SetRawValue(m_PointLHandle, m_PointLight, 0, sizeof(PointLight) * 10);//Transmit point light
	g_pBlinnPhongEffectSM30->SetRawValue(m_SpotLightHandle, m_SpotLight, 0, sizeof(SpotLight) * 8);

	/*

	ID3DXEffect* pEffect = g_pBlinnPhongEffect;

	// Set the matrices for the shader.
		// Set the matrices for the shader.


	pEffect->SetMatrix((UINT_PTR)"worldMatrix", &effectidentity);
	pEffect->SetMatrix((UINT_PTR)"worldInverseTransposeMatrix", &effectidentity);
	pEffect->SetMatrix((UINT_PTR)"worldViewProjectionMatrix", &effectcamera);

	// Set the camera position.
	D3DXVECTOR3 e = D3DXVECTOR3(m_vEyePt.x, m_vEyePt.y, m_vEyePt.z);
	HRESULT hr = pEffect->SetValue((UINT_PTR)"cameraPos", e, sizeof(e));


	//pEffect->SetValue((UINT_PTR)"globalAmbient", &g_sceneAmbient, sizeof(g_sceneAmbient));

	// Set the number of active lights. For shader model 3.0 only.

	if (pEffect == g_pBlinnPhongEffectSM30)
		pEffect->SetValue((UINT_PTR)"numLights", &g_numLights, sizeof(g_numLights));

	// Set the lighting parameters for the shader.

	const PointLight* pLight = 0;
	D3DXHANDLE hLight;
	D3DXHANDLE hLightPos;
	D3DXHANDLE hLightAmbient;
	D3DXHANDLE hLightDiffuse;
	D3DXHANDLE hLightSpecular;
	D3DXHANDLE hLightRadius;

	for (int i = 0; i < g_numLights; ++i)
	{
		pLight = &g_lights[i];
		hLight = pEffect->GetParameterElement((UINT_PTR)"lights", i);

		hLightPos = pEffect->GetParameterByName(hLight, "pos");
		hLightAmbient = pEffect->GetParameterByName(hLight, "ambient");
		hLightDiffuse = pEffect->GetParameterByName(hLight, "diffuse");
		hLightSpecular = pEffect->GetParameterByName(hLight, "specular");
		hLightRadius = pEffect->GetParameterByName(hLight, "radius");

		pEffect->SetValue(hLightPos, pLight->pos, sizeof(pLight->pos));
		pEffect->SetValue(hLightAmbient, pLight->ambient, sizeof(pLight->ambient));
		pEffect->SetValue(hLightDiffuse, pLight->diffuse, sizeof(pLight->diffuse));
		pEffect->SetValue(hLightSpecular, pLight->specular, sizeof(pLight->specular));
		pEffect->SetFloat(hLightRadius, pLight->radius);
	}
	*/
}



void InitShaderApp(IDirect3DDevice9* pd3dDevice)
{




	if (!LoadShader("../Content/Shaders/nlight_sm30.fx", g_pBlinnPhongEffectSM30, pd3dDevice)) {
		//	throw std::runtime_error("Failed to load shader: blinn_phong_sm30.fx.");

		int stop = 1;
	}
	g_numLights = MAX_LIGHTS_SM30;


	// Seed the random number generator.
	srand(GetTickCount());


	//m_PointLight [0]

	for (int i = 0; i < 10; ++i) {
		m_PointLight[i].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };//Diffuse
		m_PointLight[i].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };//Specular
		m_PointLight[i].Position = { -6.0f, 2.0f, 2.0f, 0.0f };//Position
		m_PointLight[i].RangeAttenuation = { 175.0f, 0.0f, 1.0f, 0.0f };  //x = radius z=speculaor
	}


	for (int i = 0; i < 10; ++i) {
		m_SpotLight[i].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_SpotLight[i].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_SpotLight[i].Position = { 0.0f, 1.8f, -5.0f, 0.0f };
		//m_SpotLight[i].Direction = { 1.0f, -1.0f, -5.0f, 0.0f };
		m_SpotLight[i].Direction = { 0.0f, -1.0f, 0.0f, 0.0f };
		m_SpotLight[i].FalloffThetaPhi = { 3.5f, 0.0f, 0.14f, 1.98f };//Attenuation,-, inner angle (half angle in radians), outer angle (half angle in radians)
		m_SpotLight[i].RangeAttenuation = { 800.0f, 0.0f, 0.0f, 0.0f };//Range, attenuation 1, highlight, attenuation 2
	}

	//m_SpotLight[i].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	//m_SpotLight[i].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	//m_SpotLight[i].Position = { 0.0f, 1.8f, -5.0f, 0.0f };
	////m_SpotLight[i].Direction = { 1.0f, -1.0f, -5.0f, 0.0f };
	//m_SpotLight[i].Direction = { 0.0f, -1.0f, -1.0f, 0.0f };
	//m_SpotLight[i].FalloffThetaPhi = { 3.5f, 0.0f, 0.04f, 1.00f };//Attenuation,-, inner angle (half angle in radians), outer angle (half angle in radians)
	//m_SpotLight[i].RangeAttenuation = { 800.0f, 0.0f, 10.0f, 0.0f };//Range, attenuation 1, highlight, attenuation 2




	//m_PointLight[7].RangeAttenuation = { 200.0f, 0.0f, 950.0f, 0.0f };  //x = radius z=speculaor

	// Initialize the point lights in the scene.

	//for (int i = 0; i < g_numLights; ++i)
		//g_lights[i].init();
}


void ProcessLights()
{

	int sort[200];
	float dist[200];
	int obj[200];
	int temp;

	for (int i = 0; i < 10; i++)
	{
		//TODO: fix this hack
		m_PointLight[i].Position[0] = 9000;
		m_PointLight[i].Position[1] = 9000;
		m_PointLight[i].Position[2] = 9000;

		m_SpotLight[i].Position[0] = 9000;
		m_SpotLight[i].Position[1] = 9000;
		m_SpotLight[i].Position[2] = 9000;
	}



	int dcount = 0;
	//Find lights
	for (int q = 0; q < oblist_length; q++)
	{
		int ob_type = oblist[q].type;
		float	qdist = FastDistance(m_vEyePt.x - oblist[q].x,
			m_vEyePt.y - oblist[q].y,
			m_vEyePt.z - oblist[q].z);
		//if (ob_type == 57)
		if (ob_type == 6 && qdist < 2500 && oblist[q].light_source->command == 900)
		{
			dist[dcount] = qdist;
			sort[dcount] = dcount;
			obj[dcount] = q;
			dcount++;
		}

	}


	//sorting - ASCENDING ORDER
	for (int i = 0;i < dcount;i++)
	{
		for (int j = i + 1;j < dcount;j++)
		{
			if (dist[sort[i]] > dist[sort[j]])
			{
				temp = sort[i];
				sort[i] = sort[j];
				sort[j] = temp;
			}
		}
	}

	if (dcount > 8) {
		dcount = 8;
	}

	for (int i = 0;i < dcount;i++)
	{
		int q = obj[sort[i]];
		float dist2 = dist[sort[i]];

		int angle = (int)oblist[q].rot_angle;
		int ob_type = oblist[q].type;

		m_PointLight[i].Position[0] = oblist[q].x;
		m_PointLight[i].Position[1] = oblist[q].y + 20.0f;
		m_PointLight[i].Position[2] = oblist[q].z;

		m_PointLight[i].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_PointLight[i].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	int count = 0;



	//AddMissleLight(pd3dDevice);
	for (int misslecount = 0; misslecount < MAX_MISSLE; misslecount++)
	{
		if (your_missle[misslecount].active == 1)
		{
			if (count < 2) {
				m_PointLight[8 + count].Position[0] = your_missle[misslecount].x;
				m_PointLight[8 + count].Position[1] = your_missle[misslecount].y;
				m_PointLight[8 + count].Position[2] = your_missle[misslecount].z;

				if (your_missle[misslecount].model_id == 103) {
					m_PointLight[8 + count].Diffuse = { 0.0f, 1.0f, 0.0f, 1.0f };
					m_PointLight[8 + count].Specular = { 0.0f, 0.0f, 1.0f, 1.0f };
				}
				else if (your_missle[misslecount].model_id == 104) {
					m_PointLight[8 + count].Diffuse = { 0.0f, 0.0f, 1.0f, 1.0f };
					m_PointLight[8 + count].Specular = { 1.0f, 0.5f, 0.0f, 1.0f };
				}
				else if (your_missle[misslecount].model_id == 105) {
					m_PointLight[8 + count].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
					m_PointLight[8 + count].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
				}


				count++;
			}
		}
	}


	bool flamesword = false;

	if (strstr(your_gun[current_gun].gunname, "FLAME") != NULL ||
		strstr(your_gun[current_gun].gunname, "ICE") != NULL ||
		strstr(your_gun[current_gun].gunname, "LIGHTNINGSWORD") != NULL)
	{
		flamesword = true;
	}


	if (flamesword) {

		m_PointLight[7].Position[0] = m_vEyePt.x;
		m_PointLight[7].Position[1] = m_vEyePt.y;
		m_PointLight[7].Position[2] = m_vEyePt.z;
		m_PointLight[7].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_PointLight[7].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_PointLight[7].RangeAttenuation = { 200.0f, 0.0f, 0.0f, 0.0f };  //x = radius z=speculaor

		if (strstr(your_gun[current_gun].gunname, "FLAME") != NULL) {
			//m_PointLight[7].Diffuse = { 0.0f, 0.0f, 1.0f, 1.0f };
			//m_PointLight[7].Specular = { 1.0f, 0.0f, 0.0f, 1.0f };
			m_PointLight[7].Diffuse = { 0.0f, 0.0f, 1.0f, 1.0f };
			m_PointLight[7].Specular = { 1.0f, 0.5f, 0.0f, 1.0f };
		}
		else if (strstr(your_gun[current_gun].gunname, "ICE") != NULL) {
			m_PointLight[7].Diffuse = { 1.0f, 0.0f, 0.0f, 1.0f };
			m_PointLight[7].Specular = { 0.0f, 0.0f, 1.0f, 1.0f };
		}
		else if (strstr(your_gun[current_gun].gunname, "LIGHTNINGSWORD") != NULL) {
			m_PointLight[7].Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
			m_PointLight[7].Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

	}

	dcount = 0;

	//Find lights SPOT
	for (int q = 0; q < oblist_length; q++)
	{
		int ob_type = oblist[q].type;
		float	qdist = FastDistance(m_vEyePt.x - oblist[q].x,
			m_vEyePt.y - oblist[q].y,
			m_vEyePt.z - oblist[q].z);
		//if (ob_type == 6)
		if (ob_type == 6 && qdist < 2500 && oblist[q].light_source->command == 1)
		{
			dist[dcount] = qdist;
			sort[dcount] = dcount;
			obj[dcount] = q;
			dcount++;
		}

	}


	//sorting - ASCENDING ORDER
	for (int i = 0;i < dcount;i++)
	{
		for (int j = i + 1;j < dcount;j++)
		{
			if (dist[sort[i]] > dist[sort[j]])
			{
				temp = sort[i];
				sort[i] = sort[j];
				sort[j] = temp;
			}
		}
	}

	if (dcount > 8) {
		dcount = 8;
	}

	for (int i = 0;i < dcount;i++)
	{
		int q = obj[sort[i]];
		float dist2 = dist[sort[i]];

		int angle = (int)oblist[q].rot_angle;
		int ob_type = oblist[q].type;

		//AddWorldLight(ob_type, angle, q, pd3dDevice);

		m_SpotLight[i].Position[0] = oblist[q].x;
		m_SpotLight[i].Position[1] = oblist[q].y;
		m_SpotLight[i].Position[2] = oblist[q].z;
		m_SpotLight[i].Diffuse = { (float)oblist[q].light_source->rcolour, (float)oblist[q].light_source->gcolour, (float)oblist[q].light_source->bcolour, 1.0f };
		m_SpotLight[i].Direction = { oblist[q].light_source->direction_x, oblist[q].light_source->direction_y, oblist[q].light_source->direction_z, 0.0f };

	}

	if (!flamesword) {
		m_SpotLight[7].Position[0] = m_vEyePt.x;
		m_SpotLight[7].Position[1] = m_vEyePt.y + 400.0f;
		m_SpotLight[7].Position[2] = m_vEyePt.z;
		m_SpotLight[7].Diffuse = { 0.5f, 0.0f, 0.0f, 1.0f };
		m_SpotLight[7].Specular = { 0.0f, 0.0f, 1.0f, 1.0f };
	}


}




void display_font(float x, float y, char text[1000], int r, int g, int b)
{


	int perspectiveview = 1;
	float tuad = .0625f;
	float tvad = .0625f;

	float fontsize;
	int intTextLength = 0;
	int i = 0;
	float itrue = 0;
	float tu = 1.0f, tv = 1.0f;
	int textlen = 0;
	int textmid;
	char lefttext[500];
	char righttext[500];
	char newtext[500];

	char reversetext[500];
	float adjust = 0.0f;
	float adjust2 = 0.0f;

	int j = 2;

	int countl = 0;
	int countr = 0;
	int countreverse = 0;
	float xcol;
	xcol = x;
	textlen = strlen(text);
	textmid = textlen / 2;

	fontsize = 5;

	if (perspectiveview == 1)
		fontsize = 5;

	fontsize = 3;

	int flip = 0;

	for (i = 0; i < textlen; i++)
	{

		if (i < textmid)
		{

			lefttext[countl++] = text[i];
		}
		else
		{

			righttext[countr++] = text[i];
		}
	}

	lefttext[countl] = '\0';
	righttext[countr] = '\0';

	strcpy_s(newtext, righttext);
	countreverse = strlen(lefttext);

	for (i = 1; i <= (int)strlen(lefttext); i++)
	{

		reversetext[i - 1] = lefttext[countreverse - i];
	}

	reversetext[i - 1] = '\0';

	y = 40;

	//for (j = 0; j < 1; j++)

	for (j = 1; j >= 0; j--)
	{

		if (j == 1)
		{
			strcpy_s(newtext, reversetext);
			if (flip == 0)
				flip = 1;
		}

		if (j == 0)
		{
			//corect
			strcpy_s(newtext, righttext);
		}

		intTextLength = strlen(newtext);

		for (i = 0; i < intTextLength; i++)
		{

			switch (newtext[i])
			{

			case '0':

				tu = 1.0f;
				tv = 4.0f;

				break;
			case '1':

				tu = 2.0f;
				tv = 4.0f;

				break;
			case '2':

				tu = 3.0f;
				tv = 4.0f;

				break;
			case '3':

				tu = 4.0f;
				tv = 4.0f;
				break;

			case '4':

				tu = 5.0f;
				tv = 4.0f;

				break;
			case '5':

				tu = 6.0f;
				tv = 4.0f;

				break;
			case '6':

				tu = 7.0f;
				tv = 4.0f;

				break;
			case '7':

				tu = 8.0f;
				tv = 4.0f;
				break;
			case '8':

				tu = 9.0f;
				tv = 4.0f;

				break;
			case '9':

				tu = 10.0f;
				tv = 4.0f;

				break;
			case ':':

				tu = 11.0f;
				tv = 4.0f;

				break;
			case '.':

				tu = 15.0f;
				tv = 3.0f;

				break;
			case '+':

				tu = 12.0f;
				tv = 3.0f;

				break;
			case ',':

				tu = 13.0f;
				tv = 3.0f;

				break;

			case '-':

				tu = 14.0f;
				tv = 3.0f;

				break;
			case '/':

				tu = 16.0f;
				tv = 3.0f;

				break;

			case 'A':

				tu = 2.0f;
				tv = 5.0f;

				break;

			case 'B':

				tu = 3.0f;
				tv = 5.0f;

				break;

			case 'C':

				tu = 4.0f;
				tv = 5.0f;

				break;

			case 'D':

				tu = 5.0f;
				tv = 5.0f;

				break;

			case 'E':

				tu = 6.0f;
				tv = 5.0f;

				break;

			case 'F':

				tu = 7.0f;
				tv = 5.0f;

				break;

			case 'G':

				tu = 8.0f;
				tv = 5.0f;

				break;
			case 'H':

				tu = 9.0f;
				tv = 5.0f;

				break;
			case 'I':

				tu = 10.0f;
				tv = 5.0f;

				break;
			case 'J':

				tu = 11.0f;
				tv = 5.0f;

				break;

			case 'K':

				tu = 12.0f;
				tv = 5.0f;

				break;

			case 'L':

				tu = 13.0f;
				tv = 5.0f;

				break;

			case 'M':

				tu = 14.0f;
				tv = 5.0f;

				break;
			case 'N':

				tu = 15.0f;
				tv = 5.0f;

				break;

			case 'O':

				tu = 16.0f;
				tv = 5.0f;

				break;
			case 'P':

				tu = 1.0f;
				tv = 6.0f;

				break;
			case 'Q':

				tu = 2.0f;
				tv = 6.0f;

				break;
			case 'R':

				tu = 3.0f;
				tv = 6.0f;

				break;
			case 'S':

				tu = 4.0f;
				tv = 6.0f;

				break;
			case 'T':

				tu = 5.0f;
				tv = 6.0f;

				break;
			case 'U':

				tu = 6.0f;
				tv = 6.0f;

				break;
			case 'V':

				tu = 7.0f;
				tv = 6.0f;

				break;
			case 'W':

				tu = 8.0f;
				tv = 6.0f;

				break;
			case 'X':

				tu = 9.0f;
				tv = 6.0f;

				break;
			case 'Y':

				tu = 10.0f;
				tv = 6.0f;

				break;
			case 'Z':

				tu = 11.0f;
				tv = 6.0f;

				break;

			case 'a':

				tu = 2.0f;
				tv = 7.0f;

				break;

			case 'b':

				tu = 3.0f;
				tv = 7.0f;

				break;

			case 'c':

				tu = 4.0f;
				tv = 7.0f;

				break;

			case 'd':

				tu = 5.0f;
				tv = 7.0f;

				break;

			case 'e':

				tu = 6.0f;
				tv = 7.0f;

				break;

			case 'f':

				tu = 7.0f;
				tv = 7.0f;

				break;

			case 'g':

				tu = 8.0f;
				tv = 7.0f;

				break;
			case 'h':

				tu = 9.0f;
				tv = 7.0f;

				break;
			case 'i':

				tu = 10.0f;
				tv = 7.0f;

				break;
			case 'j':

				tu = 11.0f;
				tv = 7.0f;

				break;

			case 'k':

				tu = 12.0f;
				tv = 7.0f;

				break;

			case 'l':

				tu = 13.0f;
				tv = 7.0f;

				break;

			case 'm':

				tu = 14.0f;
				tv = 7.0f;

				break;
			case 'n':

				tu = 15.0f;
				tv = 7.0f;

				break;

			case 'o':

				tu = 16.0f;
				tv = 7.0f;

				break;
			case 'p':

				tu = 1.0f;
				tv = 8.0f;

				break;
			case 'q':

				tu = 2.0f;
				tv = 8.0f;

				break;
			case 'r':

				tu = 3.0f;
				tv = 8.0f;

				break;
			case 's':

				tu = 4.0f;
				tv = 8.0f;

				break;
			case 't':

				tu = 5.0f;
				tv = 8.0f;

				break;
			case 'u':

				tu = 6.0f;
				tv = 8.0f;

				break;
			case 'v':

				tu = 7.0f;
				tv = 8.0f;

				break;
			case 'w':

				tu = 8.0f;
				tv = 8.0f;

				break;
			case 'x':

				tu = 9.0f;
				tv = 8.0f;

				break;
			case 'y':

				tu = 10.0f;
				tv = 8.0f;

				break;
			case 'z':

				tu = 11.0f;
				tv = 8.0f;

				break;

			case ' ':

				tu = 1.0f;
				tv = 3.0f;
				break;
			case '|':

				tu = 1.0f;
				tv = 1.0f;
				break;
			case '`':

				tu = 2.0f;
				tv = 1.0f;
				break;
			default:
				tu = 2.0f;
				tv = 9.0f;

				break;
			}

			if (j == 0)
				itrue = (float)-i - 1;
			else
				itrue = (float)i;

			if (flip == 1)
			{
				flip = 2;
				adjust = 2.0f;
			}

			float amount = 1.5f;

			if (j == 0)
			{
				adjust += amount;
			}
			else
			{
				adjust -= amount;
			}

			long currentcolour = 0;
			currentcolour = RGBA_MAKE(255, 255, 255, 0);

			RGBA_MAKE(0, 0, 0, 0);


			adjust = 0;

			//m_BackgroundMesh[countdisplay] = D3DVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, tuad * tu, tvad * (tv - 1.0f));

			m_BackgroundMesh[countdisplay].sx = (x + (itrue * fontsize)) + adjust;
			m_BackgroundMesh[countdisplay].sy = y;

			bubble[countdisplay].x = m_BackgroundMesh[countdisplay].sx;
			bubble[countdisplay].y = m_BackgroundMesh[countdisplay].sy;
			bubble[countdisplay].tu = tuad * tu;
			bubble[countdisplay].tv = tvad * (tv - 1.0f);

			bubble[countdisplay].z = 0;

			countdisplay++;
			//m_BackgroundMesh[countdisplay] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, tuad * tu, tvad * tv);

			m_BackgroundMesh[countdisplay].sx = (x + (itrue * fontsize)) + adjust;
			m_BackgroundMesh[countdisplay].sy = y - fontsize;

			bubble[countdisplay].x = m_BackgroundMesh[countdisplay].sx;
			bubble[countdisplay].y = m_BackgroundMesh[countdisplay].sy;
			bubble[countdisplay].tu = tuad * tu;
			bubble[countdisplay].tv = tvad * tv;
			bubble[countdisplay].z = 0;

			countdisplay++;

			//m_BackgroundMesh[countdisplay] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, tuad * (tu - 1.0f), tvad * (tv - 1.0f));
			m_BackgroundMesh[countdisplay].sx = (x + fontsize + (itrue * fontsize)) + adjust;
			m_BackgroundMesh[countdisplay].sy = y;

			bubble[countdisplay].x = m_BackgroundMesh[countdisplay].sx;
			bubble[countdisplay].y = m_BackgroundMesh[countdisplay].sy;
			bubble[countdisplay].tu = tuad * (tu - 1.0f);
			bubble[countdisplay].tv = tvad * (tv - 1.0f);
			bubble[countdisplay].z = 0;

			countdisplay++;

			//m_BackgroundMesh[countdisplay] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, tuad * (tu - 1.0f), tvad * tv);
			m_BackgroundMesh[countdisplay].sx = (x + fontsize + (itrue * fontsize)) + adjust;
			m_BackgroundMesh[countdisplay].sy = y - fontsize;

			bubble[countdisplay].x = m_BackgroundMesh[countdisplay].sx;
			bubble[countdisplay].y = m_BackgroundMesh[countdisplay].sy;
			bubble[countdisplay].tu = tuad * (tu - 1.0f);
			bubble[countdisplay].tv = tvad * tv;
			bubble[countdisplay].z = 0;

			countdisplay++;
		}
	}
}


void DisplayPlayerCaption(IDirect3DDevice9* pd3dDevice) {

	int i;
	LPDIRECTDRAWSURFACE7 lpDDsurface;
	float x, y, z;

	float pangle = 0;

	int countit = 0;
	int cullloop = 0;
	int cullflag = 0;
	int num = 0;
	int len = 0;
	int count = 0;
	char junk2[2000];
	int flag = 1;
	float yadjust = 0;

	D3DXMATRIX matWorld, matProj;

	//if (showtexture == 0)
		//return;

	D3DXMATRIX matRotate;
	int j = 0;

	//if (menuflares == 1)
	//{
	//	if (lastmaterial == 0)
	//	{
	//		D3DMATERIAL7 mtrl;
	//		D3DUtil_InitMaterial(mtrl, 1.0f, 1.0f, 1.0f, 1.0f);
	//		mtrl.emissive.r = 1.0f;
	//		mtrl.emissive.g = 1.0f;
	//		mtrl.emissive.b = 1.0f;
	//		lastmaterial = 1;
	//		m_pd3dDevice->SetMaterial(&mtrl);
	//	}
	//}

	//pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	//pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x01);
	//pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	//pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	int bground = FindTextureAlias("fontA");
	//int bground = FindTextureAlias("die10s2");
	int texture_number = TexMap[bground].texture;

	pd3dDevice->SetTexture(0, g_pTextureList[texture_number]); //set texture

	//lpDDsurface = lpddsImagePtr[texture_number];

	//if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
	//	PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

	//D3DVIEWPORT7 vp;
	//m_pd3dDevice->GetViewport(&vp);

	for (j = 0; j < num_monsters; j++)
	{

		cullflag = 0;
		for (cullloop = 0; cullloop < monstercount; cullloop++)
		{
			if (monstercull[cullloop] == monster_list[j].monsterid)
			{
				cullflag = 1;
				break;
			}
		}

		flag = 1;
		num = 0;
		count = 0;
		yadjust = 0.0f;

		if (monster_list[j].bIsPlayerValid && cullflag == 1 && monster_list[j].bStopAnimating == FALSE)
		{

			len = strlen(monster_list[j].chatstr);

			if (len > 0)
				len--;

			while (flag)
			{
				count = 0;

				while (monster_list[j].chatstr[num] != '!')
				{

					junk2[count] = monster_list[j].chatstr[num];
					count++;
					num++;
					if (num >= len)
						break;
				}
				if (monster_list[j].chatstr[num] == '!')
					num++;

				junk2[count] = '\0';

				if (num >= len || len == 0)
					flag = 0;

				float x = monster_list[j].x;
				float y = monster_list[j].y + 28.0f - yadjust;
				float z = monster_list[j].z;

				yadjust += 6.0f;

				countdisplay = 0;
				display_font(0.0f, 0.0f, junk2, 255, 255, 0);

				D3DUtil_SetTranslateMatrix(matWorld, D3DXVECTOR3(x, y, z));
				D3DXMatrixRotationY(&matRotate, (angy * k + (int)3.14));
				D3DXMatrixMultiply(&matWorld, &matRotate, &matWorld);

				pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

				g_pVB->Lock(0, sizeof(&pMonsterCaption), (void**)&pMonsterCaption, 0);
				for (i = 0; i < ((countdisplay)); i += 1)
				{
					D3DXVECTOR3 a = D3DXVECTOR3(bubble[i].x, bubble[i].y, bubble[i].z);
					pMonsterCaption[i].position = a;
					pMonsterCaption[i].color = D3DCOLOR_RGBA(105, 105, 105, 0); //0xffffffff;
					pMonsterCaption[i].tu = bubble[i].tu;
					pMonsterCaption[i].tv = bubble[i].tv;
				}

				g_pVB->Unlock();

				for (int i = 0; i < countdisplay; i = i + 4)
				{
					pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i, 2);
				}
			}
		}

	}

	//pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}
