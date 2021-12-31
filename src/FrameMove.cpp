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
#include "DirectInput.hpp"
#include "GameLogic.hpp"
#include "Missle.hpp"
#include "FrameMove.hpp"

extern int gravityon;

HRESULT FrameMove(double fTime, FLOAT fTimeKey)
{

	float r = 15.0f;
	float pos_x, pos_y, pos_z;
	float dir_x, dir_y, dir_z;
	float lx, ly, lz;
	float radius = 20.0f; // used for flashlight
	float gun_angle;
	float step_left_angy;
	int i;


	D3DVECTOR MissleMove;
	D3DVECTOR MissleVelocity;

	FLOAT tu_left;
	FLOAT tu_right;
	int misslespot = 0;

	//flarestart == 1 && 
	if (maingameloop3) {
		//CycleFlare();
		AnimateCharacters();

	}

	gametimer3 = DSTimer();

	if ((gametimer3 - gametimerlast3) * time_factor >= 110.0f / 1000.0f)
	{
		//Animation Cycle
		maingameloop3 = 1;
		gametimerlast3 = DSTimer();
	}
	else
	{
		maingameloop3 = 0;
	}

	gametimer2 = DSTimer();

	if ((gametimer2 - gametimerlast2) * time_factor >= 60.0f / 1000.0f)
	{
		//Torch & Teleport Cycle
		maingameloop2 = 1;
		gametimerlast2 = DSTimer();
	}
	else
	{
		maingameloop2 = 0;
	}

	gametimer = DSTimer();

	if ((gametimer - gametimerlast) * time_factor >= 40.0f / 1000)
	{
		//Rotation coins, keys, diamonds
		maingameloop = 1;
		gametimerlast = DSTimer();
	}
	else
	{
		maingameloop = 0;
	}


	fTimeKeysave = fTimeKey;

	elapsegametimersave = fTimeKey;

	// check that angy is between 0 and 360 degrees
	float cameradist = 50.0f;

	//UpdateMainPlayer();

	if (angy >= 360)
		angy = angy - 360;

	if (angy < 0)
		angy += 360;

	GetItem();
	MonsterHit();


	//Find lights
	for (int q = 0; q < oblist_length; q++)
	{
		if (strstr(oblist[q].name, "door") != NULL)
		{
			//door

			float qdist = FastDistance(
				m_vLookatPt.x - oblist[q].x,
				m_vLookatPt.y - oblist[q].y,
				m_vLookatPt.z - oblist[q].z);
			OpenDoor(q, qdist, fTimeKey);
		}
	}


	//slow down in water

	//	if (closesoundid[0] <100){
	//			playerspeedlevel=80.0f;
	//	}
	//	else{
	//			playerspeedlevel=180.0f;
	//	}

	// wrap background pic through 360 degrees about y axis
	// we want to only display one quater of the background
	// texture at a time, because our Field Of View is 90 degrees.

	// texture co-ordinates     angle in degrees
	//   0 to .25				[  0 to  90]
	// .25 to .5				[ 90 to 180]
	// .50 to .75				[180 to 270]
	// .75 to  0				[270 to 360]
	//   0 to .25				[  0 to 90]

	// tu_left is the vertical texture co-ordinate for the left
	// hand side of background rectangle
	//tu_left = angy / 360.0000f;

	// tu_right is the vertical texture co-ordinate for the right
	// hand side of background rectangle
	//tu_right = tu_left + 0.250f;

	//m_pBackground[0].tu = tu_left;
	//m_pBackground[1].tu = tu_left;
	//m_pBackground[2].tu = tu_right;
	//m_pBackground[3].tu = tu_right;

	//gun_angle = -angy + (float)90;

	//if (gun_angle >= 360)
		//gun_angle = gun_angle - 360;
	//if (gun_angle < 0)
		//gun_angle = gun_angle + 360;

	//i = current_gun;
	//your_gun[i].rot_angle = gun_angle;
	//your_gun[i].x = m_vEyePt.x;
	//your_gun[i].y = (float)9 + m_vEyePt.y - 22;
	//your_gun[i].z = m_vEyePt.z;

	//for (i = 0; i < num_light_sources; i++)
	//	m_pd3dDevice->LightEnable((DWORD)i, FALSE);

	//num_light_sources = 0;

	//// Set up the light structure
	//D3DLIGHT7 light;
	//ZeroMemory(&light, sizeof(D3DLIGHT7));

	//light.dcvDiffuse.r = 1.0f;
	//light.dcvDiffuse.g = 1.0f;
	//light.dcvDiffuse.b = 1.0f;

	//light.dcvAmbient.r = 0.3f;
	//light.dcvAmbient.g = 0.3f;
	//light.dcvAmbient.b = 0.3f;


	//light.dvRange = 500.0f; // D3DLIGHT_RANGE_MAX

	// Calculate the flashlight's lookat point, from
	// the player's view direction angy.

	//lx = m_vEyePt.x + radius * sinf(angy * k);
	//ly = 0;
	//lz = m_vEyePt.z + radius * cosf(angy * k);

	//// Calculate direction vector for flashlight
	//dir_x = lx - m_vEyePt.x;
	//dir_y = 0; //ly - m_vEyePt.y;
	//dir_z = lz - m_vEyePt.z;

	//// set flashlight's position to player's position
	//pos_x = player_list[trueplayernum].x;
	//pos_y = player_list[trueplayernum].y;
	//pos_z = player_list[trueplayernum].z;

	//if (lighttype == 0)
	//{
	//	light.dvPosition = D3DVECTOR(pos_x, pos_y, pos_z);
	//	light.dvDirection = D3DVECTOR(dir_x, dir_y, dir_z);
	//	light.dvFalloff = .1f;
	//	light.dvTheta = .6f; // spotlight's inner cone
	//	light.dvPhi = 1.3f;	 // spotlight's outer cone
	//	light.dvAttenuation0 = 1.0f;
	//	light.dltType = D3DLIGHT_SPOT;
	//}
	//else
	//{

	//	light.dltType = D3DLIGHT_POINT;

	//	if (strstr(your_gun[current_gun].gunname, "LIGHTNINGSWORD"))
	//	{
	//		light.dcvAmbient.r = 1.0f;
	//		light.dcvAmbient.g = 1.0f;
	//		light.dcvAmbient.b = 1.0f;
	//	}
	//	else if (strstr(your_gun[current_gun].gunname, "FLAME") != NULL)
	//	{
	//		light.dcvAmbient.r = 1.0f;
	//		light.dcvAmbient.g = 0.2f;
	//		light.dcvAmbient.b = 0.3f;
	//	}
	//	else
	//	{
	//		light.dcvAmbient.r = 0.4f;
	//		light.dcvAmbient.g = 0.3f;
	//		light.dcvAmbient.b = 1.0f;
	//	}

	//	light.dcvDiffuse.r = light.dcvAmbient.r;
	//	light.dcvDiffuse.g = light.dcvAmbient.g;
	//	light.dcvDiffuse.b = light.dcvAmbient.b;

	//	light.dcvSpecular.r = 0.0f;
	//	light.dcvSpecular.g = 0.0f;
	//	light.dcvSpecular.b = 0.0f;
	//	light.dvRange = 200.0f;
	//	light.dvPosition.x = pos_x;
	//	light.dvPosition.y = pos_y;
	//	light.dvPosition.z = pos_z;

	//	light.dvAttenuation0 = 1.0f;
	//}

	//if (bIsFlashlightOn == TRUE)
	//{
	//	m_pd3dDevice->SetLight(num_light_sources, &light);
	//	m_pd3dDevice->LightEnable((DWORD)num_light_sources, TRUE);
	//	num_light_sources++;
	//}

	//if (GetAsyncKeyState(0x44) < 0)
	//{ // d -

	//	model_y -= 1;
	//}
	//if (GetAsyncKeyState(0x45) < 0)
	//{ // e -

	//	model_y += 1;
	//}

	r = (playerspeed)*fTimeKey;
	currentspeed = r;

	savevelocity = D3DXVECTOR3(0, 0, 0);

	direction = 0;
	if (playermove == 1)
	{

		direction = 1;
		directionlast = 1;
	}

	if (playermove == 4)
	{
		direction = -1;
		directionlast = -1;
	}

	bool addVel = false;

	if (movespeed < playerspeedmax && directionlast != 0)
	{
		addVel = true;


		if (direction)
		{

			if (moveaccel * movetime >= playerspeedlevel)
			{
				movespeed = playerspeedlevel * fTimeKey;
			}
			else
			{
				movetime = movetime + fTimeKey;
				movespeed = moveaccel * (0.5f * movetime * movetime);
				movespeedsave = movespeed;
				movespeed = movespeed - movespeedold;
				movespeedold = movespeedsave;
			}

			r = movespeed;
		}
		else
		{

			movetime = movetime - fTimeKey;

			if (movetime <= 0.0)
			{
				directionlast = 0;
				movetime = 0;
				r = 0;
			}
			else
			{
				movespeed = moveaccel * (0.5f * movetime * movetime);

				movespeedsave = movespeed;
				movespeed = movespeed - movespeedold;
				movespeedold = movespeedsave;

			}
			r = -1 * movespeed;
		}
		//r = playerspeed * fTimeKey;

		savevelocity.x = directionlast * r * sinf(angy * k);
		savevelocity.y = 0.0f;
		savevelocity.z = directionlast * r * cosf(angy * k);
	}
	else
	{

		movespeed = 0.0f;
		movetime = 0.0f;
		movespeedold = 0.0f;
		r = 0.0f;
	}

	if (playermovestrife == 6)
	{


		step_left_angy = angy - 90;

		if (step_left_angy < 0)
			step_left_angy += 360;

		if (step_left_angy >= 360)
			step_left_angy = step_left_angy - 360;

		r = (playerspeed)*fTimeKey;


		if (addVel) {

			savevelocity.x = r * sinf(step_left_angy * k) + savevelocity.x;
			savevelocity.y = 0.0f;
			savevelocity.z = r * cosf(step_left_angy * k) + savevelocity.z;
		}
		else {
			savevelocity.x = r * sinf(step_left_angy * k);
			savevelocity.y = 0.0f;
			savevelocity.z = r * cosf(step_left_angy * k);
		}



		//if (Magnitude(gvel) < maxgvel)
		//{
		//	gvel = gvel + savevelocity;
		//}
	}

	if (playermovestrife == 7)
	{

		step_left_angy = angy + 90;

		if (step_left_angy < 0)
			step_left_angy += 360;

		if (step_left_angy >= 360)
			step_left_angy = step_left_angy - 360;

		r = (playerspeed)*fTimeKey;


		if (addVel) {

			savevelocity.x = r * sinf(step_left_angy * k) + savevelocity.x;
			savevelocity.y = 0.0f;
			savevelocity.z = r * cosf(step_left_angy * k) + savevelocity.z;
		}
		else {
			savevelocity.x = r * sinf(step_left_angy * k);
			savevelocity.y = 0.0f;
			savevelocity.z = r * cosf(step_left_angy * k);

		}

		//if (Magnitude(gvel) < maxgvel)
		//{
		//	gvel = gvel + savevelocity;
		//}
	}


	if (playermove == 2 || playermove == 3 || savelastmove == 2 || savelastmove == 3) {

	}
	else if (player_list[trueplayernum].current_sequence != 2   )
	{
		if (playermove == 0)
		{
			if (savelastmove != playermove && jump == 0 )
			{
					SetPlayerAnimationSequence(trueplayernum, 0);
			}
		}
		else if (playermove == 1)
		{
			if (savelastmove != playermove && jump == 0)
			{
				SetPlayerAnimationSequence(trueplayernum, 1);
			}
		}

		//if (playermovestrife == 0)
		//{
		//	if (playermovestrife != savelaststrifemove && jump == 0)
		//	{
		//		if (playermove == 0 && (savelastmove != 2 && savelastmove != 3))
		//			SetPlayerAnimationSequence(trueplayernum, 0);
		//	}
		//}
		//else
		//{
		//	if (playermovestrife != savelaststrifemove && jump == 0)
		//		SetPlayerAnimationSequence(trueplayernum, 1);
		//}
	}

	// gvel = savevelocity;


	savelastmove = playermove;


	
	savelaststrifemove = playermovestrife;
	//saveoldvelocity = savevelocity;

	//D3DVECTOR savepos;
	D3DVECTOR result;

	//savepos = m_vEyePt;

	//eRadius = D3DVECTOR(spheresize, sphereheight, spheresize);

	//if (collisioncode == 0)
	//{
	//	loadcollisionmap(m_vEyePt, gvel, eRadius);
	//	result = finalv;
	//}
	//else
	//{
		//result = collideWithWorld(m_vEyePt / eRadius, (gvel) / eRadius);
		//result = result * eRadius;

	//TODO: FIX THIS
	//savevelocity.y = savevelocity.y + 10.0f * fTimeKey;

	result = collideWithWorld(RadiusDivide(m_vEyePt, eRadius), RadiusDivide(savevelocity, eRadius));
	result = RadiusMultiply(result, eRadius);


	//}



	m_vEyePt.x = result.x;
	m_vEyePt.y = result.y;
	m_vEyePt.z = result.z;

	//GRAVITY HERE

	//savevelocity.x = 0.0f;
	//savevelocity.y = (float)(-400.0f) * fTimeKey;
	//savevelocity.z = 0.0f;


	//result = collideWithWorld(RadiusDivide(m_vEyePt, eRadius), RadiusDivide(savevelocity, eRadius));
	//result = RadiusMultiply(result, eRadius);
	//

	//m_vEyePt.x = result.x;
	//m_vEyePt.y = result.y;
	//m_vEyePt.z = result.z;


	//Jump


	if (gravityon == 1) {

		if (jump == 1)
		{

			if (jumpvdir == 0)
			{

				jumpcount = 0.0f;
				savevelocity.x = 0.0f;
				savevelocity.y = (float)(400.0f) * fTimeKey;
				savevelocity.z = 0.0f;
				jumpv = savevelocity;

				if (maingameloop)
					jumpcount++;

				if (jumpv.y <= 1.0f)
				{
					jumpv.y = 0.0f;
				}
			}

		}

		if (jumpstart == 1)
		{
			lastjumptime = 0.0f;
			jumpstart = 0;
			cleanjumpspeed = 600.0f;
			totaldist = 0.0f;
			gravityvector.y = -50.0f;
		}

		if (lastcollide == 1)
		{
			gravitytime = gravitytime + fTimeKey;
		}

		modellocation = m_vEyePt;

		savevelocity.x = 0.0f;
		savevelocity.y = (cleanjumpspeed * gravitytime) + -2600.0f * (0.5f * gravitytime * gravitytime);
		savevelocity.z = 0.0f;

		saveoldvelocity = savevelocity;
		savevelocity.y = (savevelocity.y - gravityvectorold.y);
		gravityvectorold.y = saveoldvelocity.y;

		if (savevelocity.y == 0.0f && jump == 0)
			savevelocity.y = -80.0f * fTimeKey;

		if (savevelocity.y <= -80.0f)
			savevelocity.y = -80.0f;

		foundcollisiontrue = 0;

		//eRadius = D3DVECTOR(spheresize, sphereheight, spheresize);
		//result = collideWithWorld(m_vEyePt / eRadius, (savevelocity) / eRadius);
		//result = result * eRadius;

		result = collideWithWorld(RadiusDivide(m_vEyePt, eRadius), RadiusDivide(savevelocity, eRadius));
		result = RadiusMultiply(result, eRadius);

		m_vEyePt.x = result.x;
		m_vEyePt.y = result.y;
		m_vEyePt.z = result.z;

		if (foundcollisiontrue == 0)
		{
			nojumpallow = 1;

			if (lastcollide == 1)
			{
				lastjumptime = gravitytime;
				totaldist = totaldist + savevelocity.y;
			}

			lastcollide = 1;

			gravityvector.y = -50.0f;
			if (gravitydropcount == 0)
				gravitydropcount = 1;
		}
		else
		{
			//something is under us

			if (lastcollide == 1 && savevelocity.y <= 0)
			{
				if (gravitytime >= 0.4f)
					PlayWavSound(SoundID("jump_land"), 100);

				gravityvector.y = 0.0f;
				gravityvectorold.y = 0.0f;
				cleanjumpspeed = -200.0f;

				lastcollide = 0;
				jump = 0;

				gravitytime = 0.0f;
			}
			else if (lastcollide == 1 && savevelocity.y > 0)
			{
				if (gravitytime >= 0.4f)
					PlayWavSound(SoundID("jump_land"), 100);

				cleanjumpspeed = -200.0f;
				lastcollide = 0;
				gravitytime = 0.0f;
				gravityvectorold.y = 0.0f;
			}
			nojumpallow = 0;
			gravitydropcount = 0;
		}

		modellocation = m_vEyePt;
	}

	//m_vEyePt.x = m_vEyePt.x + savevelocity.x;
	//m_vEyePt.y = m_vEyePt.y + savevelocity.y;
	//m_vEyePt.z = m_vEyePt.z + savevelocity.z;


	float newangle = 0;
	newangle = fixangle(look_up_ang, 90);

	m_vLookatPt.x = m_vEyePt.x + cameradist * sinf(newangle * k) * sinf(angy * k);
	m_vLookatPt.y = m_vEyePt.y + cameradist * cosf(newangle * k);
	m_vLookatPt.z = m_vEyePt.z + cameradist * sinf(newangle * k) * cosf(angy * k);

	//MissleSave = m_vLookatPt - m_vEyePt;

	playermove = 0;
	playermovestrife = 0;

	D3DVECTOR vw1, vw2, vw3;
	D3DVECTOR pNormal;

	realEye.x = m_vEyePt.x + 14.0f * sinf(angy * k) * -50.0f;
	realEye.y = m_vEyePt.y;
	realEye.z = m_vEyePt.z + 14.0f * cosf(angy * k) * -50.0f;

	EyeTrue = m_vEyePt;

	EyeTrue.y = m_vEyePt.y + cameraheight;

	LookTrue = m_vLookatPt;
	LookTrue.y = m_vLookatPt.y + cameraheight;
	cameraf.x = LookTrue.x;
	cameraf.y = LookTrue.y;
	cameraf.z = LookTrue.z;

	modellocation = m_vEyePt;

	GunTruesave = EyeTrue;

	//D3DUtil_SetViewMatrix(matView, EyeTrue, LookTrue, m_vUpVec);
	//D3DUtil_SetProjectionMatrix(matProj, FOV, ASPECT_RATIO, Z_NEAR, Z_FAR);

	//D3DUtil_SetIdentityMatrix(matWorld);
	//m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);
	//m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
	//m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

	//if (dialogpause == 0)
		//MoveMonsters();

	//UpdateMainPlayer();

	return S_OK;
}
