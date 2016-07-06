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

/// \file StatePlaying.h
/// \brief Interface for the StatePlaying class. This state is the actual game
/// logic.

#ifndef __STATEPLAYING_H_INCLUDED__
#define __STATEPLAYING_H_INCLUDED__

#include "GameState.h"
#include "Game/GameBase.h"
#include "Ned3DObjectManager.h"
#include "DerivedCameras/TetherCamera.h"
#include "Terrain/Terrain.h"
#include "Objects/GameObject.h"
#include "Objects/GameObjectManager.h"
#include "Objects.h"
#include "water/water.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Console/Console.h"
#include <direct.h>

struct ModelNode
{
	AnimatedModel *model;
	StandardVertexBuffer *buffer;
	Matrix4x3 orient;
	Vector3 rot;
	Vector3 scale;
	std::string path;
	std::string fullPath;
};

/// \brief Represents the playing game state.
class StatePlaying : public GameState
{
public:
  
  StatePlaying();
  
  // Overrides
  void initiate();
  void shutdown();  
  void exitState();
  void enterState();
  void process(float dt);
  void renderScreen();
  void moveCamera(bool right, float speed);
  void moveCamera2(bool up, float speed);

  void renderScene(bool asReflection = false);

  Terrain* terrain; ///< Terrain object
  Water* water; ///< Water object
  Ned3DObjectManager* m_objects; ///< Ned Object Manager

  static bool consoleSetFollowCamera(ParameterList* params,std::string* errorMessage);
  static bool consoleChangeLOD(ParameterList* params,std::string* errorMessage);
  static bool consoleSetCameraTarget(ParameterList* params,std::string* errorMessage);
  static bool consoleGodMode(ParameterList* params,std::string* errorMessage);
  static bool consoleChangeFOV(ParameterList* params,std::string* errorMessage);

  void resetGame();

  EulerAngles cameraOrient();

  Model* m_SkyDome;
  Model* m_revSkyDome;
  Model* m_cloudScroll;
  Model* m_tcloudScroll;
  Model* m_scloudScroll;
  Model* m_stcloudScroll;
  Model* m_botDome;
  Model* m_HUD;
  //Model* m_shad;
  AnimatedModel* m_shada;
  Model* m_Notch;
  float energyScale;
  StandardVertexBuffer* m_vbuf;

  int red;
  int green;
  int blue;

  int m_texHandle;
  int m_texHandle2;
  int m_texHandle3;
  int m_texHandle4;
  int m_texTargetHandle;
  float m_texWidth;
  float m_texHeight;

	TetherCamera *m_tetherCamera;
	void loadLevel();	//load a level from a file.
	unsigned int currentTarget;
	void setCam(int n);
	int getCamMode();
	void incrementCamHeading(float f);
	void incrementCamPitch(float f);
	int getCurLvl() {return m_nCurLvl;}

private:
  /// \brief Processes escape key to exit to menu and space bar key to restart
  /// game after dying.
  void processInput();
  void processCamera(float dt);

  /// \brief Calculates a location to spawn objects at so that they are flush
  /// to the terrain.
  Vector3 LocationOnterrain(float x, float y, float z);

  
 

  // Windmill sound
  int m_windmillSound;          ///< Index for windmill sound  
  int m_windmillSoundInstance;  ///< Index for windmill sound instance

  // Failed sound
  int m_failedSound;              ///< Index for failed sound
  int m_failedInstance;           ///< Index for failed sound instance
  
  // These are used to make the camera follow a crow around when the plane 
  // crashes
  bool m_planeCrashed;      ///< True if the plane has crashed into the ground
  float m_timeSinceCrashed; ///< Time in seconds since the plane crashed
  int m_crowID;             ///< ID of the crow the camera will follow
	bool copyRow(Matrix4x3 *m,char* c,int row);	//Copies the numbers in c into the given row of m
	bool copyRow(Vector3 *v, char* c);
	bool findTag(char* tag, char* store, std::ifstream *file, bool start);	//find a tag in a given file
	void loadModelFromFile(Matrix4x3 m, Vector3 vRot, Vector3 vScale, char* pathName);	//load a given model via file name.
	int m_nCurLvl;
	int m_nMaxLvl;
};



#endif