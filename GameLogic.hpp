#ifndef __GAMELOGIC_H
#define __GAMELOGIC_H



typedef struct doors
{
	int doornum;
	float angle;
	int swing;
	int key;
	int open;
	float saveangle;
	int type;
	int listen;
	int y;
	float up;

} DOORS;

typedef struct scrolllisting
{

	int num;
	float angle;
	char text[2048];
	int r;
	int g;
	int b;

} SCROLLLISTING;

extern int maingameloop;
extern int maingameloop2;
extern int doorcounter;
extern DOORS door[200];

extern SCROLLLISTING scrolllist1[50];
extern int slistcounter;
extern int sliststart;
extern int scrolllistnum;
extern int scount;
int pnpoly(int npol, float* xp, float* yp, float x, float y);
int SceneInBox(D3DVECTOR point);
int CalculateView(D3DVECTOR EyeBall, D3DVECTOR LookPoint, float angle, bool distancecheck);
int CalculateViewMonster(D3DVECTOR EyeBall, D3DVECTOR LookPoint, float angle, float angy);
void PlayerNonIndexedBox(int pmodel_id, int curr_frame, int angle, float wx, float wy, float wz);
void PlayerIndexedBox(int pmodel_id, int curr_frame, int angle, float wx, float wy, float wz);
void MakeBoundingBox();
int FindGunTexture(char* p);
void PlayerToD3DVertList(int pmodel_id, int curr_frame, int angle, int texture_alias, int tex_flag, float xt, float yt, float zt);
int FindModelID(char* p);
void AddTreasure(float x, float y, float z, int gold);
void SetMonsterAnimationSequence(int player_number, int sequence_number);
void OpenChest();
int UpdateScrollList(int r, int g, int b);
void ApplyPlayerDamage(int playerid, int damage);
int DisplayDialogText(char* text, float yloc);
int DisplayDamage(float x, float y, float z, int owner, int id, bool criticalhit);
int XpPoints(int hd, int hp);
int LevelUp(int xp);
int LevelUpXPNeeded(int xp);

D3DXVECTOR3 collideWithWorld(D3DXVECTOR3 position, D3DXVECTOR3 velocity);
D3DXVECTOR3 RadiusMultiply(D3DXVECTOR3 vector, D3DXVECTOR3 eRadius);
D3DXVECTOR3 RadiusDivide(D3DXVECTOR3 vector, D3DXVECTOR3 eRadius);

#endif