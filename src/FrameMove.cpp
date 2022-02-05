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
void PlayerJump(const FLOAT& fTimeKey);
void FindDoors(const FLOAT& fTimeKey);
void GameTimers(const FLOAT& fTimeKey);
void MovePlayer(const FLOAT& fTimeKey);
void StrifePlayer(FLOAT& fTimeKey);
void PlayerAnimation();
void CheckAngle();

HRESULT FrameMove(double fTime, FLOAT fTimeKey)
{
	float cameradist = 50.0f;
	
	GameTimers(fTimeKey);
	CheckAngle();
	GetItem();
	MonsterHit();
	FindDoors(fTimeKey);
	MovePlayer(fTimeKey);
	StrifePlayer(fTimeKey);
	PlayerAnimation();
	savelastmove = playermove;
	savelaststrifemove = playermovestrife;

	D3DVECTOR result;

	result = collideWithWorld(RadiusDivide(m_vEyePt, eRadius), RadiusDivide(savevelocity, eRadius));
	result = RadiusMultiply(result, eRadius);

	m_vEyePt.x = result.x;
	m_vEyePt.y = result.y;
	m_vEyePt.z = result.z;

	PlayerJump(fTimeKey);

	float newangle = 0;
	newangle = fixangle(look_up_ang, 90);

	m_vLookatPt.x = m_vEyePt.x + cameradist * sinf(newangle * k) * sinf(angy * k);
	m_vLookatPt.y = m_vEyePt.y + cameradist * cosf(newangle * k);
	m_vLookatPt.z = m_vEyePt.z + cameradist * sinf(newangle * k) * cosf(angy * k);

	playermove = 0;
	playermovestrife = 0;
	

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

	GunTruesave = EyeTrue;

	return S_OK;
}

void CheckAngle()
{
	if (angy >= 360)
		angy = angy - 360;

	if (angy < 0)
		angy += 360;
}

void PlayerJump(const FLOAT& fTimeKey)
{

	D3DVECTOR result;

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
}

void PlayerAnimation()
{
	if (playermove == 2 || playermove == 3 || savelastmove == 2 || savelastmove == 3) {

	}
	else if (player_list[trueplayernum].current_sequence != 2)
	{
		if (playermove == 0)
		{
			if (savelastmove != playermove && jump == 0)
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

}

void StrifePlayer(FLOAT& fTimeKey)
{
	float step_left_angy = 0;
	float r = 15.0f;
	bool addVel = false;

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
}

void MovePlayer(const FLOAT& fTimeKey)
{
	bool addVel = false;

	float r = (playerspeed)*fTimeKey;
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
}

void FindDoors(const FLOAT& fTimeKey)
{
	//Find doors
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
}

void GameTimers(const FLOAT& fTimeKey)
{
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
}
