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

/// \file StatePlaying.cpp
/// \brief Code for the StatePlaying class.

/// Edited by the SAGE team, 2005
/// Last updated June 13, 2006

#include "StatePlaying.h"
#include "game.h"
#include "input/input.h"
#include <algorithm>
#include "DirectoryManager/DirectoryManager.h"
#include "Common/MathUtil.h"
#include "Common/Renderer.h"
#include "Common/Random.h"
#include "Common/RotationMatrix.h"
#include "Console/Console.h"
#include "Graphics/ModelManager.h"
#include "Input/Input.h"
#include "Particle/ParticleEngine.h"
#include "Sound/SoundManager.h"
#include "Terrain/Terrain.h"
#include "WindowsWrapper/WindowsWrapper.h"
#include "Ned3DObjectManager.h"
#include "Game.h"
#include "Common/Camera.h"
#include "Common/Camera.cpp"
#include "common/matrix4x3.h"
#include "common/vector3.h"
#include "DerivedModels\AnimatedModel.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "Ned3DObjectManager.h"
extern CRandom Random;

Model* m_SkyDome;
Model* m_revSkyDome;
Model* cloudScroll;
Model* tcloudScroll;
Model* scloudScroll;
Model* stcloudScroll;
Model* m_botDome;
Model* m_HUD;
//Model* m_shad;
AnimatedModel* m_shada;
Model* m_Notch;
StandardVertexBuffer* m_vbuf;
extern char fileDir[MAX_PATH];
extern Ned3DObjectManager* m_objects; ///< Ned Object Manager
extern std::vector<ModelNode*> m_vModels;
extern bool fromMenu;
extern bool gameStart;
extern bool fromDeath;
extern bool fromLevel;
extern bool fromVictory;
extern bool hitPause;
extern XINPUT_STATE state; //State for xinput controller

 

/// Global function that can be called from the console

bool StatePlaying::consoleChangeFOV(ParameterList* params,std::string* errorMessage)
{
 
	return true;
}

bool StatePlaying::consoleSetFollowCamera(ParameterList* params,std::string* errorMessage)
{
  gGame.m_currentCam = gGame.m_statePlaying.m_tetherCamera;
	return true;
}

bool StatePlaying::consoleSetCameraTarget(ParameterList* params,std::string* errorMessage)
{
  unsigned int obj = gGame.m_statePlaying.m_objects->getObjectID(params->Strings[0]);
  if(obj == 0)
  {
    *errorMessage = "Object not found.";
    return false;
  }
  if(gGame.m_statePlaying.m_tetherCamera == NULL)
  {
    *errorMessage = "Tether camera not initialized.";
    return false;
  }
  gGame.m_statePlaying.m_tetherCamera->setTargetObject(obj);
  gGame.m_statePlaying.m_tetherCamera->reset();
  return true;
}

bool StatePlaying::consoleGodMode(ParameterList* params,std::string* errorMessage)
{
  PlaneObject::takeDamage = !params->Bools[0];
  return true;
}



StatePlaying::StatePlaying():
terrain(NULL),
water(NULL),
m_objects(NULL),
m_tetherCamera(NULL),
m_SkyDome(NULL),
m_cloudScroll(NULL),
m_tcloudScroll(NULL),
m_scloudScroll(NULL),
m_stcloudScroll(NULL),
m_revSkyDome(NULL),
m_botDome(NULL),
m_HUD(NULL),
red(0),
green(0),
blue(0),
//m_shad(NULL),
m_shada(NULL),
m_Notch(NULL),
m_texHandle(0),
m_texHandle2(0),
m_texHandle3(0),
m_texHandle4(0),
m_texTargetHandle(0),
m_texWidth(0.0f),
m_texHeight(0.0f),
energyScale(0.0f),
m_failedInstance(SoundManager::NOINSTANCE),
m_nCurLvl(0)
{
	m_nMaxLvl = 1;
	char wrkDir[MAX_PATH];
	_getcwd(wrkDir, _MAX_PATH);
	char directory[MAX_PATH];
	sprintf_s(directory,"%s\\Levels\\LevelCount.txt",wrkDir);

	std::ifstream readFile;
	readFile.open(directory);
	//Read model data
	char copyLine[500];
	if(readFile.is_open())
	{
		int open  = 0;
	}
	readFile.getline(copyLine,500);
	readFile.close();

	char ch = copyLine[0];
	int j = 0;
	int num2 = 0;
	while((ch >= '0' && ch <= '9'))
	{
		num2 = num2*10 + ch - '0';
		ch = copyLine[++j];
	}
	m_nMaxLvl = num2;
}

void StatePlaying::initiate()
{
  float farClippingPlane = 2000.0f;

  // Set far clipping plane
  gRenderer.setNearFarClippingPlanes(1.0f,farClippingPlane);    

  m_SkyDome = new Model();
  m_SkyDome->importS3d("dome1.s3d");
  m_SkyDome->cache();

  m_revSkyDome = new Model();
  m_revSkyDome->importS3d("revdome1.s3d");
  m_revSkyDome->cache();

  m_botDome = new Model();
  m_botDome->importS3d("domebot.s3d");
  m_botDome->cache();

  m_HUD = new Model();
  m_HUD->importS3d("pillar.s3d");
  m_HUD->cache();

  m_cloudScroll = new Model();
  m_cloudScroll->importS3d("screen.s3d");
  m_cloudScroll->cache();

  m_tcloudScroll = new Model();
  m_tcloudScroll->importS3d("screenrev.s3d");
  m_tcloudScroll->cache();

  m_scloudScroll = new Model();
  m_scloudScroll->importS3d("screens.s3d");
  m_scloudScroll->cache();

  m_stcloudScroll = new Model();
  m_stcloudScroll->importS3d("screenrevs.s3d");
  m_stcloudScroll->cache();

  m_texWidth = 350.0f;
  m_texHeight = 350.0f;

  


  m_texHandle = gRenderer.cacheTexture("flare1.tga", true);
  m_texHandle2 = gRenderer.cacheTexture("flare2.tga", true);
  m_texHandle3 = gRenderer.cacheTexture("flare3.tga", true);
  m_texHandle4 = gRenderer.cacheTexture("flare4.tga", true);
  
  /*
  m_shad = new Model();
  m_shad->importS3d("shadow.s3d");
  m_shad->cache();
  */

  std::list<const char*> frames;

  frames.push_back("shadow.s3d");
  frames.push_back("shadow1.s3d");

  m_shada = new AnimatedModel((int)frames.size());
  m_shada->importS3d(frames, true);

  m_vbuf = m_shada->getNewVertexBuffer();

  m_Notch = new Model();
  m_Notch->importS3d("notch.s3d");
  m_Notch->cache();

  m_objects = new Ned3DObjectManager();	
  m_objects->setNumberOfDeadFrames(2);
  m_tetherCamera = new TetherCamera(m_objects);
	
  // Create terrain
  terrain = new Terrain(8,"terrain.xml"); //powers of two for terrain size
  m_objects->spawnTerrain(terrain);
  
  // Load models
  m_objects->setModelManager(gModelManager);
  gModelManager.importXml("models.xml");

  // Loads game objects like the crows, plane, and silo
	resetGame();
  
  // set fog
  gRenderer.setFogEnable(true);
  gRenderer.setFogDistance(farClippingPlane - 1000.0f,farClippingPlane);
  gRenderer.setFogColor(MAKE_ARGB(0,60,180,254));

  // set lights
  gRenderer.setAmbientLightColor(MAKE_ARGB(255,100,100,100));
  gRenderer.setDirectionalLightColor(0XFFFFFFFF);
  Vector3 dir = Vector3(5.0f,-5.0f, 6.0f);
  dir.normalize();
  gRenderer.setDirectionalLightVector(dir);
  	
  // Create water now that we know what camera to use
  float fov = degToRad(gGame.m_currentCam->fov);
  water = new Water(fov, farClippingPlane, "water.xml");
  
  m_objects->spawnWater(water);

	// aquire sounds  
  gSoundManager.setDopplerUnit(0.0001f); // sound factors
  // get windmill sound
  /*
  m_windmillSound = gSoundManager.requestSoundHandle("windmill2.wav");
  m_windmillSoundInstance = gSoundManager.requestInstance(m_windmillSound);  
  */
  // add console commands  
  gConsole.addFunction("camerafollow","",consoleSetFollowCamera);
  gConsole.addFunction("cameratarget","s",consoleSetCameraTarget);
  gConsole.addFunction("godmode","b",consoleGodMode);

}

void StatePlaying::shutdown()
{
  delete m_tetherCamera; m_tetherCamera = NULL;
  delete water; water = NULL;
  delete terrain; terrain = NULL;
  delete m_objects; m_objects = NULL;
  delete m_SkyDome; m_SkyDome = NULL;
  delete m_cloudScroll; m_cloudScroll = NULL;
  delete m_tcloudScroll; m_tcloudScroll = NULL;
  delete m_scloudScroll; m_scloudScroll = NULL;
  delete m_stcloudScroll; m_stcloudScroll = NULL;
  delete m_revSkyDome; m_revSkyDome = NULL;
  delete m_botDome; m_botDome = NULL;
  delete m_HUD; m_HUD = NULL;
  //delete m_shad; m_shad = NULL;
  delete m_shada; m_shada = NULL;
  delete m_vbuf; m_vbuf = NULL;
  delete m_Notch; m_Notch = NULL;

	while(!m_vModels.empty())
	{
		delete m_vModels.back()->buffer;
		delete m_vModels.back()->model;
		delete m_vModels.back();
		m_vModels.back() = NULL;
		m_vModels.pop_back();
	}
}

void StatePlaying::exitState()
{
  // stop windmill sound
  //gSoundManager.stop(m_windmillSound, m_windmillSoundInstance);
}

void StatePlaying::enterState()
{
  // start up the windmill sound
  //gSoundManager.play(m_windmillSound, m_windmillSoundInstance, true);
}

void StatePlaying::process(float dt)
{ 
  PlaneObject *planeObject = m_objects->getPlaneObject();

  // this should never happen but if it does leave
  if (planeObject == NULL) 
    return; 
    
  gConsole.process();
  // call process and move on all objects in the object manager
  m_objects->update(dt); 
    
  // process escape key and space bar
  processInput();

  // update location of camera
  processCamera(dt);
    
  // allow water to process per frame movements
  water->process(dt);
 
  // as soon as the plane crashes, start the timer
  if (planeObject->isPlaneAlive() == false && m_planeCrashed == false)
  {
    m_planeCrashed = true; // set that the plane has crashed
    m_timeSinceCrashed = 0.0f;
    m_crowID = -1;   
  }

  // once the timer hits 3 seconds, make the camera follow a crow
  if (m_planeCrashed)
  {
    m_timeSinceCrashed += dt;
    if (m_timeSinceCrashed >= 3.0f && m_crowID == -1)
    {
      // make the camera follow a crow around
      m_crowID = m_objects->getCrow();
      if (m_crowID != -1)
        m_tetherCamera->setTargetObject(m_crowID);

      // play the failed sound
      m_failedSound = gSoundManager.requestSoundHandle("Failed.wav");
      m_failedInstance = gSoundManager.requestInstance(m_failedSound);
      //if(m_failedInstance != SoundManager::NOINSTANCE)
      //{
      //  gSoundManager.setToListener(m_failedSound,m_failedInstance);
      //  //gSoundManager.play(m_failedSound,m_failedInstance);
      //}
    }
    /*else if(m_failedInstance != SoundManager::NOINSTANCE)
      gSoundManager.setToListener(m_failedSound,m_failedInstance);*/
  }

  // render reflection
  if (Water::m_bReflection)
  {
    // render water reflection    
    Plane plane( 0, 1, 0, -water->getWaterHeight());
    //get water texture ready  
    water->m_reflection.beginReflectedScene(plane);
    renderScene(true);
    water->m_reflection.endReflectedScene();    
  }

}

void StatePlaying::renderScreen()
{
  // render the entire scene
  renderScene();
  
  PlaneObject* plane = m_objects->getPlaneObject();
  if (plane != NULL)
  {
    //render plane reticles    
    plane->renderReticle(15.0f, 1.3f);
    plane->renderReticle(8.0f, 1.0f);
    
    if (plane->isPlaneAlive() == false)
    {
      int textY = gRenderer.getScreenY()/2;
      IRectangle rect = IRectangle(0,textY,gRenderer.getScreenX()-1, textY + 30);
      gRenderer.drawText("Press \"Space Bar\" or start to Respawn",&rect, eTextAlignModeCenter, false);
    }
  }

  // render FPS and console ontop of everything
  gGame.GameBase::renderConsoleAndFPS();
}

void StatePlaying::renderScene(bool asReflection)
{
  PlaneObject* plane = m_objects->getPlaneObject();

  EulerAngles fCamOri = gGame.m_currentCam->cameraOrient;
  fCamOri.heading = plane->simplifyAngle(fCamOri.heading);

  float sunPitch = kPi * (-0.25f);
  float sunHeading = kPi * 0.5f;

  Vector3 flarePos = Vector3(0.0f,0.0f,0.0f);

  float flarey = ((fCamOri.pitch - sunPitch) / (kPi * 0.25f));
  float flarex = ((fCamOri.heading - sunHeading) / (kPi * 0.5f));

  float flareSize = ((10.0f * flarey) * (10.0f * flarey)) + ((10.0f * flarex) * (10.0f * flarex));
  flareSize = sqrt(flareSize);
  flareSize = 7.07107f - flareSize;
  if(flareSize<0.0f)
      flareSize = 0.0f;
  flareSize = flareSize / 7.07107f;
  //at this point flare size is between 0.0 and 1.0 depending on proximity of vision to sun

  float fWidth = m_texWidth * flareSize;
  float fHeight = m_texHeight * flareSize;

  bool zBufferState = gRenderer.getZBufferEnable();
	// turn off z buffer
	gRenderer.setZBufferEnable(false);

	flarePos.y = (-flarey + 0.5f) * (float)gRenderer.getScreenY();
  flarePos.x = (-flarex + 0.5f) * (float)gRenderer.getScreenX();


	// push translation
	gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle4);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 7.0f,
		fHeight * 7.0f);

	// pop translation
	gRenderer.instancePop();

	flarePos.y = (float)gRenderer.getScreenY() - (((2.5f * flarey) + 0.5f) * (float)gRenderer.getScreenY());
  flarePos.x = (float)gRenderer.getScreenX() - (((2.5f * flarex) + 0.5f) * (float)gRenderer.getScreenX());


    gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 0.5f,
		fHeight * 0.5f);

	// pop translation
	gRenderer.instancePop();
    
  flarePos.y = (-flarey + 0.5f) * (float)gRenderer.getScreenY();
  flarePos.x = (-flarex + 0.5f) * (float)gRenderer.getScreenX();


	// push translation
	gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 7.0f,
		fHeight * 7.0f);

	// pop translation
	gRenderer.instancePop();

    gRenderer.setZBufferEnable(zBufferState);

  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(gGame.m_currentCam->cameraPos, EulerAngles(kPi * 0.5f,kPi * 0.25f,0));
  m_SkyDome->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);

  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(gGame.m_currentCam->cameraPos, EulerAngles(0,kPi,0));
  m_botDome->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);


  float dTimer = float(gRenderer.getTime() % 60000) / 60000.0f;
  
  gRenderer.setFogColor(MAKE_ARGB(0,red,green,blue));
  
  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(gGame.m_currentCam->cameraPos, EulerAngles(dTimer * kPi * 2.0f, 0.0f, 0.0f));
  //gRenderer.setSourceBlendMode(eSourceBlendModeSrcAlpha);
  //gRenderer.setDestBlendMode(eDestBlendModeInvSrcAlpha);
  //gRenderer.setARGB(MAKE_ARGB(dTimer, 255, 255, 255));
  m_revSkyDome->render();
  gRenderer.instancePop();
  //gRenderer.setARGB(MAKE_ARGB(0.0f, 0, 0, 0));
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);

  float cTimer = float(gRenderer.getTime() % 200000) / 200000.0f;


  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  Vector3 screenPos = gGame.m_currentCam->cameraPos;
  screenPos.y -= 5.0f;
  screenPos.x += ((cTimer - 0.5f) * 160.0f);
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_scloudScroll->render();
  gRenderer.instancePop();
  screenPos.x += 160.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_scloudScroll->render();
  gRenderer.instancePop();
  screenPos.x -= 320.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_scloudScroll->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);


  screenPos = gGame.m_currentCam->cameraPos;
  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  screenPos.y += 5.0f;
  screenPos.x += (((cTimer - 0.5f) * 160.0f) + 80.0f);
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_stcloudScroll->render();
  gRenderer.instancePop();
  screenPos.x += 160.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_stcloudScroll->render();
  gRenderer.instancePop();
  screenPos.x -= 320.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_stcloudScroll->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);


  cTimer = float(gRenderer.getTime() % 160000) / 160000.0f;


  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  screenPos = gGame.m_currentCam->cameraPos;
  screenPos.y -= 1.25f;
  screenPos.x += ((cTimer - 0.5f) * 3200.0f);
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_cloudScroll->render();
  gRenderer.instancePop();
  screenPos.x += 3200.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_cloudScroll->render();
  gRenderer.instancePop();
  screenPos.x -= 6400.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_cloudScroll->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);


  screenPos = gGame.m_currentCam->cameraPos;
  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  screenPos.y += 1.25f;
  screenPos.x += (((cTimer - 0.5f) * 3200.0f) + 1600.0f);
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_tcloudScroll->render();
  gRenderer.instancePop();
  screenPos.x += 3200.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_tcloudScroll->render();
  gRenderer.instancePop();
  screenPos.x -= 6400.0f;
  gRenderer.instance(screenPos, EulerAngles(0,0,0));
  m_tcloudScroll->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);

  


  /*
  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(gGame.m_currentCam->cameraPos, EulerAngles(0,0,0));
  m_revSkyDome->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);
  */
  
  
  


  terrain->render(); // render the terrain

  zBufferState = gRenderer.getZBufferEnable();
	// turn off z buffer
	gRenderer.setZBufferEnable(false);


	

      flarePos.y = (0.5f) * (float)gRenderer.getScreenY();
  flarePos.x = ((-2.0f * flarex) + 0.5f) * (float)gRenderer.getScreenX();


    gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle2);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth* 0.5f,
		fHeight * 0.5f);

	// pop translation
	gRenderer.instancePop();



  flarePos.x = (0.5f) * (float)gRenderer.getScreenX();
  flarePos.y = ((-2.0f * flarey) + 0.5f) * (float)gRenderer.getScreenY();


    gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle3);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 0.5f,
		fHeight * 0.5f);

	// pop translation
	gRenderer.instancePop();
  
    
  
  flarePos.y = (float)gRenderer.getScreenY() - (((2.0f * flarey) + 0.5f) * (float)gRenderer.getScreenY());
  flarePos.x = (float)gRenderer.getScreenX() - (((2.0f * flarex) + 0.5f) * (float)gRenderer.getScreenX());


    gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle4);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 1.5f,
		fHeight * 1.5f);

	// pop translation
	gRenderer.instancePop();

    flarePos.y = (0.1f * flarey + 0.5f) * (float)gRenderer.getScreenY();
    flarePos.x = (0.9f * flarex + 0.5f) * (float)gRenderer.getScreenX();


	// push translation
	gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle2);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 0.75f,
		fHeight * 0.75f);

	// pop translation
	gRenderer.instancePop();

    flarePos.y = (0.9f * flarey + 0.5f) * (float)gRenderer.getScreenY();
    flarePos.x = (0.1f * flarex + 0.5f) * (float)gRenderer.getScreenX();


	// push translation
	gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle4);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 0.75f,
		fHeight * 0.75f);

	// pop translation
	gRenderer.instancePop();

  flarePos.y = (0.5f * flarey + 0.5f) * (float)gRenderer.getScreenY();
  flarePos.x = (0.5f * flarex + 0.5f) * (float)gRenderer.getScreenX();


	// push translation
	gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle3);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 2.0f,
		fHeight * 2.0f);

	// pop translation
	gRenderer.instancePop();


  flarePos.y = ((1.25f * flarey) + 0.5f) * (float)gRenderer.getScreenY();
  flarePos.x = ((1.25f * flarex) + 0.5f) * (float)gRenderer.getScreenX();


    gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth * 1.5f,
		fHeight * 1.5f);

	// pop translation
	gRenderer.instancePop();




  flarePos.y = ((2.0f * flarey) + 0.5f) * (float)gRenderer.getScreenY();
  flarePos.x = ((2.0f * flarex) + 0.5f) * (float)gRenderer.getScreenX();


    gRenderer.instance(flarePos,EulerAngles::kEulerAnglesIdentity);
    		
	// set the texture to the background texture
	gRenderer.selectTexture(m_texHandle2);
	
    gRenderer.setARGB(0XFFFFFFFF);

	gRenderer.renderSprite(
		fWidth,
		fHeight);

	// pop translation
	gRenderer.instancePop();




  

    







  gRenderer.setZBufferEnable(zBufferState);

  
  m_objects->renderex(plane->getID());

    
  Vector3 pPos = plane->getPosition();
  EulerAngles sOrient = EulerAngles::kEulerAnglesIdentity;
  sOrient.pitch = sOrient.pitch + kPiOver2;


  Vector3 sPos = pPos;

  sPos.y = m_objects->getShadHeight();
  
  float yDif = pPos.y - sPos.y;
  if(yDif>25.0f)
      yDif = 25.0f;
  else if(yDif<0.0f)
      yDif = 0.0f;
  yDif = 1.0f - (yDif / 25.0f);
  
  m_shada->selectAnimationFrame(yDif, 0, *m_vbuf);

  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(sPos,sOrient);
  m_shada->render(m_vbuf);
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);
  
  plane->render();

  // render water
  if (asReflection == false)      
    water->render(gGame.m_currentCam->cameraPos, gGame.m_currentCam->cameraOrient.heading);
    
   //render particles
  gParticle.render(!asReflection);   



  EulerAngles hudOrient = gGame.m_currentCam->cameraOrient;
  



  energyScale = (50.0f - plane->energy) * 0.007f;

  hudOrient.pitch = hudOrient.pitch + energyScale;

  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(gGame.m_currentCam->cameraPos, hudOrient);
  m_HUD->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);

  gRenderer.setDepthBufferMode(false, false);
  gRenderer.setLightEnable(false);
  gRenderer.instance(gGame.m_currentCam->cameraPos, gGame.m_currentCam->cameraOrient);
  m_Notch->render();
  gRenderer.instancePop();
  gRenderer.setDepthBufferMode(true, true);
  gRenderer.setLightEnable(true);  
}

void StatePlaying::loadLevel()
{
	std::list<std::string> filenames;
	std::string directory;
	//gWindowsWrapper.showOpenFileDialog(false, "lvl Files (*.lvl)\0*.lvl\0", directory, filenames);
	// make sure the user didn't press cancel
	while(!m_vModels.empty())	//Clear current work.
	{
		delete m_vModels.back()->buffer;
		delete m_vModels.back()->model;
		delete m_vModels.back();
		m_vModels.back() = NULL;
		m_vModels.pop_back();
	}
	if(!fromMenu || m_nCurLvl == 0)
		m_nCurLvl++;
	if(m_nCurLvl > m_nMaxLvl)
	{
		m_nCurLvl = 1;
	}
	/*savePos.x = 0;
	savePos.y = 90;
	savePos.z = 0;*/
	//savePoint = false;
	char fullLevelPath[MAX_PATH];
	char partialPath[MAX_PATH];
	sprintf_s(partialPath,"Level%i.lvl",m_nCurLvl);
	sprintf_s(fullLevelPath,"%s\\Levels\\%s",fileDir,partialPath);

	std::ifstream readFile;
	readFile.open(fullLevelPath);
	//Read model data
	char copyLine[500];
	findTag("<models>",copyLine,&readFile,true);
	bool cont = true;
	cont = findTag("<mod>",copyLine,&readFile,false);
	while(strcmp(copyLine,"</models>") != 0 && cont)
	{
		
		Matrix4x3 tempMatrix;
		Vector3 rotVec;
		Vector3 scaleVec;
		char modName[500];
		readFile.getline(modName,500);
		for(int a = 0; a < 4; a++)
		{
			readFile.getline(copyLine,500);
			copyRow(&tempMatrix,copyLine,a);
		}
		readFile.getline(copyLine,500);
		copyRow(&rotVec,copyLine);
		readFile.getline(copyLine,500);
		copyRow(&scaleVec,copyLine);
		readFile.getline(copyLine,500);
		loadModelFromFile(tempMatrix,rotVec,scaleVec,modName);
		cont = findTag("<mod>",copyLine,&readFile,false);
	}
	readFile.close();
	//readFile.open
}

void StatePlaying::loadModelFromFile(Matrix4x3 m, Vector3 vRot, Vector3 vScale, char* pathName)
{
	char fullName[MAX_PATH];
	

	sprintf_s(fullName,"%s\\Models\\%s",fileDir,pathName);
	std::list<const char*> frames;
	frames.push_back(fullName);
	AnimatedModel *tempMod = new AnimatedModel(1);
	tempMod->m_bModelLerp = true;
	tempMod->importS3d(frames, false);

	ModelNode *tempModelNode = new ModelNode();
	tempModelNode->fullPath = fullName;
	tempModelNode->path = pathName;
	StandardVertexBuffer *tempBuf = tempMod->getNewVertexBuffer();
	Vector3 tempRot;
	tempRot = vRot;
	tempModelNode->model = tempMod;
	tempModelNode->buffer = tempBuf;
	tempModelNode->orient = m;
	tempModelNode->rot = tempRot;
	m_vModels.push_back(tempModelNode);
}

//Finds a tag in the given file.
//tag: tag to be found.
//file: the file to be searched.
//start: whether to search from the current iterator position or the start.
bool StatePlaying::findTag(char* tag, char* store, std::ifstream *file, bool start)
{
	if(start)
		file->seekg(0);
	char copyLine[500];
	bool found = false;
	bool exit = false;
	if(file->is_open())
	{
		while(!found && !exit)
		{
			file->getline(copyLine,500);
			if(strcmp(copyLine,tag) == 0)
			{
				found = true;
				strcpy(store,copyLine);
			}
			if(file->eof())
				exit = true;
		}
	}
	return found;
}

bool StatePlaying::copyRow(Vector3 *v, char* c)
{
	bool validString = true;
	for(unsigned int a = 0; a < strlen(c); a++)
	{
		if(c[a] == 0)
			a = 100;
		else if(!(validString && ((c[a] >= '0' && c[a] <= '9') || c[a] == '.' || c[a] == '-' || c[a] == ' ')))
		{
			validString = false;
			a = 100;
		}
	}
	if(validString)
	{
		char ch = c[0];
		int j = 0;
		float num = 0.0f;
		int num2 = 0;
		int counter = 0;
		float deciNum = -1;
		int multiplier = 1;
		for(int b = 0; b < 3; b++)
		{
			num = 0.0f;
			num2 = 0;
			counter = 0;
			deciNum = -1;
			multiplier = 1;
			while((ch != ' ' && ch >= '0' && ch <= '9') || ch == '.' || ch == '-')
			{
				if(ch == '.')
				{
					deciNum = 0;
					ch = c[++j];
				}
				else if(ch == '-')
				{
					multiplier *= -1;
					ch = c[++j];
				}
				else if(deciNum < 0)
				{
					num2 = num2*10 + ch - '0';
					ch = c[++j];
				}
				else
				{
					counter++;
					deciNum = deciNum + ((ch - '0')/(pow(10.0f,counter))); //handle decimals...
					ch = c[++j];
				}
			}
			if(deciNum == -1)
				num = (float)(num2)*multiplier;
			else
				num = (float)(num2+deciNum)*multiplier;
			switch(b)
			{
				case 0:
					v->x = num;
					break;
				case 1:
					v->y = num;
					break;
				case 2:
					v->z = num;
					break;
				dFefault:
					break;
			}
			ch = c[++j];
		}
		
	}
	return validString;		
		
}

bool StatePlaying::copyRow(Matrix4x3 *m,char* c,int row)
{
	bool validString = true;
	for(unsigned int a = 0; a < strlen(c); a++)
	{
		if(c[a] == 0)
			a = 100;
		else if(!(validString && ((c[a] >= '0' && c[a] <= '9') || c[a] == '.' || c[a] == '-' || c[a] == ' ')))
		{
			validString = false;
			a = 100;
		}
	}
	if(validString)
	{
		char ch = c[0];
		int j = 0;
		float num = 0.0f;
		int num2 = 0;
		int counter = 0;
		float deciNum = -1;
		int multiplier = 1;
		for(int b = 0; b < 3; b++)
		{
			num = 0.0f;
			num2 = 0;
			counter = 0;
			deciNum = -1;
			multiplier = 1;
			while((ch != ' ' && ch >= '0' && ch <= '9') || ch == '.' || ch == '-')
			{
				if(ch == '.')
				{
					deciNum = 0;
					ch = c[++j];
				}
				else if(ch == '-')
				{
					multiplier *= -1;
					ch = c[++j];
				}
				else if(deciNum < 0)
				{
					num2 = num2*10 + ch - '0';
					ch = c[++j];
				}
				else
				{
					counter++;
					deciNum = deciNum + ((ch - '0')/(pow(10.0f,counter))); //handle decimals...
					ch = c[++j];
				}
			}
			if(deciNum == -1)
				num = (float)(num2)*multiplier;
			else
				num = (float)(num2+deciNum)*multiplier;
			if(row == 0)
			{
				switch(b)
				{
					case 0:
						m->m11 = num;
						break;
					case 1:
						m->m12 = num;
						break;
					case 2:
						m->m13 = num;
						break;
					default:
						break;
				}
			}
			else if(row == 1)
			{
				switch(b)
				{
					case 0:
						m->m21 = num;
						break;
					case 1:
						m->m22 = num;
						break;
					case 2:
						m->m23 = num;
						break;
					default:
						break;
				}
			}
			else if(row == 2)
			{
				switch(b)
				{
					case 0:
						m->m31 = num;
						break;
					case 1:
						m->m32 = num;
						break;
					case 2:
						m->m33 = num;
						break;
					default:
						break;
				}
			}
			else if(row == 3)
			{
				switch(b)
				{
					case 0:
						m->tx = num;
						break;
					case 1:
						m->ty = num;
						break;
					case 2:
						m->tz = num;
						break;
					default:
						break;
				}
			}
			ch = c[++j];
		}
					
		
	}
	return validString;
}

void StatePlaying::resetGame()
{
  m_objects->clear();
  m_objects->spawnTerrain(terrain);
  m_objects->spawnWater(water);
  PlaneObject *planeObject = m_objects->getPlaneObject();
  // Create plane  
	if(fromVictory)
	{		
		m_objects->setSavePoint(0.0f,90.0f,0.0f);
	}


	  unsigned int planeID = m_objects->spawnPlane(m_objects->getSavePoint());
      m_tetherCamera->setTargetObject(planeID);
			currentTarget = planeID;

  //m_objects->spawnShadow(LocationOnterrain(10.0f,10.0f,0.0f));

 //if(savePoint == false){
 // m_v3Position[0].zero(); // collision handling will fix height
 // }
 // else if(savePoint == true){
	//m_v3Position[0].x = savex;
	//m_v3Position[0].y = savey;
	//m_v3Position[0].z = savez;
 // }


     /*int randcolor = Random.getInt(1,4);
     switch(randcolor){
     case 1:
         red = 60;
         green = 180;
         blue = 254;
         break;
     case 2:
         red = 180;
         green = 60;
         blue = 254;
         break;
     case 3:
         red = 254;
         green = 180;
         blue = 60;
         break;
     case 4:
         red = 254;
         green = 60;
         blue = 180;
         break;
     default:
         red = 60;
         green = 180;
         blue = 254;
         break;
     }*/

//   m_objects->spawnSave(LocationOnterrain(-10.0f,5.0f,0.0f));
 	if((fromVictory) || (fromMenu && !gameStart && !fromDeath))
	{
		if(fromVictory)
			fromVictory = false;
		while(!m_vModels.empty())
		{
			delete m_vModels.back()->buffer;
			delete m_vModels.back()->model;
			delete m_vModels.back();
			m_vModels.back() = NULL;
			m_vModels.pop_back();
		}
		loadLevel();
		int randcolor = Random.getInt(1,4);
		 switch(randcolor){
		 case 1:
			 red = 60;
			 green = 180;
			 blue = 254;
			 break;
		 case 2:
			 red = 180;
			 green = 60;
			 blue = 254;
			 break;
		 case 3:
			 red = 254;
			 green = 180;
			 blue = 60;
			 break;
		 case 4:
			 red = 254;
			 green = 60;
			 blue = 180;
			 break;
		 default:
			 red = 60;
			 green = 180;
			 blue = 254;
			 break;
		 }
		fromMenu = false;
		if(m_vModels.size() == 0){
			fromMenu = true;
			return;
		}
		for(int a = 0; a < m_vModels.size(); a++)
		{
			m_objects->spawnObject(m_vModels[a]->orient,a,m_vModels[a]->rot,m_vModels[a]->path);
		}
	}
	else if(fromDeath)
	{
		for(int a = 0; a < m_vModels.size(); a++)
		{
			m_objects->spawnObject(m_vModels[a]->orient,a,m_vModels[a]->rot,m_vModels[a]->path);
		}
	}

  // Load silos
  //m_objects->spawnSilo(LocationOnterrain(-35.0f, 10.0f, -140.0f));
  //m_objects->spawnSilo(LocationOnterrain(35.0f, 10.0f, -140.0f));
  ////m_objects->spawnSilo(LocationOnterrain(80.0f, 10.0f, -140.0f));
  //m_objects->spawnSilo(LocationOnterrain(125.0f, 20.0f, -140.0f));
  ////m_objects->spawnSilo(LocationOnterrain(170.0f, 10.0f, -140.0f));
  //m_objects->spawnSilo(LocationOnterrain(215.0f, 30.0f, -140.0f));
  ////m_objects->spawnSilo(LocationOnterrain(250.0f, 10.0f, -140.0f));

  //m_objects->spawnSilo(LocationOnterrain(80.0f, 15.0f, -60.0f));
  ////m_objects->spawnSilo(LocationOnterrain(125.0f, 10.0f, -60.0f));
  //m_objects->spawnSilo(LocationOnterrain(170.0f, 25.0f, -60.0f));
  ////m_objects->spawnSilo(LocationOnterrain(215.0f, 10.0f, -60.0f));
  //m_objects->spawnSilo(LocationOnterrain(250.0f, 35.0f, -60.0f));
  ////m_objects->spawnSilo(LocationOnterrain(295.0f, 10.0f, -60.0f));

  //
  //m_objects->spawnSwing(LocationOnterrain(0.0f, 60.0f, 275.0f), true);
  //m_objects->spawnSwing(LocationOnterrain(0.0f, 65.0f, 300.0f), true);
  //m_objects->spawnSwing(LocationOnterrain(0.0f, 70.0f, 325.0f), true);
  
    m_objects->spawnSave(LocationOnterrain(-10000138.530731, -1000086.736305, -100062.000000));

  // Load windmill
  //unsigned int windmillID = m_objects->spawnWindmill(LocationOnterrain(60.0f, 0.0f, 100.0f));
  
  // Load crows
  /*
  static const int numCrows = 20;
  for(int i = 0; i < numCrows; ++i)
  {
    // Distibute evenly over a hollow cylinder around the windmill
    
    GameObject *windmill = m_objects->getObjectPointer(windmillID);
    const Vector3 &windmillPos = windmill->getPosition();
    Vector3 boxSize = windmill->getBoundingBox().size();
    float minDist = sqrt(boxSize.x * boxSize.x + boxSize.z - boxSize.z); // At least the windmill's maximum diameter away from the windmill
    float maxDist = 175.0f; // Maximum distance; season to taste
    float radius = Random.getFloat(minDist, maxDist);
    float angle = Random.getFloat(0,k2Pi);
    float height = windmillPos.y + boxSize.y * Random.getFloat(1.5, 2.0f) + 10.0f; // Randomize crow height to somewhat above the windmill
    Vector3 crowPos(cos(angle) * radius + windmillPos.x, height, sin(angle) * radius + windmillPos.z);
    float crowSpeed = Random.getFloat(0.8f,2.5f);
    // One speedy crow, one slow crow
    if(i == numCrows - 2)
      crowSpeed = 6.0f;
    else if(i == numCrows - 1)
      crowSpeed = 0.5;
    
    //crowSpeed = 0.0;
    m_objects->spawnCrow(crowPos, windmillPos, crowSpeed, Random.getBool());
  }
  */

  /*
  // reset camera following crow variables
  m_planeCrashed = false;
  m_timeSinceCrashed = 0.0f;
  m_crowID = -1;
  */

  // Set the tether camera
  m_tetherCamera->minDist = 30.0f;
  m_tetherCamera->maxDist = 38.0f;
  m_tetherCamera->fov;
  m_tetherCamera->reset();
  m_tetherCamera->process(0.0f);
  gGame.m_currentCam = m_tetherCamera;   // select tether camera as the current camera

  //gGame.setFreeCamera();
  
  // kill all active particle effects
  gParticle.killAll();

  // release our endgame sound, and stop all currently playing sounds
  gSoundManager.releaseInstance(m_failedSound,m_failedInstance);
  m_failedInstance = SoundManager::NOINSTANCE;
  gSoundManager.stop();
  gameStart = false;
}

void StatePlaying::processInput()
{
	 XInputGetState(0, &state);
  PlaneObject *planeObject = m_objects->getPlaneObject();
  float timeDiff = 0;
  if(fromVictory)
  {
	  m_objects->lockMovement();
	  timeDiff = gRenderer.getTime()-m_objects->winTime;
  }

  // Exit to menu if requested
  if (gInput.keyJustUp(DIK_ESCAPE, true) || state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) 
  {        
    gGame.changeState(eGameStateMenu);
	hitPause = true;
	fromDeath = false;
    return;
  }

  if (fromVictory && timeDiff > 1000){
	  //fromVictory = false;
	  m_objects->lockMovement();
	  gGame.changeState(eGameNextLevel);
	  return;
  }
 
  // If you press space bar after you die reset game
  if (gInput.keyJustUp(DIK_SPACE) || state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
   if (planeObject->isPlaneAlive() == false)
    {
			fromVictory = false;
			fromDeath = true;
      resetGame();
      return;
    }    
}

void StatePlaying::setCam(int n)
{
	gGame.m_currentCam->m_nCamMode = n;
}

int StatePlaying::getCamMode()
{
	return gGame.m_currentCam->m_nCamMode;
}

void StatePlaying::incrementCamHeading(float f)
{
	gGame.m_currentCam->m_fMode1Heading += f;
}

void StatePlaying::incrementCamPitch(float f)
{
	gGame.m_currentCam->m_fMode1Pitch += f;
}

/// \param dt Change in time
void StatePlaying::processCamera(float dt)
{
  PlaneObject *planeObject = m_objects->getPlaneObject();
  Camera * cam = gGame.m_currentCam;

  // this should never happen but if it does leave
  if (planeObject == NULL || cam == NULL) 
    return; 

  // Grab current states
  Vector3 cameraPosOld = cam->cameraPos;
  Vector3 planePosOld = planeObject->getPosition();
  
  cam->process(dt); // move camera


  //if(planeObject->inAir==TRUE)
  //{
      cam->cameraPos.y = planePosOld.y + 18.0f;
  //}


  // Test for camera collision with terrain
  const float CameraHeight = 18.0f;
  float fTerrainHt = terrain->getHeight(cam->cameraPos.x,cam->cameraPos.z);
  if(cam->cameraPos.y - fTerrainHt < CameraHeight) 
    cam->cameraPos.y = fTerrainHt + CameraHeight;



  // location is now above terrain, set it
  cam->setAsCamera();

  // Tell terrain where the camera is so that it can adjust for LOD
  terrain->setCameraPos(cam->cameraPos);
 
  // Set 3D sound parameters based on new camera position and velocity
  Vector3 cameraVel = (cam->cameraPos - cameraPosOld) / dt;  
  gSoundManager.setListenerPosition(cam->cameraPos);
  gSoundManager.setListenerVelocity(cameraVel);
  gSoundManager.setListenerOrientation(cam->cameraOrient);
}

/// This is used to help spawn objects.
/// \param x X Coordinate on the terrain.
/// \param y Value that is added to the Y component after it is set to the height
/// of the terrain under point (X, Z)
/// \param z Z Coordinate on the terrain.
Vector3 StatePlaying::LocationOnterrain(float x, float y, float z)
{
  return Vector3(x,terrain->getHeight(x,z)+y, z);
}

//returns orientation of camera
EulerAngles StatePlaying::cameraOrient()
{
    return gGame.m_currentCam->cameraOrient;
}

void StatePlaying::moveCamera(bool right, float speed)
{
  Matrix4x3 view;

  Vector3 movement = Vector3(0.0f,0.0f,0.0f);

  if (right==false)  
    movement.x = -1.0f;
  else  
    movement.x = 1.0f;


    
  // create a matrix to transform the movement to world space
  view.setupParentToLocal(Vector3::kZeroVector, gGame.m_currentCam->cameraOrient);
  view = view.inverse();
  movement = movement*view;

  gGame.m_currentCam->cameraPos += movement * speed;
}

void StatePlaying::moveCamera2(bool up, float speed)
{
	Matrix4x3 view;
	Vector3 movement = Vector3(0.0f, 0.0f, 0.0f);

	if (up == false)
		movement.y += -10.0f;
	else
		movement.y += 5.0f;

	view.setupParentToLocal(Vector3::kZeroVector, gGame.m_currentCam->cameraOrient);
	view = view.inverse();
	movement = movement*view;

	gGame.m_currentCam->cameraPos += movement * speed;
}