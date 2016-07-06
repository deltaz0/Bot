/*
----o0o=================================================================o0o----
* Copyright (c) 2006, Ian Parberry
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the University of North Texas nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
----o0o=================================================================o0o----
*/

/// \file PlaneObject.cpp
/// \brief Code for the PlaneObject class.

/// Edited by the SAGE team, 2005
/// Last updated June 13, 2006

#include <assert.h>
#include "Common/MathUtil.h"
#include "PlaneObject.h"
#include "Input/Input.h"
#include "ObjectTypes.h"
#include "common/RotationMatrix.h"
#include "Ned3DObjectManager.h"
#include "Particle/ParticleEngine.h"
#include "Sound/SoundManager.h"
#include "game.h"
#include "directorymanager/directorymanager.h"
#include "StatePlaying.h"
#include "math.h"
#include "Game.h"
#include "Game\GameBase.h"
#include "game.h"
#include "Ned3DObjectManager.h"


StatePlaying gStatePlaying;
XINPUT_STATE state; //State for xinput controller

//bool PlaneObject::savePoint = false;
bool PlaneObject::takeDamage = true;


PlaneObject::PlaneObject(Model *m):
  GameObject(m,1),
  m_gunPosition(0, 2.2f, 3.1f),
  m_enginePosition(0,0.0f, 3.3f),
  m_propOffset(0.4f),
  m_isPlaneAlive(true),
	looking(false),
  m_maxTurnRate(kPi * 0.25f),
  m_maxPitchRate(kPi * 0.25f),
  m_maxBankRate(kPi * 0.25f),
  m_planeState(PS_FLYING),
  m_turnState(TS_STRAIGHT),
  m_pitchState(PS_LEVEL),
  m_moveState(MS_STOP),
  m_hp(4),
  m_maxHP(4),
  jumpHeading(0.0f),
  jumpSpeedRatio(0.0f),
  shiftHeld(FALSE),
  spaceHeld(FALSE),
  hoverJump(FALSE),
  dHeld(FALSE),
  aHeld(FALSE),
  wHeld(FALSE),
  sHeld(FALSE),
  prevShiftHeld(FALSE),
  ledgeGrab(FALSE),
  ledgeMin(0.0f),
  ledgeMax(0.0f),
  ledgeTimer(-1),
  canHover(TRUE),
  wallJumpDir(0.0f),
  swingState(0),
  swingPitch(0.0f),
  swingPos(0.0f,0.0f,0.0f),
  energy(100.0f),
  energyFlag(TRUE),
  jumpTimer(-1),
  swingTimer(-1),
  sTimer(-1),
  hTimer(-1),
  wallRun(0),
  dashSoundPlaying(FALSE),
  musicPlaying(FALSE),
  lockMovement(false),
  XA(false),
  XRT(false),
  XLT(false),
  XX(false),
  XY(false),
  XB(false)
{
  assert(m);
  assert(m->getPartCount() >= 1);
  setModelOrientation(EulerAngles(kPi, 0.0f, 0.0f));
  setPosition(0,m_propOffset,0,0);
  m_fSpeed = 0.0f;
  m_ySpeed = 0.0f;
  m_maxSpeed = 2.0f;
  m_className = "Plane";
  m_type = ObjectTypes::PLANE;
  m_pitchRate = m_turnRate = 0.0f;
  m_smokeID = -1;
  m_reticleLockOnUpdated = false;

  m_timeSinceFired = gRenderer.getTime();

  // load gunshot sound
  gDirectoryManager.setDirectory(eDirectorySounds);
  m_gunSound = gSoundManager.load("Gun.wav",12);
  gSoundManager.setDistance(m_gunSound,50.0f,100000.0f);

  m_jumpSound = gSoundManager.load("jump.wav",2);
  gSoundManager.setDistance(m_jumpSound,50.0f,100000.0f);

  m_dashSound = gSoundManager.load("dash.wav",1);
  gSoundManager.setDistance(m_dashSound,50.0f,100000.0f);

  m_music = gSoundManager.load("funkhouse.wav", 1);

  // load reticle texture
  m_reticleTexture = gRenderer.cacheTextureDX("reticle.png");

  // load all textures that will be used on the plane.
  // Multiple textures are used throughout game play so the plane appears to
  // "take damage"
  /*
  m_allTextures.resize(5);
  m_allTextures[0] = "plane2life0.tga"; // when hp = 0
  m_allTextures[1] = "plane2life1.tga"; // hp = 1
  m_allTextures[2] = "plane2life2.tga"; // hp = 2
  m_allTextures[3] = "plane2life3.tga"; // hp = 3
  m_allTextures[4] = "plane2.tga";      // hp > 3

  // cache all these
  for (int a =0; a < (int)m_allTextures.size(); a++)
    gRenderer.cacheTextureDX(m_allTextures[a].c_str());
    */
  m_allParticles.resize(5);
  m_allParticles[0] = "smokeveryheavy";   // when hp = 0
  m_allParticles[1] = "smokeheavy";   // when hp = 0
  m_allParticles[2] = "smokemedium";  // hp = 1
  m_allParticles[3] = "smokelight";   // hp = 2
  m_allParticles[4] = "";             // hp > 2

  setTextureAndSmoke();

}

PlaneObject::~PlaneObject()
{
  // kill particle engine if one is attached
  if (m_smokeID != -1)
    gParticle.killSystem(m_smokeID);
  m_smokeID = -1;

}

void PlaneObject::renderReticle(float distance, float size)
{    
	/*  
  // test terrain ray intersect
    Vector3 right = Vector3::kRightVector * size;
    Vector3 up = Vector3::kUpVector * size;
    
    Vector3 gunPos = m_gunPosition;
    Vector3 reticlePos = m_gunPosition;
    reticlePos.z += distance;

    Vector3 bulletDir = Vector3(0, 0, 1);    
        
    // set up a matrix to rotate points that are relative to the plane
    RotationMatrix r1;
    r1.setup(getOrientation());

    // transform points relative to the plane
    bulletDir = r1.objectToInertial(bulletDir);
    gunPos = r1.objectToInertial(gunPos);
    reticlePos = r1.objectToInertial(reticlePos);    
    right = r1.objectToInertial(right);
    up = r1.objectToInertial(up);

    // add position of the plane
    reticlePos += getPosition();
    gunPos += getPosition();


    // set up quad
    RenderVertexL verts[4];
    verts[0].p = reticlePos - right + up; 
    verts[0].u = 0.0f; verts[0].v = 0.0f;
    verts[1].p = reticlePos + right + up;
    verts[1].u = 1.0f; verts[1].v = 0.0f;
    verts[2].p = reticlePos + right - up;
    verts[2].u = 1.0f; verts[2].v = 1.0f;
    verts[3].p = reticlePos - right - up;
    verts[3].u = 0.0f; verts[3].v = 1.0f;
    
    
    // if we haven't already checked if the gun lines up with a crow
    if (!m_reticleLockOnUpdated)
    {
      //TODO: Make sure this only happens once per process() call    
      m_reticleLockedOn = 
      gGame.m_statePlaying.m_objects->rayIntersectCrow(gunPos,bulletDir * gBulletRange);
      m_reticleLockOnUpdated = true;
    }

    // set up color
    unsigned int color;
    if (m_reticleLockedOn)
    {      
      // make the color flash based on time
      int repeat = 600;
      int time = gRenderer.getTime() % repeat;
      float intensity = (sin( (float)time / (float)repeat * k2Pi) + 1.0f) / 2.0f;
      color = MAKE_ARGB(200.0f * intensity + 55.0f,255,0,0);
    }
    else color = MAKE_ARGB(255,0,180,0);

    for (int a = 0; a < 4; a++) verts[a].argb = color;

    // render reticle
    gRenderer.selectTexture(m_reticleTexture);
    gRenderer.renderQuad(verts);
	*/
}


void PlaneObject::process(float dt)
{
  if(!m_isPlaneAlive) return;
  // save the position
  m_oldPosition = getPosition();

  // Put any non-movement logic here

  // Unbuffered input (remove if handling input elsewhere)  
  inputStraight();
  inputLevel();
  inputStop();

  XInputGetState(0, &state);




	if((!looking && gInput.keyDown(DIK_RSHIFT)) || (!looking && state.Gamepad.bLeftTrigger ) || (!looking && (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)) && !lockMovement)
	{
		prevShiftHeld = true;
		if(energyFlag==TRUE)
		{
			if(inAir==false/*&&(swingState==0)*/)
				shiftHeld = TRUE;
			else
				shiftHeld = FALSE;
		}
		else
		{
			shiftHeld = FALSE;
			prevShiftHeld = false;
		}
	}
	else
	{
	 shiftHeld = FALSE;
	 prevShiftHeld = false;
	}
  //dI used to store directional input value
  int dI = 0;
 

  if(gInput.keyJustDown(DIK_SPACE) || state.Gamepad.wButtons & XINPUT_GAMEPAD_A || state.Gamepad.bRightTrigger)
  {
		if(!looking)
		{
			if(state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !XA)
			{
				XA = true;
				jumpPlayer(5.0f);
			}
			else if(state.Gamepad.bRightTrigger && !XRT)
			{
				XRT = true;
				jumpPlayer(5.0f);
			}
			else if(gInput.keyJustDown(DIK_SPACE))
				jumpPlayer(5.0f);
		}
  }

 

  if(gInput.keyJustUp(DIK_SPACE))
  {
      canHover = true;
  }

   //|| state.Gamepad.wButtons & XINPUT_GAMEPAD_A || state.Gamepad.bRightTrigger 

  if(gInput.keyDown(DIK_SPACE) || state.Gamepad.wButtons & XINPUT_GAMEPAD_A || state.Gamepad.bRightTrigger)
  {
     spaceHeld = true;
	 if(state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !XA)
		 XA = true;
	 if(state.Gamepad.bRightTrigger && !XRT)
		 XRT = true;
	 //jumpPlayer(5.0f);
  }
  else
  {
	  stopJump();
      spaceHeld = false;
  }
  if(!(state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && XA)
  {
	  stopJump();
	  XA = false;
	  canHover = true;
  }
  if(!(state.Gamepad.bRightTrigger) && XRT)
  {
	  stopJump();
	  XRT = false;
	  canHover = true;
  }
 
  //W,D,A,S set dI to a 4bit binary representation of directional keys
  //ex: 5 = 0101 = _D_S = S+D held = down+right
 // if(state.Gamepad.wButtons & XINPUT_GAMEPAD_X){
 //   aHeld = true;
	//dI = dI +2;
 // }

  if ((gInput.keyDown(DIK_A) || state.Gamepad.sThumbLX < -5000) && !lockMovement)
  {
		if(!looking)
		{
			aHeld = true;
			dI = dI + 2;
		}
  }
  else
      aHeld = false;
  if ((gInput.keyDown(DIK_D) || state.Gamepad.sThumbLX > 5000) && !lockMovement)
  {
		if(!looking)
		{
			dHeld = true;
			dI = dI + 4;
		}
  }
  else
      dHeld = false;
  if ((gInput.keyDown(DIK_S) || state.Gamepad.sThumbLY < -5000)   && !lockMovement)
  {
		if(!looking)
		{
			sHeld = true;
			dI = dI + 1;
		}
  }
  else
      sHeld = false;
  if ((gInput.keyDown(DIK_W) || state.Gamepad.sThumbLY > 5000) && !lockMovement)
  {
		if(!looking)
		{
			wHeld = true;
			dI = dI + 8;
		}
  }
  else
      wHeld = false;
    
  //send dI to binaryconv() which changes the binary representation to respective 0-9 numpad DI value
  dI = binaryConv(dI);
  bool move = true;
  float horizontal = state.Gamepad.sThumbLX/*/65534*/;
  float vertical = state.Gamepad.sThumbLY/*/65534*/;
  if(vertical < 5000 && vertical > -5000)
	  vertical = 0;
  if(horizontal < 5000 && horizontal > -5000)
	  horizontal = 0;
  if(horizontal == 0 && vertical == 0)
	  move = false;
  D3DXVECTOR3 tempVec;
  tempVec.x = horizontal;
  tempVec.y = vertical;
  tempVec.z = 0;
  D3DXVec3Normalize(&tempVec,&tempVec);
  float angle = atan2(tempVec.x, tempVec.y);
  if(move)
	movePlayer(angle);
  //call movePlayer() based on 0-9 dI value
  if(swingState==0 && horizontal == 0 && vertical == 0)
  {
  switch(dI)
  {
  case 0:
	  break;
  case 1:	//down left
	  movePlayer(-0.75f*kPi);
	  break;
  case 2:	//down
	  movePlayer(kPi);
	  break;
  case 3:	//down right
	  movePlayer(0.75f*kPi);
	  break;
  case 4:	//left
	  movePlayer(-0.5f*kPi);
	  break;
  case 5:
	  break;
  case 6:	//right
	  movePlayer(0.5f*kPi);
	  break;
  case 7:	//up left
	  movePlayer(-0.25f*kPi);
	  break;
  case 8:	//forward
	  movePlayer(0.0f);
	  break;
  case 9:	//up right
	  movePlayer(0.25f*kPi);
	  break;
  default:
	  break;
  }
  }


	if(gInput.keyJustDown(DIK_U) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y))
	{
		if(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y && !XY)
		{
			XY = true;
			if(gStatePlaying.getCamMode() == 0)
			{
				gStatePlaying.setCam(1);
				looking = true;
			}
			else
			{
				gStatePlaying.setCam(0);
				looking = false;
			}
		}
		else if(gInput.keyJustDown(DIK_U))
		{
			if(gStatePlaying.getCamMode() == 0)
			{
				gStatePlaying.setCam(1);
				looking = true;
			}
			else
			{
				gStatePlaying.setCam(0);
				looking = false;
			}
		}
		//gStatePlaying.m_tetherCamera->m_nCamMode = 0;
		/*int tempId = m_objects->getPlane();
		m_crowID = m_objects->getCrow();
		if (m_crowID != -1)
		m_tetherCamera->setTargetObject(m_crowID);*/
	}
	if(!(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) && XY)
	{
		XY = false;
	}

	if(state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
	{
		if(state.Gamepad.wButtons & XINPUT_GAMEPAD_B && !XB)
		{
			XB = true;
			if(gStatePlaying.getCamMode() == 0)
			{
				gStatePlaying.setCam(1);
				looking = true;
			}
			else
			{
				gStatePlaying.setCam(0);
				looking = false;
			}
		}
		//gStatePlaying.m_tetherCamera->m_nCamMode = 0;
		/*int tempId = m_objects->getPlane();
		m_crowID = m_objects->getCrow();
		if (m_crowID != -1)
		m_tetherCamera->setTargetObject(m_crowID);*/
	}
	if(!(state.Gamepad.wButtons & XINPUT_GAMEPAD_B) && XB)
	{
		XB = false;
  }

  if(gInput.keyDown(DIK_LEFTARROW) || state.Gamepad.sThumbRX >= 22000)
  {
		if(looking)
		{
			if(gInput.keyDown(DIK_LEFTARROW))
				gStatePlaying.incrementCamHeading(-.1f);
			else
				gStatePlaying.incrementCamHeading(.1f);
		}
		else
      gStatePlaying.moveCamera(false,1.0f);
  }

  if(gInput.keyDown(DIK_RIGHTARROW) || state.Gamepad.sThumbRX < -22000)
  {
		if(looking)
		{
			if(gInput.keyDown(DIK_RIGHTARROW))
				gStatePlaying.incrementCamHeading(.1f);
			else
				gStatePlaying.incrementCamHeading(-.1f);
		}
		else
      gStatePlaying.moveCamera(true,1.0f);
  }

  if(gInput.keyDown(DIK_UPARROW) || state.Gamepad.sThumbRY >= 22000)
  {
		if(looking)
			gStatePlaying.incrementCamPitch(-.1f);
		else
      gStatePlaying.moveCamera2(false,0.4f);
  }

  if(gInput.keyDown(DIK_DOWNARROW) || state.Gamepad.sThumbRY <= -22000)
  {
		if(looking)
			gStatePlaying.incrementCamPitch(.1f);
		else
      gStatePlaying.moveCamera2(true,0.4f);
  }



  if (gInput.keyJustUp(DIK_EQUALS))
    inputSpeedUp();
  if (gInput.keyJustUp(DIK_MINUS))
    inputSpeedDown();
  if (gInput.keyDown(DIK_RETURN))
    inputForward(1.0f);

  // process all joystick input if it is disabled
  //if (gInput.joyEnabled())
  //{
  //  // set plane speed based on slider
  //  float speed = 1.0f;
  //  if (gInput.joySlider(&speed))
  //  {
  //    // if there is a slider
  //    inputForward(speed);    
  //  }
  //  else // there isn't a slider
  //  {
  //    // if button 1 is down then the plane flys foward
  //    if (gInput.joyButtonDown(1))
  //      inputForward(1.0f);         
  //  }

  //  if(gInput.joyPadPositionX() < 0.0f)  
  //    inputTurnLeft(-gInput.joyPadPositionX());  
  //  if(gInput.joyPadPositionX() > 0.0f)
  //    inputTurnRight(gInput.joyPadPositionX());
  //  if(gInput.joyPadPositionY() > 0.0f)
  //    inputDive(gInput.joyPadPositionY());
  //  if(gInput.joyPadPositionY() < 0.0f)
  //    inputClimb(-gInput.joyPadPositionY());

  //  if (gInput.joyButtonDown(0)) inputFire();
  //}

  if (m_planeState == PS_FLYING)
    m_velocity = (getPosition() - m_oldPosition)/dt;

}

void PlaneObject::move(float dt)
{
  if(!m_isPlaneAlive)
    return;
  
  if(musicPlaying==FALSE)
  {
              int musicInstance = gSoundManager.requestInstance(m_music);
              gSoundManager.setPosition(m_music, musicInstance, getPosition());
              gSoundManager.play(m_music, musicInstance);
              gSoundManager.releaseInstance(m_music, musicInstance);
              musicPlaying = TRUE;
  }

  if(sTimer==-1)
      sTimer = gRenderer.getTime();
  else if((gRenderer.getTime() - sTimer)>10)
  {
      shiftTimer();
      hoverTimer();
      sTimer = gRenderer.getTime();
  }

  EulerAngles &planeOrient = m_eaOrient[0];
  Vector3 displacement = Vector3::kZeroVector;
  
  if (m_planeState == PS_FLYING)
  {

    // Update plane orientation

    /*
    switch(m_turnState)
    {
      case TS_LEFT:
      {
        planeOrient.heading -= m_maxTurnRate * m_turnRate * dt;
        if(planeOrient.bank < kPi * 0.25f)
        {
          //planeOrient.bank += m_maxBankRate * m_turnRate * dt;
        }
      } break;
      case TS_RIGHT:
      {
        planeOrient.heading += m_maxTurnRate * m_turnRate * dt;
        if(planeOrient.bank > kPi * -0.25f)
        {
          //planeOrient.bank -= m_maxBankRate * m_turnRate * dt;
        }
      }
    };
    switch(m_pitchState)
    {
      case PS_CLIMB:
      {
        planeOrient.pitch += m_pitchRate * m_maxPitchRate * dt;
      } break;
      case PS_DIVE:
      {
        planeOrient.pitch -= m_pitchRate * m_maxPitchRate * dt;
      }
    }
    */
    //clamp(planeOrient.pitch,-kPi * 0.125f,kPi * 0.125f);
    //float bankCorrect = 0.5f * m_maxBankRate * dt;
    //float pitchCorrect = 0.5f * m_maxPitchRate * dt;
    if(m_turnState == TS_STRAIGHT)
        /*
      if(planeOrient.bank > bankCorrect)
        planeOrient.bank -= bankCorrect;
      else if(planeOrient.bank < -bankCorrect)
        planeOrient.bank += bankCorrect;
      else*/
      planeOrient.bank = 0.0f;
    if(m_pitchState == PS_LEVEL)
        /*
      if(planeOrient.pitch > pitchCorrect)
        planeOrient.pitch -= pitchCorrect;
      else if(planeOrient.pitch < -pitchCorrect)
        planeOrient.pitch += pitchCorrect;
      else*/
      planeOrient.pitch = 0.0f;
      
    
    // Check move state

	  //gravity
	  /*
      Vector3 grav = getPosition();
      setPosition(grav.x, grav.y - 0.5f, grav.z);
	  */
	  if(inAir==TRUE/*&&(swingState==0)*/)
	  {
          if(wallRun==0)
            setySpeed(m_ySpeed-0.25f);
          else
            setySpeed(m_ySpeed-0.15f);
 
          //EulerAngles &planeOrient = m_eaOrient[0];
          float pHead = planeOrient.heading;
      
          bool hFlag = false;
          if(wallJumpDir!=0.0f)
          {
              if(pHead<0)
                  pHead = pHead + (kPi * 2.0f);
              if(wallJumpDir==2.0f)
              {
                  if((pHead>=0.0f)&&(pHead<=(kPi*0.5f)))
                      hFlag = true;
                  else if((pHead>=(kPi*1.5f))&&(pHead<=(kPi*2.0f)))
                      hFlag = true;
              }
              else
              {
                  float wMax = (wallJumpDir + 0.5f) * kPi;
                  float wMin = (wallJumpDir - 0.5f) * kPi;
                  if((pHead<=wMax)&&(pHead>=wMin))
                      hFlag = true;
              }
          }
          if((hFlag==true)&&(ledgeGrab==false))
          {
              jumpHeading = planeOrient.heading;
              jumpSpeedRatio = 1.0f;
              wallJumpDir = 0.0f;
              wallRun = 0;
          }

          if(((hoverJump==true)&&(spaceHeld==true))&&((energy>10.0f)&&(m_ySpeed<-0.7f)))
          {
              if(((gRenderer.getTime()>(swingTimer + 300))&&(canHover==true))&&(wallJumpDir==0.0f))
              {
                if(m_ySpeed<-0.8f)
                    m_ySpeed = -0.8f;
                jumpSpeedRatio = 0.7f;
                jumpHeading = planeOrient.heading;
                if(((aHeld==false)&&(sHeld==false))&&((wHeld==false)&&(dHeld==false)))
                    jumpSpeedRatio = 0.1f;
              }
          }

          planeOrient.heading = jumpHeading;
          m_moveState = MS_FORWARD;
          //if(swingState==0)
          m_speedRatio = jumpSpeedRatio;


          if(ledgeGrab==true)
          {
              if(dashSoundPlaying==TRUE)
              {
                  gSoundManager.stop(m_dashSound);
                  dashSoundPlaying = FALSE;
              }

              m_speedRatio = 0.0f;
              m_fSpeed = 0.0f;
              m_ySpeed = 0.0f;
              jumpSpeedRatio = 0.0f;
              EulerAngles lOrient = EulerAngles::kEulerAnglesIdentity;
              lOrient.heading = (wallJumpDir * kPi) + kPi;
              Vector3 gPos = getPosition();
              if(wallJumpDir==1.0f)
                  gPos.x += sin(pHead - lOrient.heading) * 0.4f;
              else if(wallJumpDir==2.0f)
                  gPos.x -= sin(pHead - lOrient.heading) * 0.4f;
              else if(wallJumpDir==0.5f)
                  gPos.z += sin(pHead - lOrient.heading) * 0.4f;
              else if(wallJumpDir==1.5f)
                  gPos.z -= sin(pHead - lOrient.heading) * 0.4f;
              if((wallJumpDir==1.0f)||(wallJumpDir==2.0f))
              {
                  if(gPos.x>ledgeMax)
                      gPos.x = ledgeMax;
                  if(gPos.x<ledgeMin)
                      gPos.x = ledgeMin;
              }
              else if((wallJumpDir==0.5f)||(wallJumpDir==1.5f))
              {
                  if(gPos.z>ledgeMax)
                      gPos.z = ledgeMax;
                  if(gPos.z<ledgeMin)
                      gPos.z = ledgeMin;
              }
              setOrientation(lOrient);
              setPosition(gPos);
              //planeOrient.heading = lOrient.heading;
              jumpHeading = lOrient.heading;

              if((spaceHeld==true)&&(gRenderer.getTime()>(ledgeTimer + 500)))
              {
                  float hopSpeed = 0.5f;
                  if(hFlag==false)
                  {
                      wallJumpDir += (wallJumpDir>1.0f) ? -1.0f : 1.0f;
                      hopSpeed = 3.0f;
                  }
                  ledgeGrab = false;
                  ledgeTimer = gRenderer.getTime();
                  jumpPlayer(hopSpeed);
                  if(hFlag==true)
                      jumpSpeedRatio = jumpSpeedRatio * 0.7f;
                  m_speedRatio = jumpSpeedRatio;
              }
              else if((prevShiftHeld==true)&&(gRenderer.getTime()>(ledgeTimer + 500)))
              {
                  ledgeGrab = false;
                  ledgeTimer = gRenderer.getTime();
              }
          }


          if(swingState!=0)
          {
              if(dashSoundPlaying==TRUE)
              {
                  gSoundManager.stop(m_dashSound);
                  dashSoundPlaying = FALSE;
              }
              m_speedRatio = 0.0f;
              m_fSpeed = 0.0f;
              m_ySpeed = 0.0f;
              jumpHeading = 0.0f;
              jumpSpeedRatio = 0.0f;
              float rTimer = float(gRenderer.getTime() % 1000) * 0.001f;
              Vector3 radPos = getRadialPos(swingPos, swingState, 7.5f, rTimer);
              EulerAngles radOrient = getRadialOrient(rTimer, swingState);
              setPosition(radPos);

              if((spaceHeld==true)&&(gRenderer.getTime()>(swingTimer + 1000)))
              {
                  swingState = 0;
                  jumpTimer = gRenderer.getTime();
                  swingTimer = jumpTimer;
                  m_ySpeed = 5.0f * sin(-radOrient.pitch);
                  jumpSpeedRatio = 2.0f * cos(radOrient.pitch);
                  if(jumpSpeedRatio<0.0f)
                      radOrient.heading -= kPi;
                  jumpHeading = radOrient.heading;
                  jumpSpeedRatio = abs(jumpSpeedRatio);
                  m_speedRatio = jumpSpeedRatio;
                  radOrient.pitch = 0.0f;
                  canHover==false;
              }

              setOrientation(radOrient);
          }
	  }
      /*
      else if((inAir==true)&&(swingState!=0))
      {
            float pz = getPreviousPosition().z;
            float py = getPreviousPosition().y;
            EulerAngles &planeOrient = m_eaOrient[0];
            if(bz>=pz)
            {
                if(by>=py)
                    planeOrient.pitch = (kPi * 0.0f) + atan((bz-pz)/(by-py));
                else
                    planeOrient.pitch = (kPi * 0.5f) + atan((py-by)/(bz-pz));
            }
            else
            {
                if(by<=py)
                    planeOrient.pitch = (kPi * 1.0f) + atan((pz-bz)/(py-by));
                else
                    planeOrient.pitch = (kPi * 1.5f) + atan((by-py)/(pz-bz));
            }
            m_speedRatio = 0.0f;
            m_moveState = MS_FORWARD;
      }
      */
      else
      {
          wallJumpDir = 0.0f;
          wallRun = 0;
          hoverJump = false;
          ledgeGrab = false;
          //swingState = 0;
          if((dashSoundPlaying==TRUE)&&(shiftHeld==false))
          {
              gSoundManager.stop(m_dashSound);
              dashSoundPlaying = FALSE;
          }
      }

    switch(m_moveState)
    {
      case MS_STOP:
      {
        m_fSpeed = 0.0f;
        jumpSpeedRatio = 0.0f;
      } break;
      case MS_FORWARD:
      {
        m_fSpeed = m_maxSpeed * m_speedRatio;
      } break;
      case MS_BACKWARD:
      {
        m_fSpeed = -m_maxSpeed * m_speedRatio;
      } break;
    }
  } // end if (m_planeState == PS_FLYING)
  if (m_planeState == PS_CRASHING)
  {
      m_eaAngularVelocity[0].bank = 1.0f;
      Vector3 normVelocity = m_velocity;
      normVelocity.normalize();
      m_eaOrient[0].pitch = -(float)asin((double)(normVelocity.y));
      float gravity =  -10.0f;
      
      m_fSpeed = 0.0f;
      m_velocity.y += gravity * dt;      
      m_v3Position[0] += m_velocity * dt;  
  }

  // Update propeller orientation

  //m_eaOrient[1].bank += kPi * dt * 4.0f;
  

  // Move it
  
  GameObject::move(dt);

      if(dashSoundPlaying==TRUE)
      {
            planeOrient.pitch = 0.15f;
      }


  // flag that m_reticleLockedOn is out of date
  m_reticleLockOnUpdated = false; 

  // tell the smoke particle system where the plane is if there is smoke
  if (m_smokeID != -1)
    gParticle.setSystemPos(m_smokeID, transformObjectToInertial(m_enginePosition));
  
}

void PlaneObject::reset()
{
  m_isPlaneAlive = true;
  m_v3Position[0].zero();
  m_eaOrient[0].identity();
  m_eaOrient[1].identity();
  m_fSpeed = 0.0f;
  m_ySpeed = 0.0f;
  m_maxSpeed = 3.0f;
  m_turnState = TS_STRAIGHT;
  m_pitchState = PS_LEVEL;
  m_moveState = MS_STOP;
}



void PlaneObject::inputTurnLeft(float turnRatio)
{
  EulerAngles &planeOrient = m_eaOrient[0];
  EulerAngles camOri = gStatePlaying.cameraOrient();
  float planeHead = planeOrient.heading;
  float camHead = camOri.heading;

  float fplaneHead = planeHead*10000.0f;
  int planeOff = (int)fplaneHead;
  planeOff = planeOff/62830;
  if(planeOff<0)
      planeOff++;
  camHead = camHead + ((float)planeOff*6.283f);
  if(planeHead-camHead>4.71225f)
  {
      camHead = camHead + 6.283f;
  }
  else if(planeHead-camHead<-4.71225f)
  {
      camHead = camHead - 6.283f;
  }
  /*
  EulerAngles &planeOrient = m_eaOrient[0];
  float fplaneHead = planeOrient.heading * 10000.0f;
  int planeHead = (int)fplaneHead;
  planeHead = planeHead % (2*31415);

  EulerAngles camOri = gStatePlaying.cameraOrient();
  float fcamHead = camOri.heading * 10000.0f;
  int camHead = (int)fcamHead;
  camHead = camHead % (2*31415);
  */
  if(camHead < (planeHead + 1.5f))
  {
    m_turnRate = 12.0f;
  }
  else
    m_turnRate = 0.5f;
  m_turnState = TS_LEFT;
  m_speedRatio = turnRatio;
  m_moveState = m_moveState == MS_BACKWARD ? MS_STOP : MS_FORWARD;
}

void PlaneObject::inputTurnRight(float turnRatio)
{
  EulerAngles &planeOrient = m_eaOrient[0];
  EulerAngles camOri = gStatePlaying.cameraOrient();
  float planeHead = planeOrient.heading;
  float camHead = camOri.heading;

  float fplaneHead = planeHead*10000.0f;
  int planeOff = (int)fplaneHead;
  planeOff = planeOff/62830;
  if(planeOff<0)
      planeOff++;
  camHead = camHead + ((float)planeOff*6.283f);
  if(planeHead-camHead>4.71225f)
  {
      camHead = camHead + 6.283f;
  }
  else if(planeHead-camHead<-4.71225f)
  {
      camHead = camHead - 6.283f;
  }
  /*
  EulerAngles &planeOrient = m_eaOrient[0];
  float fplaneHead = planeOrient.heading * 10000.0f;
  int planeHead = (int)fplaneHead;
  planeHead = planeHead % (2*31415);

  EulerAngles camOri = gStatePlaying.cameraOrient();
  float fcamHead = camOri.heading * 10000.0f;
  int camHead = (int)fcamHead;
  camHead = camHead % (2*31415);
  */
  if(camHead > (planeHead - 1.5f))
  {
    m_turnRate = 12.0f;
  }
  else
    m_turnRate = 0.5f;
  m_turnState = TS_RIGHT;
  m_speedRatio = turnRatio;
  m_moveState = m_moveState == MS_BACKWARD ? MS_STOP : MS_FORWARD;
}

void PlaneObject::inputStraight()
{
  m_turnState = TS_STRAIGHT;
}

void PlaneObject::inputClimb(float climbRatio)
{
  m_pitchRate = climbRatio;
  m_pitchState = m_pitchState == PS_DIVE ? PS_LEVEL : PS_CLIMB;
}

void PlaneObject::inputDive(float diveRatio)
{
  m_pitchRate = diveRatio;
  m_pitchState = m_pitchState == PS_CLIMB ? PS_LEVEL : PS_DIVE;
}

void PlaneObject::inputLevel()
{
  m_pitchState = PS_LEVEL;
}

void PlaneObject::inputSpeedDown()
{
  m_maxSpeed -= 1.0f;
}

void PlaneObject::inputSpeedUp()
{
  m_maxSpeed += 1.0f;
}

void PlaneObject::inputForward(float speed)
{
  EulerAngles &planeOrient = m_eaOrient[0];
  EulerAngles camOri = gStatePlaying.cameraOrient();
  float planeHead = planeOrient.heading;
  float camHead = camOri.heading;

  float fplaneHead = planeHead*10000.0f;
  int planeOff = (int)fplaneHead;
  planeOff = planeOff/62830;
  if(planeOff<0)
      planeOff++;
  camHead = camHead + ((float)planeOff*6.283f);
  if(planeHead-camHead>4.71225f)
  {
      camHead = camHead + 6.283f;
  }
  else if(planeHead-camHead<-4.71225f)
  {
      camHead = camHead - 6.283f;
  }
  /*
  EulerAngles &planeOrient = m_eaOrient[0];
  float fplaneHead = planeOrient.heading * 10000.0f;
  int planeHead = (int)fplaneHead;
  planeHead = planeHead % (2*31415);

  EulerAngles camOri = gStatePlaying.cameraOrient();
  float fcamHead = camOri.heading * 10000.0f;
  int camHead = (int)fcamHead;
  camHead = camHead % (2*31415);
  */

  if(planeHead-camHead>.15f)
  {
      m_turnRate = 12.0f;
      m_turnState = TS_LEFT;
  }
  else if(camHead-planeHead>.15f)
  {
      m_turnRate = 12.0f;
      m_turnState = TS_RIGHT;
  }
  else
  {
      m_turnRate = 1.0f;
      m_turnState = TS_STRAIGHT;
  }
  
  m_speedRatio = speed;
  m_moveState = m_moveState == MS_BACKWARD ? MS_STOP : MS_FORWARD;
}

void PlaneObject::inputBackward(float speed)
{
  EulerAngles &planeOrient = m_eaOrient[0];
  EulerAngles camOri = gStatePlaying.cameraOrient();
  float planeHead = planeOrient.heading;
  float camHead = camOri.heading;

  float fplaneHead = planeHead*10000.0f;
  int planeOff = (int)fplaneHead;
  planeOff = planeOff/62830;
  if(planeOff<0)
      planeOff++;
  camHead = camHead + ((float)planeOff*6.283f);
  if(planeHead-camHead>4.71225f)
  {
      camHead = camHead + 6.283f;
  }
  else if(planeHead-camHead<-4.71225f)
  {
      camHead = camHead - 6.283f;
  }
  /*
  EulerAngles &planeOrient = m_eaOrient[0];
  float fplaneHead = planeOrient.heading * 10000.0f;
  int planeHead = (int)fplaneHead;
  planeHead = planeHead % (2*31415);

  EulerAngles camOri = gStatePlaying.cameraOrient();
  float fcamHead = camOri.heading * 10000.0f;
  int camHead = (int)fcamHead;
  camHead = camHead % (2*31415);
  */

  if(planeHead-camHead>.15f)
  {
      m_turnRate = 12.0f;
      m_turnState = TS_LEFT;
  }
  else if(camHead-planeHead>.15f)
  {
      m_turnRate = 12.0f;
      m_turnState = TS_RIGHT;
  }
  else
  {
      m_turnRate = 1.0f;
      m_turnState = TS_STRAIGHT;
  }
  
  m_speedRatio = speed;
  m_moveState = m_moveState == MS_FORWARD ? MS_STOP : MS_BACKWARD;
}

void PlaneObject::inputStop()
{
  m_moveState = MS_STOP;
}

void PlaneObject::inputFire()
{
  const float BulletsPerSecond = 10.0f;
  
  int time = gRenderer.getTime();
  if ((float)(time - m_timeSinceFired)/1000.0f < 1.0f/BulletsPerSecond)
    return;

  m_timeSinceFired = time;
  
  // test terrain ray intersect
  Vector3 bulletPos = getPosition();
  Vector3 bulletDir = Vector3(0, 0, 1);
  Vector3 gunPos = m_gunPosition;

  RotationMatrix r1;
  r1.setup(getOrientation());
  
  bulletDir = r1.objectToInertial(bulletDir);
  gunPos = r1.objectToInertial(gunPos);
  gunPos += getPosition();
  Vector3 intersectPoint = Vector3::kZeroVector;
  
  unsigned int bulletID = gGame.m_statePlaying.m_objects->spawnBullet(gunPos,getOrientation());
  unsigned int mfID = gParticle.createSystem("muzzlefire");
  gParticle.setSystemPos(mfID, gGame.m_statePlaying.m_objects->getObjectPointer(bulletID)->getPosition());

  int gunSoundInstance = gSoundManager.requestInstance(m_gunSound);
  gSoundManager.setPosition(m_gunSound, gunSoundInstance, getPosition());
  gSoundManager.setVelocity(m_gunSound, gunSoundInstance, -m_velocity );
  gSoundManager.play(m_gunSound, gunSoundInstance);
  gSoundManager.releaseInstance(m_gunSound, gunSoundInstance);

  // get ray trace up' and right'
  Vector3 v1 = bulletDir;
  v1.z = (10.0f + v1.z) * 20.0f; // pick a vector that is not bulletDir
  Vector3 up = Vector3::crossProduct(bulletDir, v1);
  up.normalize();
  float traceWidth = 0.2f;
  up *= traceWidth;

  Vector3 right = Vector3::crossProduct(bulletDir, up);
  right.normalize();
  right *= traceWidth;

  Vector3 traceVector = bulletDir * gBulletRange;

  if(gGame.m_statePlaying.terrain->rayIntersect(gunPos, traceVector, intersectPoint))
  {
    if(intersectPoint.y > gGame.m_statePlaying.water->getWaterHeight())
    {
      int tmpHndl = gParticle.createSystem("bulletdust");
      gParticle.setSystemPos(tmpHndl, intersectPoint);
    }
    else
    {
      float dy = bulletPos.y - gGame.m_statePlaying.water->getWaterHeight();
      if(bulletDir.y < 0)
        dy *= -1.0f;
      intersectPoint = bulletPos + (bulletDir * (dy / bulletDir.y));

      int tmpHndl = gParticle.createSystem("bulletspray");
      gParticle.setSystemPos(tmpHndl, intersectPoint);
    }
  }

}


void PlaneObject::killPlane()
{
  m_planeState = PS_DEAD;
  m_isPlaneAlive = false;
}

bool PlaneObject::isPlaneAlive() const
{
  return m_isPlaneAlive;
}

int PlaneObject::getHP() const
{
  return m_hp;
}

int PlaneObject::getMaxHP() const
{
  return m_maxHP;
}

void PlaneObject::setMaxHP(int maxHP, bool recoverAll)
{
  if(maxHP <= 0)
    return;
  m_maxHP = maxHP;
  if(recoverAll)
    this->recoverAll(); // the function, not the parameter
}

void PlaneObject::damage(int hp)
{
  // if god mode is on leave 
  if (!takeDamage)
    return;

  m_hp -= hp;
  setTextureAndSmoke(); // change to a texture with more damange on it
        
  if(m_isPlaneAlive && m_hp <= 0)
  {
    m_planeState = PS_CRASHING;
    m_velocity = Vector3::kForwardVector;
    m_eaOrient[0].pitch = degToRad(20);
    RotationMatrix r; r.setup(m_eaOrient[0]);
    m_velocity = r.objectToInertial(m_velocity);
    m_velocity *= m_maxSpeed * m_speedRatio * 20.0f;
  }
}

void PlaneObject::recover(int hp)
{
  m_hp += hp;
  if(m_hp > m_maxHP)
    m_hp = m_maxHP;
}

void PlaneObject::recoverAll()
{
  m_hp = m_maxHP;
}

void PlaneObject::setTextureAndSmoke()
{
  
  int textureIndex = m_hp; // index into m_allTextures array
  int smokeIndex = m_hp; // index into m_allParticles array
  /*
  // make sure the indicies are in range
  if (textureIndex >= (int)m_allTextures.size())
    textureIndex = (int)m_allTextures.size() - 1;
  if (textureIndex < 0) textureIndex = 0;
  */
  // make sure the indicies are in range
  if (smokeIndex >= (int)m_allParticles.size())
    smokeIndex = (int)m_allParticles.size() - 1;
  if (smokeIndex < 0) smokeIndex = 0;
  /*
    // set texture 
  int numParts = m_pModel->getPartCount();
  for (int a = 0; a < numParts; a++)
    m_pModel->setPartTextureName(a,m_allTextures[textureIndex].c_str());
   m_pModel->cache();
   */
   // remove previous smoke system
   if (m_smokeID != -1)
     gParticle.killSystem(m_smokeID);
   if (m_allParticles[smokeIndex] != "")
   {
     m_smokeID = gParticle.createSystem(m_allParticles[smokeIndex]);
     gParticle.setSystemPos(m_smokeID, transformObjectToInertial(m_enginePosition));
   }
}

//sets the planes heading to a direction based on directional input dInput and the camera orientation
//moves the plane forward with a 1.0f speed
void PlaneObject::movePlayer(float dInput, float speedRatio)
{
  EulerAngles &planeOrient = m_eaOrient[0];
  EulerAngles camOri = gStatePlaying.cameraOrient();
  planeOrient.heading = camOri.heading + dInput;
  m_turnState = TS_STRAIGHT;
  //if(shiftHeld==TRUE)
  //    speedRatio = speedRatio + 1.5f;
  m_speedRatio = speedRatio;
  m_moveState = m_moveState == MS_BACKWARD ? MS_STOP : MS_FORWARD;
}

//returns the DI numpad conversion of the binary int representing keys
int PlaneObject::binaryConv(int bin)
{
	int num = 0;
	switch(bin)
	{
	case 0:
		//nothing held
		num = 0;
		break;
	case 1:
		//down
		num = 2;
		break;
	case 2:
		//left
		num = 4;
		break;
	case 3:
		//down+left
		num = 1;
		break;
	case 4:
		//right
		num = 6;
		break;
	case 5:
		//down+right
		num = 3;
		break;
	case 6:
		//left+right
		num = 0;
		break;
	case 7:
		//down+left+right
		num = 2;
		break;
	case 8:
		//up
		num = 8;
		break;
	case 9:
		//down+up
		num = 0;
		break;
	case 10:
		//up+left
		num = 7;
		break;
	case 11:
		//up+left+down
		num = 4;
		break;
	case 12:
		//up+right
		num = 9;
		break;
	case 13:
		//up+right+down
		num = 6;
		break;
	case 14:
		//up+left+right
		num = 8;
		break;
	case 15:
		//up+left+right+down
		num = 0;
		break;
	default:
		num = 0;
		break;
	}
	return num;
}

void PlaneObject::stopJump()
{
    if((m_ySpeed>0.5f)&&(wallJumpDir==0.0f))
        m_ySpeed = m_ySpeed - 0.5f;
}

void PlaneObject::jumpPlayer(float ySpeed)
{
    if(((m_ySpeed<2.0f)&&(gRenderer.getTime()>(jumpTimer+0)))&&(swingState==0))
    {
	if(inAir==FALSE)
    {
		m_ySpeed = ySpeed;
        jumpSpeedRatio = m_speedRatio;
        EulerAngles &planeOrient = m_eaOrient[0];
        jumpHeading = planeOrient.heading;
        jumpTimer = gRenderer.getTime();
        int jumpSoundInstance = gSoundManager.requestInstance(m_jumpSound);
        gSoundManager.setPosition(m_jumpSound, jumpSoundInstance, getPosition());
        gSoundManager.setVelocity(m_jumpSound, jumpSoundInstance, -m_velocity );
        gSoundManager.play(m_jumpSound, jumpSoundInstance);
        gSoundManager.releaseInstance(m_jumpSound, jumpSoundInstance);
        canHover = false;
    }
    else if(wallJumpDir!=0.0f)
    {
        if(ledgeGrab==false)
        {
            m_ySpeed = ySpeed;
            jumpSpeedRatio = 1.0f;
            EulerAngles &planeOrient = m_eaOrient[0];
            if(wallRun==1)
            {
                planeOrient.heading = planeOrient.heading - (kPi * 0.3f);
                jumpSpeedRatio = 2.0f;
            }
            else if(wallRun==2)
            {
                planeOrient.heading = planeOrient.heading + (kPi * 0.3f);
                jumpSpeedRatio = 2.0f;
            }
            else
                planeOrient.heading = (kPi * wallJumpDir);
            wallJumpDir = 0.0f;
            wallRun = 0;
            jumpHeading = planeOrient.heading;
            jumpTimer = gRenderer.getTime();
            int jumpSoundInstance = gSoundManager.requestInstance(m_jumpSound);
            gSoundManager.setPosition(m_jumpSound, jumpSoundInstance, getPosition());
            gSoundManager.setVelocity(m_jumpSound, jumpSoundInstance, -m_velocity );
            gSoundManager.play(m_jumpSound, jumpSoundInstance);
            gSoundManager.releaseInstance(m_jumpSound, jumpSoundInstance);
            canHover = false;
        }
    }
    else if(((energy>20.0f)&&(hoverJump==false))&&((m_ySpeed<-0.5f)&&(wallJumpDir==0.0f)))
    {
        hoverJump = true;
        m_ySpeed = 2.5f;
        energy = energy - 10.0f;
        jumpTimer = gRenderer.getTime();
        int jumpSoundInstance = gSoundManager.requestInstance(m_jumpSound);
        gSoundManager.setPosition(m_jumpSound, jumpSoundInstance, getPosition());
        gSoundManager.setVelocity(m_jumpSound, jumpSoundInstance, -m_velocity );
        gSoundManager.play(m_jumpSound, jumpSoundInstance);
        gSoundManager.releaseInstance(m_jumpSound, jumpSoundInstance);
    }
    }
}

void PlaneObject::shiftTimer()
{
        if(shiftHeld==FALSE)
        {
            if(energy<100.0f)
                energy = energy + 0.5f;
            if(energy>25.0f)
                energyFlag=TRUE;
            if((dashSoundPlaying==TRUE)&&(spaceHeld==false))
            {
                gSoundManager.stop(m_dashSound);
                dashSoundPlaying = FALSE;
            }
        }
        else if(((shiftHeld==TRUE)&&(swingState==0))&&(ledgeGrab==false))
        {
            if(energy>0.0f)
                energy = energy - 1.5f;
            if(dashSoundPlaying==FALSE)
            {
                dashSoundPlaying = TRUE;
                int dashSoundInstance = gSoundManager.requestInstance(m_dashSound);
                gSoundManager.setPosition(m_dashSound, dashSoundInstance, getPosition());
                gSoundManager.setVelocity(m_dashSound, dashSoundInstance, -m_velocity );
                gSoundManager.play(m_dashSound, dashSoundInstance);
                gSoundManager.releaseInstance(m_dashSound, dashSoundInstance);
            }
            EulerAngles &planeOrient = m_eaOrient[0];
            float tdi = 0.0f;
            if(aHeld)
                tdi -= (kPi*0.1f);
            if(dHeld)
                tdi += (kPi*0.1f);
            movePlayer(tdi, 2.0f);
        }
        if(energy<1.0f)
            energyFlag = FALSE;
}


float PlaneObject::simplifyAngle(float angle)
{
    int iangle = int(angle * 100000.0f);
    int tPi = int(kPi * 200000.0f);
    int x = iangle % tPi;
    if((iangle<0)&&(x>0))
        x = -x;
    if(x<0)
        x = x + tPi;
    float result = float(x) * .00001f;
    return result;
}


void PlaneObject::hoverTimer()
{
        if(spaceHeld==FALSE)
        {
            if((dashSoundPlaying==TRUE)&&(shiftHeld==false))
            {
                gSoundManager.stop(m_dashSound);
                dashSoundPlaying = FALSE;
            }
        }
        else if((hoverJump==true)&&((m_ySpeed<-0.5f)&&(gRenderer.getTime()>(swingTimer+300))))
        {
            if((swingState==0)&&(ledgeGrab==false))
            {
                if(energy>0.0f)
                    energy = energy - 1.2f;
                if(dashSoundPlaying==FALSE)
                {
                    dashSoundPlaying = TRUE;
                    int dashSoundInstance = gSoundManager.requestInstance(m_dashSound);
                    gSoundManager.setPosition(m_dashSound, dashSoundInstance, getPosition());
                    gSoundManager.setVelocity(m_dashSound, dashSoundInstance, -m_velocity );
                    gSoundManager.play(m_dashSound, dashSoundInstance);
                    gSoundManager.releaseInstance(m_dashSound, dashSoundInstance);
                }
            }
        }
}

void PlaneObject::setSwingPos(Vector3 pos)
{
    swingPos = pos;
}

Vector3 PlaneObject::getRadialPos(Vector3 sPos, int dir, float radius, float angleRatio)
{
    Vector3 rPos = sPos;
    float rAngle = angleRatio * (kPi * 2.0f);
    switch(dir)
    {
    case 1:
        rPos.x += cos(rAngle) * radius;
        break;
    case 2:
        rPos.z -= cos(rAngle) * radius;
        break;
    case 3:
        rPos.x -= cos(rAngle) * radius;
        break;
    case 4:
        rPos.z += cos(rAngle) * radius;
        break;
    default:
        break;
    }
    rPos.y += sin(rAngle) * radius;
    return rPos;
}

EulerAngles PlaneObject::getRadialOrient(float angleRatio, int dir)
{
    EulerAngles rOrient = EulerAngles::kEulerAnglesIdentity;
    rOrient.heading = (float)dir * kPiOver2;
    rOrient.pitch = (angleRatio * (kPi * -2.0f)) - kPiOver2;
    return rOrient;
}

void PlaneObject::setSwingTimer(int time)
{
    swingTimer = time;
}