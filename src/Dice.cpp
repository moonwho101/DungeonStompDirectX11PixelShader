#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"
#include "d3dtypes.h"
#include "LoadWorld.hpp"
#include "world.hpp"
#include "GlobalSettings.hpp"
#include <string.h>
#include "GameLogic.hpp"
#include "Missle.hpp"
#include "col_local.h"
#include "XAudio2Versions.h"
#include "dice.hpp"


extern ID3DXSprite* g_pSprite9;
extern ID3DXSprite* g_pDice9;
extern ID3DXSprite* g_pHud9;
struct diceroll dice[50];
int numdice;
extern LPDIRECT3DTEXTURE9      g_pTextureList[MAX_NUM_TEXTURESLIST];
D3DVERTEX MakeVertex(float x, float y, float z);
void SetMaterialBright(IDirect3DDevice9* pd3dDevice);
int showsavingthrow = 0;
extern int d20roll;
extern int damageroll;

D3DVERTEX MakeVertex(float x, float y, float z) {

	D3DVERTEX d;

	d.x = x;
	d.y = y;
	d.z = z;

	return d;
}

int MakeDice()
{
	float x1, x2, y1, y2;
	float scale = 16.0f;
	float adjust = 100.0f;

	int viewportwidth = wWidth;
	int viewportheight = wHeight;

	x1 = 70.0f + scale;
	x2 = 10.0f;

	y1 = 110.0f + scale;
	y2 = 50.0f;

	dice[0].dicebox[0] = MakeVertex(0, 0, 1);
	dice[0].dicebox[1] = MakeVertex(0, 0, 0);
	dice[0].dicebox[2] = MakeVertex(0, 1, 1);
	dice[0].dicebox[3] = MakeVertex(0, 1, 0);

	dice[0].dicebox[1].x = (viewportwidth / 2) - x1;
	dice[0].dicebox[3].x = (viewportwidth / 2) - x2;
	dice[0].dicebox[0].x = (viewportwidth / 2) - x1;
	dice[0].dicebox[2].x = (viewportwidth / 2) - x2;

	dice[0].dicebox[1].y = viewportheight - y1;
	dice[0].dicebox[3].y = viewportheight - y1;
	dice[0].dicebox[0].y = viewportheight - y2;
	dice[0].dicebox[2].y = viewportheight - y2;
	dice[0].sides = 20;
	strcpy_s(dice[0].name, "die20s20");
	strcpy_s(dice[0].prefix, "die20");
	dice[0].rollnum = 8;
	dice[0].roll = 0;
	dice[0].rollmax = 5;

	dice[1].dicebox[0] = MakeVertex(0, 0, 1);
	dice[1].dicebox[1] = MakeVertex(0, 0, 0);
	dice[1].dicebox[2] = MakeVertex(0, 1, 1);
	dice[1].dicebox[3] = MakeVertex(0, 1, 0);

	dice[1].dicebox[1].x = (viewportwidth / 2) + x2;
	dice[1].dicebox[3].x = (viewportwidth / 2) + x1;
	dice[1].dicebox[0].x = (viewportwidth / 2) + x2;
	dice[1].dicebox[2].x = (viewportwidth / 2) + x1;

	dice[1].dicebox[1].y = viewportheight - y1;
	dice[1].dicebox[3].y = viewportheight - y1;
	dice[1].dicebox[0].y = viewportheight - y2;
	dice[1].dicebox[2].y = viewportheight - y2;

	strcpy_s(dice[1].name, "die4s4");
	strcpy_s(dice[1].prefix, "die4");
	dice[1].rollnum = 4;
	dice[1].roll = 0;
	dice[1].sides = 4;
	dice[1].rollmax = 3;

	dice[2].dicebox[0] = MakeVertex(0, 0, 1);
	dice[2].dicebox[1] = MakeVertex(0, 0, 0);
	dice[2].dicebox[2] = MakeVertex(0, 1, 1);
	dice[2].dicebox[3] = MakeVertex(0, 1, 0);

	dice[2].dicebox[1].x = (viewportwidth / 2) + adjust + x2;
	dice[2].dicebox[3].x = (viewportwidth / 2) + adjust + x1;
	dice[2].dicebox[0].x = (viewportwidth / 2) + adjust + x2;
	dice[2].dicebox[2].x = (viewportwidth / 2) + adjust + x1;

	dice[2].dicebox[1].y = viewportheight - y1;
	dice[2].dicebox[3].y = viewportheight - y1;
	dice[2].dicebox[0].y = viewportheight - y2;
	dice[2].dicebox[2].y = viewportheight - y2;

	strcpy_s(dice[2].name, "die6s6");
	strcpy_s(dice[2].prefix, "die6");
	dice[2].rollnum = 6;
	dice[2].roll = 0;
	dice[2].sides = 6;
	dice[2].rollmax = 4;

	dice[3].dicebox[0] = MakeVertex(0, 0, 1);
	dice[3].dicebox[1] = MakeVertex(0, 0, 0);
	dice[3].dicebox[2] = MakeVertex(0, 1, 1);
	dice[3].dicebox[3] = MakeVertex(0, 1, 0);

	dice[3].dicebox[1].x = (viewportwidth / 2) + (adjust * 2.0f) + x2;
	dice[3].dicebox[3].x = (viewportwidth / 2) + (adjust * 2.0f) + x1;
	dice[3].dicebox[0].x = (viewportwidth / 2) + (adjust * 2.0f) + x2;
	dice[3].dicebox[2].x = (viewportwidth / 2) + (adjust * 2.0f) + x1;

	dice[3].dicebox[1].y = viewportheight - y1;
	dice[3].dicebox[3].y = viewportheight - y1;
	dice[3].dicebox[0].y = viewportheight - y2;
	dice[3].dicebox[2].y = viewportheight - y2;
	dice[3].sides = 20;
	strcpy_s(dice[3].name, "die20s20");
	strcpy_s(dice[3].prefix, "die20");
	dice[3].rollnum = 8;
	dice[3].roll = 0;
	dice[3].rollmax = 5;

	x1 = 70.0f + scale;
	x2 = 10.0f;
	y1 = 100.0f + scale;
	y2 = 40.0f;

	//crosshair[0] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, 0, 1);
	//crosshair[1] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, 0, 0);
	//crosshair[2] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, 1, 1);
	//crosshair[3] = D3DTLVERTEX(D3DVECTOR(0, 0, 0.99f), 0.5f, -1, 0, 1, 0);

	//int offset = 5;

	//crosshair[0].sx = (viewportwidth / 2) - offset;
	//crosshair[1].sx = (viewportwidth / 2) - offset;
	//crosshair[2].sx = (viewportwidth / 2) + offset;
	//crosshair[3].sx = (viewportwidth / 2) + offset;

	//crosshair[0].sy = (viewportheight / 2) + offset;
	//crosshair[1].sy = (viewportheight / 2) - offset;
	//crosshair[2].sy = (viewportheight / 2) + offset;
	//crosshair[3].sy = (viewportheight / 2) - offset;

	return 1;
}

void MakeDamageDice()
{

	float x1, x2, y1, y2;
	float scale = 16.0f;

	x1 = 70.0f + scale;
	x2 = 10.0f;
	y1 = 110.0f + scale;
	y2 = 50.0f;

	int a;

	int gunmodel = 0;
	for (a = 0; a < num_your_guns; a++)
	{
		if (your_gun[a].model_id == player_list[trueplayernum].gunid)
		{
			gunmodel = a;
		}
	}

	if (your_gun[gunmodel].damage2 == 4)
	{
		strcpy_s(dice[1].name, "die4s4");
		strcpy_s(dice[1].prefix, "die4");
		dice[1].rollnum = 4;
		dice[1].roll = 0;
		dice[1].sides = 4;
		dice[1].rollmax = 3;
	}
	else if (your_gun[gunmodel].damage2 == 6)
	{
		strcpy_s(dice[1].name, "die6s6");
		strcpy_s(dice[1].prefix, "die6");
		dice[1].rollnum = 6;
		dice[1].roll = 0;
		dice[1].sides = 6;
		dice[1].rollmax = 4;
	}
	else if (your_gun[gunmodel].damage2 == 8)
	{
		strcpy_s(dice[1].name, "die8s8");
		strcpy_s(dice[1].prefix, "die8");
		dice[1].rollnum = 8;
		dice[1].roll = 0;
		dice[1].sides = 8;
		dice[1].rollmax = 5;
	}
	else if (your_gun[gunmodel].damage2 == 10)
	{

		strcpy_s(dice[1].name, "die10s10");
		strcpy_s(dice[1].prefix, "die10");
		dice[1].rollnum = 10;
		dice[1].roll = 0;
		dice[1].sides = 10;
		dice[1].rollmax = 5;
	}
	else if (your_gun[gunmodel].damage2 == 12)
	{

		strcpy_s(dice[1].name, "die12s12");
		strcpy_s(dice[1].prefix, "die12");
		dice[1].rollnum = 12;
		dice[1].roll = 0;
		dice[1].sides = 12;
		dice[1].rollmax = 4;
	}


	dice[1].dicebox[0] = MakeVertex(0, 0, 1);
	dice[1].dicebox[1] = MakeVertex(0, 0, 0);
	dice[1].dicebox[2] = MakeVertex(0, 1, 1);
	dice[1].dicebox[3] = MakeVertex(0, 1, 0);

	dice[1].dicebox[1].x = (wWidth / 2) + x2;
	dice[1].dicebox[3].x = (wWidth / 2) + x1;
	dice[1].dicebox[0].x = (wWidth / 2) + x2;
	dice[1].dicebox[2].x = (wWidth / 2) + x1;

	dice[1].dicebox[1].y = wHeight - y1;
	dice[1].dicebox[3].y = wHeight - y1;
	dice[1].dicebox[0].y = wHeight - y2;
	dice[1].dicebox[2].y = wHeight - y2;

}



extern int usespell;
//extern int spellhiton;
extern int hitmonster;
extern int savefailed;
extern int criticalhiton;
int spellhiton = 0;

void SetDiceTexture(bool showroll)
{
	for (int i = 0; i < numdice; i++)
	{

		if (dice[i].roll == 1)
		{
			//roll the die

			if (maingameloop)
				dice[i].rollnum++;
			if (dice[i].rollnum > dice[i].rollmax)
			{

				dice[i].rollnum = 1;
			}
			sprintf_s(dice[i].name, "%sr%d", dice[i].prefix, dice[i].rollnum);
		}
	}



	
	char junk[255];

	if (usespell == 1)
	{
		spellhiton = player_list[trueplayernum].hd;
	}
	else
	{
		spellhiton = 0;
	}

	if (hitmonster == 1 || usespell == 1)
	{

		if (strstr(your_gun[current_gun].gunname, "SCROLL-HEALING") != NULL)
		{
			strcpy_s(junk, "Attack      Heal");
		}
		else
		{
			strcpy_s(junk, "Attack      Damage");
		}
	}
	else
	{
		strcpy_s(junk, "Attack");
	}

	if (numdice == 3 && hitmonster == 1)
	{
		
	}
	else if (numdice == 3 && hitmonster == 0)
	{
		strcpy_s(junk, "Attack");
	}

	if (showsavingthrow > 0)
	{
		display_message((wWidth / 2) + 222.0f - 90.0f, wHeight - 30.0f, "Save", 255, 255, 0, 12.5, 16, 0);
		if (savefailed == 1)
			display_message((wWidth / 2) + 212.0f - 90.0f, wHeight - 15.0f, "Failed", 255, 255, 0, 12.5, 16, 0);
	}
	//if (showlisten > 0)
	//{

	//	display_message((wWidth / 2) + 111.0f, wHeight - 30.0f, "Listen", vp, 255, 255, 0, 12.5, 16, 0);
	//	if (listenfailed == 1)
	//		display_message((wWidth / 2) + 111.0f, wHeight - 15.0f, "Failed", vp, 255, 255, 0, 12.5, 16, 0);
	//}

	display_message((wWidth / 2) - 32.0f, wHeight - 30.0f, junk, 255, 255, 0, 12.5, 16, 0);

	if (criticalhiton == 1)
	{
		strcpy_s(junk, "Critical Hit  X 2");
		display_message((wWidth / 2) - 30.0f, wHeight - 145.0f, junk, 255, 255, 0, 12.5, 16, 0);
	}
	if (spellhiton > 1)
	{
		sprintf_s(junk, "           X %d", spellhiton);
		display_message((wWidth / 2) - 30.0f, wHeight - 130.0f, junk, 0, 255, 0, 12.5, 16, 0);
	}
	else
	{
		int gunmodel = 0;
		for (int a = 0; a < num_your_guns; a++)
		{

			if (your_gun[a].model_id == player_list[trueplayernum].gunid)
			{

				gunmodel = a;
			}
		}

		int attackbonus = your_gun[gunmodel].sattack;
		int damagebonus = your_gun[gunmodel].sdamage;

		if (attackbonus > 0 || damagebonus > 0)
		{
			sprintf_s(junk, "   +%d      +%d", attackbonus, damagebonus);
			display_message((wWidth / 2) - 30.0f, wHeight - 130.0f, junk, 0, 255, 0, 12.5, 16, 0);
		}
	}


	if (showroll) {
		sprintf_s(junk, "%d", d20roll);
		display_message((wWidth / 2) - 15.0f, wHeight - 60.0f, junk, 0, 255, 0, 12.5, 16, 0);

		sprintf_s(junk, "%d", damageroll);
		display_message((wWidth / 2) + 70.0f, wHeight - 60.0f, junk, 0, 255, 0, 12.5, 16, 0);
	}




}

int thaco(int ac, int thaco)
{

	int result;

	result = thaco - ac;

	return result;
}





void ShowScreenOverlays(IDirect3DDevice9* pd3dDevice)
{

	int showpanel = 1;
	int showlisten = 0;
	

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	SetMaterialBright(pd3dDevice);


	if (showpanel == 1)
	{
		for (int i = 0; i < 4; i++)
		{

			int sdice = 0;

			if (maingameloop)
			{
				if (showlisten > 0)
				{
					showlisten++;
					if (showlisten > 240)
						showlisten = 0;
				}

				if (showsavingthrow > 0)
				{
					showsavingthrow++;
					if (showsavingthrow > 240)
						showsavingthrow = 0;
				}
			}

			if (i == 0 || i == 1)
				sdice = 1;

			if (showlisten >= 1 && i == 2)
				sdice = 1;
			if (showsavingthrow >= 1 && i == 3)
				sdice = 1;

			if (sdice == 1)
			{

				D3DXVECTOR3	g_position = { dice[0].dicebox[i].x, dice[0].dicebox[i].y, dice[0].dicebox[i].z };

				RECT srcRect = { 0,0,wWidth,wHeight };

				D3DXVECTOR3 pos((float)(wWidth - 80.0f) / 2, (float)(wHeight - 100.0f), 0);
				D3DXVECTOR3 pos2((float)(wWidth + 80.0f) / 2, (float)(wHeight - 100.0f), 0);
				D3DXVECTOR3 pos3((float)(wWidth + 160.0f) / 2, (float)(wHeight - 100.0f), 0);
				D3DXVECTOR3 pos4((float)(wWidth + 160.0f + 80.0f) / 2, (float)(wHeight - 100.0f), 0);
				D3DXVECTOR3 pos5(0, 0, 0);
				D3DXVECTOR3 final;
				
					g_pDice9->Begin(NULL);
					{

						int bground = FindTextureAlias(dice[i].name);
						int texture_number = TexMap[bground].texture;
						//pd3dDevice->SetTexture(0, g_pTextureList[texture_number]); //set texture

						if (i == 0) {
							final = pos;
						}
						else if (i == 1) {
							final = pos2;
						}
						else if (i == 2) {
							final = pos3;
						}
						else if (i == 3) {
							final = pos4;
						}
						D3DXMATRIX matrix;
						//D3DXMATRIX final_matrix;
						//D3DXMATRIX scale;

						//final = pos5;
						//final = final / 12.0f;

						//D3DXMatrixScaling(&scale, 4.0f, 4.0f, 4.0f);
						//D3DXMatrixTranslation(&matrix, final.x, final.y, 0);
						//D3DXMatrixMultiply(&final_matrix,  &scaling_matrix, &matrix);

						//g_pDice9->SetTransform(&scaling_matrix);

						//D3DXVECTOR2	scale = { 3.0f,3.0f };

						//final.x = 50.0f;
						//final.y = 50.0f ;
						//final.z = 0;

						//g_pDice9->Draw(g_pTextureList[texture_number], NULL, NULL, NULL, D3DCOLOR_RGBA(105, 105, 105, 0));

						//D3DXVECTOR2 center2D = D3DXVECTOR2(32.0f, 32.0f);
						//D3DXMatrixTransformation2D(&matrix, &center2D, NULL, &scale, &center2D, NULL, new D3DXVECTOR2(0, 0));
						//g_pDice9->SetTransform(&matrix);
						g_pDice9->Draw(g_pTextureList[texture_number], NULL, NULL, &final, D3DCOLOR_RGBA(105, 105, 105, 0));


						//g_pDice9->Draw(g_pTextureList[texture_number], NULL, NULL, &final, D3DCOLOR_RGBA(105, 105, 105, 0));

					}
					g_pDice9->End();




				//lpDDsurface = lpddsImagePtr[texture_number];

				//if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
					//PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

				//if (zbufferenable == 1)
					//m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

				//g_pVB->Lock(0, sizeof(&pBoundingBox), (void**)&pBoundingBox, 0);

				//int i = 0;
				//for (int i = 0; i < 3; i++)
				//{
					//D3DXVECTOR3 a = D3DXVECTOR3(dice[i].dicebox[0].x, dice[i].dicebox[0].y, dice[i].dicebox[0].z);
					//D3DXVECTOR3 a = D3DXVECTOR3(dice[0].dicebox[i].x, dice[0].dicebox[i].y, dice[0].dicebox[i].z);
					//pBoundingBox[i].position = a;
					//pBoundingBox[i].color = D3DCOLOR_RGBA(105, 105, 105, 0); //0xffffffff;
				//}
				//g_pVB->Unlock();


				//pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 1);

				//pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX,(LPVOID)&dice[i].dicebox, 4, 0);

				//if (zbufferenable == 1)
					//m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
			}
		}




		g_pHud9->Begin(NULL);
		{

			
			D3DXMATRIX matrix;
			//D3DXMATRIX final_matrix;
			//D3DXMATRIX scale;

			//final = pos5;
			//final = final / 12.0f;

			//D3DXMatrixScaling(&scale, 4.0f, 4.0f, 4.0f);
			//D3DXMatrixTranslation(&matrix, final.x, final.y, 0);
			//D3DXMatrixMultiply(&final_matrix,  &scaling_matrix, &matrix);

			//g_pDice9->SetTransform(&scaling_matrix);

			//D3DXVECTOR2	scale = { 3.0f,3.0f };
			D3DXVECTOR3	pos = { 0.0f,(float)(wHeight - 200.0f),0.0f };

			//final.x = 50.0f;
			//final.y = 50.0f ;
			//final.z = 0;

			//g_pDice9->Draw(g_pTextureList[texture_number], NULL, NULL, NULL, D3DCOLOR_RGBA(105, 105, 105, 0));

			//D3DXVECTOR2 center2D = D3DXVECTOR2(32.0f, 32.0f);
			//D3DXMatrixTransformation2D(&matrix, &center2D, NULL, &scale, &center2D, NULL, new D3DXVECTOR2(0, 0));
			//g_pDice9->SetTransform(&matrix);
			g_pHud9->Draw(g_pTextureList[355], NULL, NULL, &pos, D3DCOLOR_RGBA(105, 105, 105, 0));


			//g_pDice9->Draw(g_pTextureList[texture_number], NULL, NULL, &final, D3DCOLOR_RGBA(105, 105, 105, 0));

		}
		g_pHud9->End();


	}

	//if (filtertype > 0)
	//{
	//	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
	//	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
	//}


	/*
	int bground = pCMyApp->FindTextureAlias("fontA");
	texture_number = TexMap[bground].texture;
	lpDDsurface = lpddsImagePtr[texture_number];

	if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
		PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

	if (zbufferenable == 1)
		m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

	if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
		PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

	int lastfont = 0;

	for (int i = 0; i < ((countmessage / 4)); i += 1)
	{
		if (m_DisplayMessageFont[i * 4] != lastfont)
		{
			lastfont = m_DisplayMessageFont[i * 4];
			if (lastfont == 0)
			{
				bground = pCMyApp->FindTextureAlias("fontA");
				texture_number = TexMap[bground].texture;
				lpDDsurface = lpddsImagePtr[texture_number];
				if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
					PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

				if (bEnableAlphaTransparency)
				{
					m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				}
			}
			else if (lastfont == 1)
			{
				bground = pCMyApp->FindTextureAlias("box2");
				texture_number = TexMap[bground].texture;
				lpDDsurface = lpddsImagePtr[texture_number];

				if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
					PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);

				if (bEnableAlphaTransparency)
				{
					m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

					m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,
						D3DBLEND_SRCCOLOR);

					m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,
						D3DBLEND_INVSRCCOLOR);
				}
			}
			else if (lastfont == 2)
			{
				bground = pCMyApp->FindTextureAlias("box1");
				texture_number = TexMap[bground].texture;
				lpDDsurface = lpddsImagePtr[texture_number];

				if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
					PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);
			}
			else if (lastfont == 3)
			{
				bground = pCMyApp->FindTextureAlias("dungeonmaster");
				texture_number = TexMap[bground].texture;
				lpDDsurface = lpddsImagePtr[texture_number];

				if (m_pd3dDevice->SetTexture(0, lpDDsurface) != DD_OK)
					PrintMessage(NULL, "SetTexture FAILED", NULL, LOGFILE_ONLY);
			}
		}

		for (int q = 0; q < ((countmessage)); q += 1)
		{
			m_DisplayMessage[q].rhw = 1.0f;
		}

		m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX,
			(LPVOID)&m_DisplayMessage[i * (int)4], 4, 0);
	}

	*/

}
