
#ifndef __PROCESSMODEL_H
#define __PROCESSMODEL_H

#include <stdio.h>
#include <io.h>
#include <wtypes.h>




//#define D3DFVF_ULTVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0))

#ifdef SHOW_HOW_TO_USE_TCI
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
#else
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_NORMAL)
#endif

extern int monstercull[1000];
extern int monstercount;
extern float fDot2;
extern int weapondrop;
extern int obdata_length;
extern int obdata_length;
extern int oblist_length;
extern int* num_vert_per_object;
extern int num_polys_per_object[500];
extern int num_triangles_in_scene;
extern int num_verts_in_scene;
extern int num_dp_commands_in_scene;
extern int cnt;


void PlayerToD3DIndexedVertList(int pmodel_id, int curr_frame, int angle, int texture_alias, int tex_flag, float xt, float yt, float zt);

int FindModelID(char* p);
void AddModel(float x, float y, float z, float rot_angle, float monsterid, float monstertexture, float monnum, char modelid[80], char modeltexture[80], int ability);
int FindGunTexture(char* p);
int CycleBitMap(int i);
int CalculateView(D3DVECTOR EyeBall, D3DVECTOR LookPoint, float angle);
void ObjectToD3DVertList(int ob_type, int angle, int oblist_index);
void AddMissleLight(IDirect3DDevice9* pd3dDevice);

#endif 