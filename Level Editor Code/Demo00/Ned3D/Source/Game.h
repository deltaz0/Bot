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

/// \file Game.h
/// Code to run game logic by deriving an object from class GameBase.
/// Last updated June 26th, 2005

#ifndef __GAME_H_INCLUDED__
#define __GAME_H_INCLUDED__
#define DEGTORAD(degree) ((D3DX_PI / 180.0f) * (degree)) // converts from degrees to radians

#include "HelpText.h"
#include "Game/GameBase.h"
#include "Graphics/VertexTypes.h"
#include "DerivedModels/AnimatedModel.h"
#include "WindowsWrapper/WindowsWrapper.h"
#include "IDManager/IDManager.h"

#include <Windows.h>
#include <vector>

/// \brief The Model Viewer class.  Renders an animated model.
/// Game is derived from base class GameBase.
/// 3 Member functions are then overridden(Initiate(), Main(), and Shutdown()).
/// The WindowsWrapper class will Initiate the game engine and then call the
/// initiate function of the Game object.  It will then repetitively call the
/// Main method until the program terminates.  Upon termination, the Shutdown
/// method will be called.
struct ModNode
{
	AnimatedModel *model;
	StandardVertexBuffer *buffer;
	ModNode *next;
	ModNode *previous;
	int index;
};

struct ModelNode
{
	AnimatedModel *model;
	StandardVertexBuffer *buffer;
	Matrix4x3 orient;
	Vector3 rot;
	Vector3 scale;
	Matrix4x3 location;
	std::string path;
	std::string fullPath;
};
class Game: GameBase
{
public:
  /// Game Constructor.
  /// Clears member variables to default values
  Game();
	
  /// \brief Called by the windows wrapper after the game engine is initiated
  /// \return True on success.  False on failure (application will terminate).
  bool initiate();

  /// \brief Called when the program is exiting
  /// Game logic memory is deallocated here.
  void shutdown();

  void renderScene(); ///< Renders our model

  void process(); ///< Process input

	void rotateModel(std::vector<bool> b,float x, float y, float z);	//Rotates the current model to the given parameters
	void translateModel(std::vector<bool> b,float x, float y, float z);		//Translates the current model to the given parameters
	void scaleModel(std::vector<bool> b, float x, float y, float z);	//Scales the current model based on parameters.

	bool translateOpen;
	void toggleCamMenu(bool b) {camMenuOpen = b;}
	void updateCamMenu();
	
private:
	
	D3DXVECTOR3 diff;
	D3DXVECTOR3 intersected;
	BOOL moving;
	POINT start;
	POINT end;
	float colDist;
	int tempTrack;
	int tick;	//counts to 5 frames then moves an object that is being dragged.
	int m_nCurMode;
	int m_nCurModelIndex;
	ModNode *m_Head;
	int m_nListSize;
	int m_nCurrent;
	std::vector<ModelNode*> m_vModels;
  AnimatedModel *m_curModel; ///< Current animated model we are viewing
  AnimatedModel *m_curModel2;
  StandardVertexBuffer *m_vertexBuffer; ///< Buffer for the current model
  StandardVertexBuffer *m_vertexBuffer2;
  Matrix4x3 m_modelOrient; ///< Orientation of the world
  int m_nColorIndex; ///< Current background color

  const float m_minViewDistance; ///< Minimum distance of the camera
  const float m_maxViewDistance; ///< Maximum distance of the camera

  unsigned int m_textColor; ///< Stores the color that the text should be rendered with

  HelpText m_helpText; ///< Text that gets displayed on the screen to help the user

  /// Helper function that displays the open file dialog, and loads the models
  void loadModel();
  void loadLevel();
  void loadLevelFromModel();
	void saveLevel();
	void openCamMenu();
	bool camMenuOpen;
	bool checkExist(const char* filename); //checks if the given filename exists in the models folder	
	bool findTag(char* tag, char* store, std::ifstream *file, bool start);	//find a tag in a given file
	bool copyRow(Matrix4x3 *m,char* c,int row);	//Copies the numbers in c into the given row of m
	bool copyRow(Vector3 *v, char* c);
	void loadModelFromFile(Matrix4x3 m, Vector3 vRot, Vector3 vScale, char* pathName);	//load a given model via file name.
};

extern Game gGame;

#endif