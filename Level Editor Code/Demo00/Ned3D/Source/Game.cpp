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
//------------------------------------------------
//TODO:  line 120ish, set up camera with buttons.
//------------------------------------------------

/// \file Game.cpp
/// Code to run game logic.
/// Last updated June 26th, 2005

#include "Common/MathUtil.h"
#include "Common/Renderer.h"
#include "Common/Random.h"
#include "Common/RotationMatrix.h"
#include "DirectoryManager/DirectoryManager.h"
#include "Input/Input.h"
#include "WindowsWrapper/WindowsWrapper.h"
#include "Game.h"
#include "Common/AABB3.h"
#include <d3dx9.h>
#include "WindowsWrapper\WindowsWrapper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

extern char fileDir[MAX_PATH];
extern LPDIRECT3DDEVICE9 pD3DDevice;
Game gGame; ///< Global game object
extern WNDCLASS	wc;
extern HINSTANCE hInstApp;
extern IDMan gIDMan;

Game::Game()
: GameBase(), m_curModel(NULL), m_vertexBuffer(NULL), 
  m_minViewDistance(5.0f), m_maxViewDistance(200.0f),
  m_nColorIndex (0),
  m_helpText    (&gRenderer)
{
	m_nCurrent = -1;
  m_currentCam = new Camera();
  m_modelOrient.identity();
  tempTrack = 0;
  m_nListSize = 0;
	moving = FALSE;
	colDist = 0;
	diff.x = 0; diff.y = 0; diff.z = 0;
	translateOpen = false;
	camMenuOpen = false;
	tick = 0;
}

bool Game::initiate()
{
	
	// Call parent's init

	GameBase::initiate();

	// Set the window

	gRenderer.setFullScreenWindow();

  //Renderer settings

  gRenderer.setNearFarClippingPlanes(1.0f,2000.0f);

  //initialize the world

  float fclip = gRenderer.getFarClippingPlane();
 
  gRenderer.setFogEnable(false);
  gRenderer.setFogColor(MAKE_RGB(0, 0, 0));

  m_currentCam->cameraPos.z = -20.0f;
  
  m_textColor = MAKE_RGB(255,255,255); // set default text color to white

	m_nCurModelIndex = -1;
	openCamMenu();

	


	return true;
}

void Game::updateCamMenu()
{
	HWND vertHandle;
	HWND midHandle;
	HWND horiHandle;
	gIDMan.getID(VERT,vertHandle);
	gIDMan.getID(MID,midHandle);
	gIDMan.getID(HORI,horiHandle);
	switch(m_currentCam->dir)
	{
		case XYPLANEZPOS:
			MoveWindow(vertHandle,10,245,20,60,1);
			SendMessage(vertHandle,WM_SETTEXT,0,(LPARAM)" Y");

			MoveWindow(horiHandle,29,304,60,20,1);
			SendMessage(horiHandle,WM_SETTEXT,0,(LPARAM)" X");

			MoveWindow(midHandle,10,304,20,20,1);
			SendMessage(midHandle,WM_SETTEXT,0,(LPARAM)" +");
			break;
		case XYPLANEZNEG:
			MoveWindow(vertHandle,88,245,20,60,1);
			SendMessage(vertHandle,WM_SETTEXT,0,(LPARAM)" Y");

			MoveWindow(horiHandle,29,304,60,20,1);
			SendMessage(horiHandle,WM_SETTEXT,0,(LPARAM)" X");

			MoveWindow(midHandle,88,304,20,20,1);
			SendMessage(midHandle,WM_SETTEXT,0,(LPARAM)" -");
			break;
		case XZPLANEYPOS:
			MoveWindow(vertHandle,10,323,20,60,1);
			SendMessage(vertHandle,WM_SETTEXT,0,(LPARAM)" Z");

			MoveWindow(horiHandle,29,304,60,20,1);
			SendMessage(horiHandle,WM_SETTEXT,0,(LPARAM)" X");

			MoveWindow(midHandle,10,304,20,20,1);
			SendMessage(midHandle,WM_SETTEXT,0,(LPARAM)" +");
			break;
		case XZPLANEYNEG:
			MoveWindow(vertHandle,10,245,20,60,1);
			SendMessage(vertHandle,WM_SETTEXT,0,(LPARAM)" Z");

			MoveWindow(horiHandle,29,304,60,20,1);
			SendMessage(horiHandle,WM_SETTEXT,0,(LPARAM)" X");

			MoveWindow(midHandle,10,304,20,20,1);
			SendMessage(midHandle,WM_SETTEXT,0,(LPARAM)" -");
			break;
		case YZPLANEXPOS:
			MoveWindow(vertHandle,88,245,20,60,1);
			SendMessage(vertHandle,WM_SETTEXT,0,(LPARAM)" Y");

			MoveWindow(horiHandle,29,304,60,20,1);
			SendMessage(horiHandle,WM_SETTEXT,0,(LPARAM)" Z");

			MoveWindow(midHandle,88,304,20,20,1);
			SendMessage(midHandle,WM_SETTEXT,0,(LPARAM)" +");
			break;
		case YZPLANEXNEG:
			MoveWindow(vertHandle,10,245,20,60,1);
			SendMessage(vertHandle,WM_SETTEXT,0,(LPARAM)" Y");

			MoveWindow(horiHandle,29,304,60,20,1);
			SendMessage(horiHandle,WM_SETTEXT,0,(LPARAM)" Z");

			MoveWindow(midHandle,10,304,20,20,1);
			SendMessage(midHandle,WM_SETTEXT,0,(LPARAM)" -");
			break;
		default:
			break;
	}
	HWND *temp = gWindowsWrapper.getCamHandle();
	UpdateWindow(*temp);
}

void Game::openCamMenu()
{
	if(!camMenuOpen)
	{
		camMenuOpen = true;
		HWND *temp = gWindowsWrapper.getCamHandle();

		*temp = CreateWindowEx(
			WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME,
			"Ultimate Model Viewer",
			"Camera",
			WS_POPUP | WS_OVERLAPPEDWINDOW,
			1025,
			0,
			150,
			500,
			gWindowsWrapper.getHandle(),
			NULL,
			gWindowsWrapper.getHInstance(),
			NULL
		);
	
		gIDMan.createWin(TEXT("button"), TEXT("XY Plane, Z+"),
			WS_VISIBLE | WS_CHILD,
			10,15,120,20,
			*temp,(HMENU) BUTTON_XYPLANEZPOS, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("XY Plane, Z-"),
			WS_VISIBLE | WS_CHILD,
			10,45,120,20,
			*temp,(HMENU) BUTTON_XYPLANEZNEG, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("XZ Plane, Y+"),
			WS_VISIBLE | WS_CHILD,
			10,75,120,20,
			*temp,(HMENU) BUTTON_XZPLANEYPOS, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("XZ Plane, Y-"),
			WS_VISIBLE | WS_CHILD,
			10,105,120,20,
			*temp,(HMENU) BUTTON_XZPLANEYNEG, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("YZ Plane, X+"),
			WS_VISIBLE | WS_CHILD,
			10,135,120,20,
			*temp,(HMENU) BUTTON_YZPLANEXPOS, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("YZ Plane, X-"),
			WS_VISIBLE | WS_CHILD,
			10,165,120,20,
			*temp,(HMENU) BUTTON_YZPLANEXNEG, NULL, NULL);

		switch(m_currentCam->dir)
		{
			case XYPLANEZPOS:
				gIDMan.createWin(TEXT("STATIC"), TEXT(" Y"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,245,20,60,
					*temp,(HMENU) VERT, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" X"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					29,304,60,20,
					*temp,(HMENU) HORI, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" +"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,304,20,20,
					*temp,(HMENU) MID, NULL, NULL);
				break;
			case XYPLANEZNEG:
				gIDMan.createWin(TEXT("STATIC"), TEXT(" Y"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					88,245,20,60,
					*temp,(HMENU) VERT, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" X"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					29,304,60,20,
					*temp,(HMENU) HORI, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" -"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					88,304,20,20,
					*temp,(HMENU) MID, NULL, NULL);
				break;
			case XZPLANEYPOS:
				gIDMan.createWin(TEXT("STATIC"), TEXT(" Z"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,323,20,60,
					*temp,(HMENU) VERT, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" X"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					29,304,60,20,
					*temp,(HMENU) HORI, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" +"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,304,20,20,
					*temp,(HMENU) MID, NULL, NULL);
				break;
			case XZPLANEYNEG:
				gIDMan.createWin(TEXT("STATIC"), TEXT(" Z"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,245,20,60,
					*temp,(HMENU) VERT, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" X"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					29,304,60,20,
					*temp,(HMENU) HORI, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" -"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,304,20,20,
					*temp,(HMENU) MID, NULL, NULL);
				break;
			case YZPLANEXPOS:
				gIDMan.createWin(TEXT("STATIC"), TEXT(" Y"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					88,245,20,60,
					*temp,(HMENU) VERT, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" Z"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					29,304,60,20,
					*temp,(HMENU) HORI, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" +"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					88,304,20,20,
					*temp,(HMENU) MID, NULL, NULL);
				break;
			case YZPLANEXNEG:
				gIDMan.createWin(TEXT("STATIC"), TEXT(" Y"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,245,20,60,
					*temp,(HMENU) VERT, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" Z"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					29,304,60,20,
					*temp,(HMENU) HORI, NULL, NULL);

				gIDMan.createWin(TEXT("STATIC"), TEXT(" -"),
					WS_VISIBLE | WS_CHILD | WS_BORDER,
					10,304,20,20,
					*temp,(HMENU) MID, NULL, NULL);
				break;
			default:
				break;
		}
		ShowWindow(*temp, 8);
		UpdateWindow(*temp);
	}
}

/// Game Shutdown.
///  Game logic memory is deallocated here.

void Game::shutdown()
{
  // Call parent's shutdown
  GameBase::shutdown();

  delete m_curModel;
  m_curModel = NULL;

  delete m_vertexBuffer;
  m_vertexBuffer = NULL;
 
	while(!m_vModels.empty())
	{
		delete m_vModels.back()->buffer;
		delete m_vModels.back()->model;
		delete m_vModels.back();
		m_vModels.back() = NULL;
		m_vModels.pop_back();
	}
	/*while(m_vCurBuffer.size() != 0)
	{
		delete &m_vCurBuffer[0];
	}*/
	return;
}

void Game::renderScene()
{
  // our scene is pretty simple with a single model. Just set the orientation
  // and render the model
	/*ModNode *tempTrackNode = m_Head;
  for(int a = 0; a < m_nListSize; a++)
  {
	  gRenderer.instance(m_modelOrient);
	  tempTrackNode->model->render(tempTrackNode->buffer);
	  tempTrackNode = tempTrackNode->next;
	  gRenderer.instancePop(); 
  }*/
	if(tick < 5)
		tick+=1;
	for(unsigned int b = 0; b < m_vModels.size(); b++)
	{
		Matrix4x3 temp;
		temp.identity();
		gRenderer.instance(m_vModels[b]->orient);
		m_vModels[b]->model->render(m_vModels[b]->buffer);
		gRenderer.instancePop();
	}
	//if(m_nCurModelIndex >= 0)
	//{
	//	std::list<AnimatedModel>::iterator modIter = m_vCurModel.begin();
	//	std::list<StandardVertexBuffer>::iterator bufIter = m_vCurBuffer.begin();
	//	for(int a = 0; a <= m_nCurModelIndex; a++)
	//	{
	//		gRenderer.instance(m_modelOrient);
	//		//m_vCurModel[a].selectAnimationFrame(totalTime, 0, m_vCurBuffer[a]);
	//		StandardVertexBuffer *tempBuffer = modIter->getNewVertexBuffer();
	//		modIter->render(tempBuffer);
	//		modIter++;
	//		bufIter++;
	//		gRenderer.instancePop();
	//	}
	//	
	//}
  gRenderer.setARGB(m_textColor);
  m_helpText.Draw();
}

void Game::process()
{
  gInput.updateInput();

  float dt = gRenderer.getTimeStep();

  // process camera 
  m_currentCam->process(dt);
  
  // set camera
  m_currentCam->setAsCamera();

  static float totalTime = 0; // keep track of the time for animation
  totalTime += dt;

  // load our vertex buffer with the interpolated model
	for(unsigned int b = 0; b < m_vModels.size(); b++)
	{
		m_vModels[b]->model->selectAnimationFrame(totalTime, 0, *m_vModels[b]->buffer);
	}
	int tempInt = 0;


  /////////////////////////////////////////////////////////////
  // process input
  /////////////////////////////////////////////////////////////

  // escape is our exit key
  if(gInput.keyDown(DIK_ESCAPE))
    gWindowsWrapper.quit();

  // load model
  if(gInput.keyJustDown(DIK_F2))
    loadModel();
	if(gInput.keyJustDown(DIK_F5))
	{
		//Save Data.
		saveLevel();
	}
	if(gInput.keyJustDown(DIK_F7))
	{
		loadLevel();
	}
  // toggle background color
  if(gInput.keyJustDown(DIK_F3))
  {
    m_nColorIndex++;
    if(m_nColorIndex > 4)
      m_nColorIndex = 0;
    
    unsigned int bkColor;
    switch(m_nColorIndex)
    {
    case 0:
      bkColor = MAKE_RGB(0, 0, 0);      
      break;
    case 1:
      bkColor = MAKE_RGB(255, 0, 0);
      break;
    case 2:
      bkColor = MAKE_RGB(0, 255, 0);
      break;
    case 3:
      bkColor = MAKE_RGB(0, 0, 255);
      break;
    case 4:
      bkColor = MAKE_RGB(255, 255, 255);
      break;
    }

	  gRenderer.setFogColor(bkColor);
    
	  if (bkColor == MAKE_RGB(255,255,255))
      m_renderInfoColor = m_textColor = MAKE_RGB(0,0,0);
	  else
	    m_renderInfoColor = m_textColor = MAKE_RGB(255,255,255);
	
  }

  if(gInput.keyJustDown(DIK_TAB))
    m_helpText.ToggleVisibility();
	if(gInput.keyJustDown(DIK_DELETE))
	{
		if(m_nCurrent >= 0)
		{
			//m_vModels[m_nCurrent
			delete m_vModels[m_nCurrent]->buffer;
			delete m_vModels[m_nCurrent]->model;
			delete m_vModels[m_nCurrent];
			m_vModels[m_nCurrent] = NULL;
			m_vModels.erase(m_vModels.begin()+m_nCurrent);
			m_nCurrent = -1;
		}
	}
	if(gInput.keyJustDown(DIK_LEFT) && m_nCurrent >= 0)
	{
		m_vModels[m_nCurrent]->orient.tx -= 1;
	}

	if(gInput.keyJustDown(DIK_RIGHT) && m_nCurrent >= 0)
	{
		m_vModels[m_nCurrent]->orient.tx += 1;
	}

	if(gInput.keyJustDown(DIK_UP) && m_nCurrent >= 0)
	{
		m_vModels[m_nCurrent]->orient.ty += 1;
	}

	if(gInput.keyJustDown(DIK_DOWN) && m_nCurrent >= 0)
	{
		m_vModels[m_nCurrent]->orient.ty -= 1;
	}

	if(gInput.keyJustDown(DIK_LBRACKET) && m_nCurrent >= 0)
	{
		m_vModels[m_nCurrent]->orient.tz += 1;
	}

	if(gInput.keyJustDown(DIK_RBRACKET) && m_nCurrent >= 0)
	{
		m_vModels[m_nCurrent]->orient.tz -= 1;
	}

	if(gInput.keyJustDown(DIK_R) && m_nCurrent >= 0)
	{
			HMENU hPopMenu = CreatePopupMenu();
		/*InsertMenuItem(hPopMenu,0,TRUE,*/
		AppendMenu(hPopMenu, MF_STRING, 0, "&Save");
		HWND *temp = gWindowsWrapper.getPopHandle();
		*temp = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME,
		"Ultimate Model Viewer",
		"Rotation",
		WS_POPUP | WS_OVERLAPPEDWINDOW,
		0,
		0,
		150,
		150,
		gWindowsWrapper.getHandle(),
		NULL,
		gWindowsWrapper.getHInstance(),
		NULL
	);

		/*HWND hWndTxt = CreateWindow(TEXT("edit"), TEXT("text goes here"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,10,300,20,
			*temp,(HMENU) EDITX, NULL, NULL);*/
		/*std::string testString = "wakka wakka";
		std::wstring test2 = (testString);*/
		char buffx[100];
		char buffy[100];
		char buffz[100];

    sprintf_s(buffx, "%f", m_vModels[m_nCurrent]->rot.x);
		sprintf_s(buffy, "%f", m_vModels[m_nCurrent]->rot.y);
		sprintf_s(buffz, "%f", m_vModels[m_nCurrent]->rot.z);
    LPCSTR px = buffx;
		LPCSTR py = buffy;
		LPCSTR pz = buffz;

		gIDMan.createWin(TEXT("edit"), TEXT(px),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,10,100,20,
			*temp,(HMENU) EDITX, NULL, NULL);
		
		HWND tempXLabel = CreateWindow(TEXT("STATIC"), TEXT(" X"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,10,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("edit"), TEXT(py),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,35,100,20,
			*temp,(HMENU) EDITY, NULL, NULL);

		HWND tempYLabel = CreateWindow(TEXT("STATIC"), TEXT(" Y"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,35,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("edit"), TEXT(pz),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,60,100,20,
			*temp,(HMENU) EDITZ, NULL, NULL);

		HWND tempZLabel = CreateWindow(TEXT("STATIC"), TEXT(" Z"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,60,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("BACK"),
			WS_VISIBLE | WS_CHILD,
			85,90,50,20,
			*temp,(HMENU) BUTTON_CANCEL, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("APPLY"),
			WS_VISIBLE | WS_CHILD,
			10,90,50,20,
			*temp, (HMENU) BUTTON_OK, NULL, NULL);

		ShowWindow(*temp, 5);
		UpdateWindow(*temp);
	}

	if(gInput.keyJustDown(DIK_T) && m_nCurrent >= 0)
	{
		translateOpen = true;
		HWND *temp = gWindowsWrapper.getTrnHandle();
		*temp = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME,
		"Ultimate Model Viewer",
		"Translation",
		WS_POPUP | WS_OVERLAPPEDWINDOW,
		0,
		0,
		150,
		150,
		gWindowsWrapper.getHandle(),
		NULL,
		gWindowsWrapper.getHInstance(),
		NULL
	);

		/*HWND hWndTxt = CreateWindow(TEXT("edit"), TEXT("text goes here"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,10,300,20,
			*temp,(HMENU) EDITX, NULL, NULL);*/
		/*std::string testString = "wakka wakka";
		std::wstring test2 = (testString);*/
		char buffx[100];
		char buffy[100];
		char buffz[100];

    sprintf_s(buffx, "%f", m_vModels[m_nCurrent]->orient.tx);
		sprintf_s(buffy, "%f", m_vModels[m_nCurrent]->orient.ty);
		sprintf_s(buffz, "%f", m_vModels[m_nCurrent]->orient.tz);
    LPCSTR px = buffx;
		LPCSTR py = buffy;
		LPCSTR pz = buffz;

		gIDMan.createWin(TEXT("edit"), TEXT(px),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,10,100,20,
			*temp,(HMENU) TRANSX, NULL, NULL);
		
		HWND tempXLabel = CreateWindow(TEXT("STATIC"), TEXT(" X"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,10,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("edit"), TEXT(py),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,35,100,20,
			*temp,(HMENU) TRANSY, NULL, NULL);

		HWND tempYLabel = CreateWindow(TEXT("STATIC"), TEXT(" Y"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,35,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("edit"), TEXT(pz),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,60,100,20,
			*temp,(HMENU) TRANSZ, NULL, NULL);

		HWND tempZLabel = CreateWindow(TEXT("STATIC"), TEXT(" Z"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,60,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("BACK"),
			WS_VISIBLE | WS_CHILD,
			85,90,50,20,
			*temp,(HMENU) BUTTON_CANCEL, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("APPLY"),
			WS_VISIBLE | WS_CHILD,
			10,90,50,20,
			*temp, (HMENU) BUTTON_OK, NULL, NULL);

		ShowWindow(*temp, 5);
		UpdateWindow(*temp);
	}

	if(gInput.keyJustDown(DIK_S) && m_nCurrent >= 0)
	{
		HWND *temp = gWindowsWrapper.getSclHandle();
		*temp = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME,
		"Ultimate Model Viewer",
		"Scale",
		WS_POPUP | WS_OVERLAPPEDWINDOW,
		0,
		0,
		150,
		150,
		gWindowsWrapper.getHandle(),
		NULL,
		gWindowsWrapper.getHInstance(),
		NULL
	);

		/*HWND hWndTxt = CreateWindow(TEXT("edit"), TEXT("text goes here"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,10,300,20,
			*temp,(HMENU) EDITX, NULL, NULL);*/
		/*std::string testString = "wakka wakka";
		std::wstring test2 = (testString);*/
		char buffx[100];
		char buffy[100];
		char buffz[100];

    sprintf_s(buffx, "%f", m_vModels[m_nCurrent]->scale.x);
		sprintf_s(buffy, "%f", m_vModels[m_nCurrent]->scale.y);
		sprintf_s(buffz, "%f", m_vModels[m_nCurrent]->scale.z);
    LPCSTR px = buffx;
		LPCSTR py = buffy;
		LPCSTR pz = buffz;

		gIDMan.createWin(TEXT("edit"), TEXT(px),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,10,100,20,
			*temp,(HMENU) SCALEX, NULL, NULL);
		
		HWND tempXLabel = CreateWindow(TEXT("STATIC"), TEXT(" X"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,10,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("edit"), TEXT(py),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,35,100,20,
			*temp,(HMENU) SCALEY, NULL, NULL);

		HWND tempYLabel = CreateWindow(TEXT("STATIC"), TEXT(" Y"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,35,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("edit"), TEXT(pz),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			35,60,100,20,
			*temp,(HMENU) SCALEZ, NULL, NULL);

		HWND tempZLabel = CreateWindow(TEXT("STATIC"), TEXT(" Z"),
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			10,60,20,20,
			*temp, NULL, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("BACK"),
			WS_VISIBLE | WS_CHILD,
			85,90,50,20,
			*temp,(HMENU) BUTTON_CANCEL, NULL, NULL);

		gIDMan.createWin(TEXT("button"), TEXT("APPLY"),
			WS_VISIBLE | WS_CHILD,
			10,90,50,20,
			*temp, (HMENU) BUTTON_OK, NULL, NULL);

		ShowWindow(*temp, 5);
		UpdateWindow(*temp);
	}

	if(gInput.keyJustDown(DIK_O))
	{
		m_currentCam->cameraPos.x = 0;
		m_currentCam->cameraPos.y = 0;
		m_currentCam->cameraPos.z = 0;
		switch(m_currentCam->dir)
		{
			case XYPLANEZPOS:
				m_currentCam->cameraPos.z = -20;
				break;
			case XYPLANEZNEG:
				m_currentCam->cameraPos.z = 20;
				break;
			case XZPLANEYPOS:
				m_currentCam->cameraPos.y = -20;
				break;
			case XZPLANEYNEG:
				m_currentCam->cameraPos.y = 20;
				break;
			case YZPLANEXPOS:
				m_currentCam->cameraPos.x = -20;
				break;
			case YZPLANEXNEG:
				m_currentCam->cameraPos.x = 20;
				break;
			default:
				break;
		}
		
	}

	if(gInput.keyJustDown(DIK_C))
	{
		openCamMenu();
	}

  if(gInput.getLeftMouseDown()) // rotate the model
  {
    EulerAngles orientDelta(
      wrapPi(gInput.getMouseLX() / 100.0f),
      -wrapPi(gInput.getMouseLY() / 100.0f),
      0);

    Matrix4x3 mOrientDelta;
    mOrientDelta.setupLocalToParent(Vector3::kZeroVector, orientDelta);

		/*if(m_nCurrent >= 0)
		{
			m_vModels[m_nCurrent]->orient = m_vModels[m_nCurrent]->orient * mOrientDelta;
		}*/
    /*m_modelOrient = m_modelOrient * mOrientDelta;*/
		/*for(int a = 0; a < m_vModels.size(); a++)
		{
			m_vModels[a]->orient = m_modelOrient;
		}*/
  }
  else if(gInput.getRightMouseDown()) // move camera in/out
  {
		if(m_currentCam->dir == XYPLANEZPOS)
		{
			if(gInput.keyDown(DIK_LSHIFT))
				m_currentCam->cameraPos.z += gInput.getMouseLY();
			else
			{
				m_currentCam->cameraPos.y += gInput.getMouseLY()*.25f;
				m_currentCam->cameraPos.x -= gInput.getMouseLX()*.25f;
			}
		}
		else if(m_currentCam->dir == XYPLANEZNEG)
		{
			if(gInput.keyDown(DIK_LSHIFT))
				m_currentCam->cameraPos.z -= gInput.getMouseLY();
			else
			{
				m_currentCam->cameraPos.y += gInput.getMouseLY()*.25f;
				m_currentCam->cameraPos.x += gInput.getMouseLX()*.25f;
			}
		}
		else if(m_currentCam->dir == XZPLANEYPOS)
		{
			if(gInput.keyDown(DIK_LSHIFT))
				m_currentCam->cameraPos.y += gInput.getMouseLY();
			else
			{
				m_currentCam->cameraPos.z -= gInput.getMouseLY()*.25f;
				m_currentCam->cameraPos.x -= gInput.getMouseLX()*.25f;
			}
		}
		else if(m_currentCam->dir == XZPLANEYNEG)
		{
			if(gInput.keyDown(DIK_LSHIFT))
				m_currentCam->cameraPos.y -= gInput.getMouseLY();
			else
			{
				m_currentCam->cameraPos.z += gInput.getMouseLY()*.25f;
				m_currentCam->cameraPos.x -= gInput.getMouseLX()*.25f;
			}
		}
		else if(m_currentCam->dir == YZPLANEXPOS)
		{
			if(gInput.keyDown(DIK_LSHIFT))
				m_currentCam->cameraPos.x += gInput.getMouseLY();
			else
			{
				m_currentCam->cameraPos.y += gInput.getMouseLY()*.25f;
				m_currentCam->cameraPos.z += gInput.getMouseLX()*.25f;
			}
		}
		else if(m_currentCam->dir == YZPLANEXNEG)
		{
			if(gInput.keyDown(DIK_LSHIFT))
				m_currentCam->cameraPos.x -= gInput.getMouseLY();
			else
			{
				m_currentCam->cameraPos.y += gInput.getMouseLY()*.25f;
				m_currentCam->cameraPos.z -= gInput.getMouseLX()*.25f;
			}
		}
    // remember our camera stays in the negative z direction, looking in the
    // direction of positive z.
    /*if(m_currentCam->cameraPos.z < -m_maxViewDistance)
      m_currentCam->cameraPos.z = -m_maxViewDistance;
    else if(m_currentCam->cameraPos.z > -m_minViewDistance)
      m_currentCam->cameraPos.z = -m_minViewDistance;*/
  }
	if(gInput.getLeftMouseClicked())
	{
		gInput.setLeftMouseClicked();
		HWND tempWindow = gWindowsWrapper.getHandle();
		HWND compWindow = GetForegroundWindow();
		if(compWindow == tempWindow)
		{
			D3DXVECTOR3 finalVec;
				D3DXVECTOR3 originVec;
			Camera tempCam = gGameBase->getCam();
			Matrix4x3 viewMatrix1;
			tempCam.getViewMatrix(&viewMatrix1);

			D3DXMATRIX projMatrix;
			D3DVIEWPORT9 tempView;
			pD3DDevice->GetTransform(D3DTS_PROJECTION,&projMatrix);
			pD3DDevice->GetViewport(&tempView);

			//Change view matrix into a standard matrix.
			D3DXMATRIX viewMatrix;
			viewMatrix._11 = viewMatrix1.m11;
			viewMatrix._12 = viewMatrix1.m12;
			viewMatrix._13 = viewMatrix1.m13;
			viewMatrix._14 = 0;
			viewMatrix._21 = viewMatrix1.m21;
			viewMatrix._22 = viewMatrix1.m22;
			viewMatrix._23 = viewMatrix1.m23;
			viewMatrix._24 = 0;
			viewMatrix._31 = viewMatrix1.m31;
			viewMatrix._32 = viewMatrix1.m32;
			viewMatrix._33 = viewMatrix1.m33;
			viewMatrix._34 = 0;
			viewMatrix._41 = viewMatrix1.tx;
			viewMatrix._42 = viewMatrix1.ty;
			viewMatrix._43 = viewMatrix1.tz;
			viewMatrix._44 = 1;
			//End copy

	
			POINT tempMouse;
			GetCursorPos(&tempMouse);
			GetCursorPos(&start);
		
			ScreenToClient(tempWindow,&tempMouse);
			ScreenToClient(tempWindow,&start);

			D3DXVECTOR4 testVec;
			testVec.x = (((2.0f * tempMouse.x) / tempView.Width ) - 1.0f ) / projMatrix._11;
			testVec.y = -(((2.0f * tempMouse.y) / tempView.Height ) - 1.0f ) / projMatrix._22;
			testVec.z = 1.0f;
			std::vector<int> hits;
			std::vector<float> distance;
			for(unsigned int a = 0; a < m_vModels.size(); a++)
			{
				Matrix4x3 tempMatrix = m_vModels[a]->orient;
				//D3DXMATRIX world = tempMatrix.convertMatrix();
				Matrix4x3 modToWorld = gRenderer.getModelToWorldMatrix();
				D3DXMATRIX world = modToWorld.convertMatrix();
				//TEST
				//D3DXMatrixIdentity(&world);
				//END TEST
				D3DXMATRIX combinedMatrices = world*viewMatrix;
				D3DXMATRIX invView;
			
				D3DXMatrixInverse(&invView, NULL, &combinedMatrices);
				
				finalVec.x = testVec.x*invView._11 + testVec.y*invView._21 + testVec.z*invView._31;
				finalVec.y = testVec.x*invView._12 + testVec.y*invView._22 + testVec.z*invView._32;
				finalVec.z = testVec.x*invView._13 + testVec.y*invView._23 + testVec.z*invView._33;
				originVec.x = invView._41;
				originVec.y = invView._42;
				originVec.z = invView._43;

				Vector3 norm;
				Vector3 delta;
				Vector3 ori;
				delta.x = finalVec.x;
				delta.y = finalVec.y;
				delta.z = finalVec.z;
				ori.x = originVec.x;
				ori.y = originVec.y;
				ori.z = originVec.z;
				norm.x = 0;
				norm.y = 0;
				norm.z = 0;
				AABB3 testBox = m_vModels[a]->model->getBoundingBox(m_vModels[a]->orient);
				colDist = testBox.rayIntersect(ori,delta*90000,&norm);
				if(colDist < 1)	//found collision
				{
					//COLLISION GET
					hits.push_back(a);
					distance.push_back(colDist);
					//m_nCurrent = a;
					//a = m_vModels.size();
					
					//finished
					//moving = FALSE; //remove later
				}
				else
				{
					//YOU.  GET.  NOTHING.
					m_nCurrent = -1;
				}
			}
			int currentSmallest = 0;
			if(hits.size() > 0)
				m_nCurrent = hits[0];
			for(int a = 1; a < hits.size(); a++)
			{
				if(distance[a] < distance[currentSmallest])
				m_nCurrent = hits[a];
			}
			//collision found code here
			if(m_nCurrent >= 0)
			{
				moving = TRUE;
					//Find difference between where we clicked and where the origin is.
					//First, find where our ray intersects the plane the object is on.
					D3DXVECTOR3 p1;
					D3DXVECTOR3 p2;
					D3DXVECTOR3 p3;
					D3DXVECTOR3 intersect;
					D3DXVECTOR3 modOri; modOri.x = m_vModels[m_nCurrent]->orient.tx; modOri.y = m_vModels[m_nCurrent]->orient.ty;
															modOri.z = m_vModels[m_nCurrent]->orient.tz;
					D3DXPLANE plane;
					if(m_currentCam->dir == XYPLANEZPOS || m_currentCam->dir == XYPLANEZNEG)
					{
						p1.x = 0; p1.y = 1; p1.z = m_vModels[m_nCurrent]->orient.tz;//These form our plane.
						p2.x = 1; p2.y = 0; p2.z = m_vModels[m_nCurrent]->orient.tz;	
						p3.x = 0; p3.y = 0; p3.z = m_vModels[m_nCurrent]->orient.tz;
					}
					else if(m_currentCam->dir == XZPLANEYPOS || m_currentCam->dir == XZPLANEYNEG)
					{
						p1.x = 0; p1.y = m_vModels[m_nCurrent]->orient.ty; p1.z = 1;
						p2.x = 1; p2.y = m_vModels[m_nCurrent]->orient.ty; p2.z = 0;	
						p3.x = 0; p3.y = m_vModels[m_nCurrent]->orient.ty; p3.z = 0;
					}
					else if(m_currentCam->dir == YZPLANEXPOS || m_currentCam->dir == YZPLANEXNEG)
					{
						p1.x = m_vModels[m_nCurrent]->orient.tx; p1.y = 0; p1.z = 1;
						p2.x = m_vModels[m_nCurrent]->orient.tx; p2.y = 1; p2.z = 0;	
						p3.x = m_vModels[m_nCurrent]->orient.tx; p3.y = 0; p3.z = 0;
					}
					D3DXPlaneFromPoints(&plane, &p1, &p2, &p3);
					D3DXPlaneNormalize(&plane, &plane);
					D3DXVECTOR3 startPoint = originVec;
					D3DXVECTOR3 endPoint = originVec+(90000*finalVec);
					D3DXPlaneIntersectLine(&intersect, &plane, &startPoint, &endPoint);
					//finished
					diff = intersect-modOri;
			}
		}
	}
	if(!gInput.getLeftMouseDown() && moving && m_nCurrent >= 0 || moving && m_nCurrent >= 0 && tick == 5)
	{
		if(tick == 5)
			tick = 0;
		if(!gInput.getLeftMouseDown())
			moving = FALSE;
		HWND tempWindow = gWindowsWrapper.getHandle();
		GetCursorPos(&end);
		ScreenToClient(tempWindow,&end);

		D3DVIEWPORT9 tempView;
		pD3DDevice->GetViewport(&tempView);
		D3DXMATRIX projMatrix;
		pD3DDevice->GetTransform(D3DTS_PROJECTION,&projMatrix);

		Camera tempCam = gGameBase->getCam();
		Matrix4x3 viewMatrix1;
		tempCam.getViewMatrix(&viewMatrix1);
		D3DXMATRIX viewMatrix = viewMatrix1.convertMatrix();
		Matrix4x3 modToWorld = gRenderer.getModelToWorldMatrix();
		D3DXMATRIX world = modToWorld.convertMatrix();

		D3DXMATRIX combinedMatrices = world*viewMatrix;
			
			
			

		D3DXMATRIX invView;
		//D3DXMatrixInverse(&invView, NULL, &viewMatrix);
		D3DXMatrixInverse(&invView, NULL, &combinedMatrices);
		D3DXMATRIX invProj;
		D3DXMatrixInverse(&invProj, NULL, &projMatrix);

		//Convert starting point to world space.
		/*D3DXVECTOR4 testVec;
		testVec.x = (((2.0 * start.x) / tempView.Width ) - 1 ) / projMatrix._11;
		testVec.y = -(((2.0 * start.y) / tempView.Height ) - 1 ) / projMatrix._22;
		testVec.z = 1.0f;

		D3DXVECTOR3 finalVec;
		D3DXVECTOR3 originVec;
		finalVec.x = testVec.x*invView._11 + testVec.y*invView._21 + testVec.z*invView._31;
		finalVec.y = testVec.x*invView._12 + testVec.y*invView._22 + testVec.z*invView._32;
		finalVec.z = testVec.x*invView._13 + testVec.y*invView._23 + testVec.z*invView._33;
		originVec.x = invView._41;
		originVec.y = invView._42;
		originVec.z = invView._43;

		Vector3 norm;
		Vector3 delta;
		Vector3 ori;
		delta.x = finalVec.x;
		delta.y = finalVec.y;
		delta.z = finalVec.z;
		ori.x = originVec.x;
		ori.y = originVec.y;
		ori.z = originVec.z;
		norm.x = 0;
		norm.y = 0;
		norm.z = 0;*/
		//Convert end point
		D3DXVECTOR4 testVec;
		testVec.x = (((2.0f * end.x) / tempView.Width ) - 1.0f ) / projMatrix._11;
		testVec.y = -(((2.0f * end.y) / tempView.Height ) - 1.0f ) / projMatrix._22;
		testVec.z = 1.0f;

		D3DXVECTOR3 finalVec;
		D3DXVECTOR3 originVec;
		finalVec.x = testVec.x*invView._11 + testVec.y*invView._21 + testVec.z*invView._31;
		finalVec.y = testVec.x*invView._12 + testVec.y*invView._22 + testVec.z*invView._32;
		finalVec.z = testVec.x*invView._13 + testVec.y*invView._23 + testVec.z*invView._33;
		originVec.x = invView._41;
		originVec.y = invView._42;
		originVec.z = invView._43;

		Vector3 norm;
		D3DXVECTOR3 delta;
		D3DXVECTOR3 ori;
		delta.x = finalVec.x;
		delta.y = finalVec.y;
		delta.z = finalVec.z;
		delta*=90000;
		ori.x = originVec.x;
		ori.y = originVec.y;
		ori.z = originVec.z;
		norm.x = 0;
		norm.y = 0;
		norm.z = 0;
		//Find difference between where we clicked and where the origin is.
				//First, find where our ray intersects the plane the object is on.
				D3DXVECTOR3 p1;
				D3DXVECTOR3 p2;
				D3DXVECTOR3 p3;
				D3DXVECTOR3 intersect;
				D3DXVECTOR3 modOri; modOri.x = m_vModels[m_nCurrent]->orient.tx; modOri.y = m_vModels[m_nCurrent]->orient.ty;
														modOri.z = m_vModels[m_nCurrent]->orient.tz;
				D3DXPLANE plane;
				if(m_currentCam->dir == XYPLANEZPOS || m_currentCam->dir == XYPLANEZNEG)
				{
					p1.x = 0; p1.y = 1; p1.z = m_vModels[m_nCurrent]->orient.tz;//These form our plane.
					p2.x = 1; p2.y = 0; p2.z = m_vModels[m_nCurrent]->orient.tz;	
					p3.x = 0; p3.y = 0; p3.z = m_vModels[m_nCurrent]->orient.tz;
				}
				else if(m_currentCam->dir == XZPLANEYPOS || m_currentCam->dir == XZPLANEYNEG)
				{
					p1.x = 0; p1.y = m_vModels[m_nCurrent]->orient.ty; p1.z = 1;//These form our plane.
					p2.x = 1; p2.y = m_vModels[m_nCurrent]->orient.ty; p2.z = 0;	
					p3.x = 0; p3.y = m_vModels[m_nCurrent]->orient.ty; p3.z = 0;
				}
				else if(m_currentCam->dir == YZPLANEXPOS || m_currentCam->dir == YZPLANEXNEG)
				{
					p1.x = m_vModels[m_nCurrent]->orient.tx; p1.y = 0; p1.z = 1;
					p2.x = m_vModels[m_nCurrent]->orient.tx; p2.y = 1; p2.z = 0;	
					p3.x = m_vModels[m_nCurrent]->orient.tx; p3.y = 0; p3.z = 0;
				}
				D3DXPlaneFromPoints(&plane, &p1, &p2, &p3);
				D3DXPlaneNormalize(&plane, &plane);
				D3DXVECTOR3 startPoint = originVec;
				D3DXVECTOR3 endPoint = originVec+(90000*finalVec);
				D3DXPlaneIntersectLine(&intersect, &plane, &startPoint, &endPoint);
				//finished
				
				//finished
		D3DXVECTOR3 dropPoint = ori+(colDist*delta);
		/*if(m_currentCam->dir == XYPLANEZPOS || m_currentCam->dir == XYPLANEZNEG)
		{
			m_vModels[m_nCurrent]->orient.tx = intersect.x-diff.x;
			m_vModels[m_nCurrent]->orient.ty = intersect.y-diff.y;
			m_vModels[m_nCurrent]->orient.tz = intersect.z;
		}
		else if(m_currentCam->dir == XZPLANEYPOS || m_currentCam->dir == XZPLANEYNEG)
		{
			m_vModels[m_nCurrent]->orient.tx = intersect.x-diff.x;
			m_vModels[m_nCurrent]->orient.ty = intersect.y;
			m_vModels[m_nCurrent]->orient.tz = intersect.z-diff.z;
		}*/
		m_vModels[m_nCurrent]->orient.tx = intersect.x-diff.x;
		m_vModels[m_nCurrent]->orient.ty = intersect.y-diff.y;
		m_vModels[m_nCurrent]->orient.tz = intersect.z-diff.z;

		if(translateOpen)	//update the translation window if it is open
		{
			char buffx[100];
			char buffy[100];
			char buffz[100];
			sprintf_s(buffx,"%f",m_vModels[m_nCurrent]->orient.tx);
			sprintf_s(buffy,"%f",m_vModels[m_nCurrent]->orient.ty);
			sprintf_s(buffz,"%f",m_vModels[m_nCurrent]->orient.tz);
			HWND tempHandle;
			if(gIDMan.getID(TRANSX,tempHandle))
				SendMessage(tempHandle,WM_SETTEXT,0,(LPARAM)buffx);
			if(gIDMan.getID(TRANSY,tempHandle))
				SendMessage(tempHandle,WM_SETTEXT,0,(LPARAM)buffy);
			if(gIDMan.getID(TRANSZ,tempHandle))
				SendMessage(tempHandle,WM_SETTEXT,0,(LPARAM)buffz);
		}
		//done
		// done
		//Try something different, unproject then unview the end point
		//D3DXVECTOR4 endVec;
		//endVec.x = end.x;
		//endVec.y = end.y;
		//endVec.z = 1.0f;

		////testVec = testVec*invProj;
		//D3DXVec4Transform(&endVec,&endVec,&invProj);
		//D3DXVec4Transform(&endVec,&endVec,&invView);

		////done
		////same for start
		//D3DXVECTOR4 startVec;
		//startVec.x = start.x;
		//startVec.y = start.y;
		//startVec.z = 1.0f;

		////testVec = testVec*invProj;
		//D3DXVec4Transform(&startVec,&startVec,&invProj);
		//D3DXVec4Transform(&startVec,&startVec,&invView);
		////done

		//D3DXVECTOR4 transformVec;
		//transformVec.x = endVec.x-startVec.x;
		//transformVec.y = endVec.y-startVec.y;
		//transformVec.z = endVec.z-startVec.z;
		//int check = 0;
		//m_vModels[m_nCurrent]->orient.tx+=transformVec.x;
		//m_vModels[m_nCurrent]->orient.ty+=transformVec.y;
		//m_vModels[m_nCurrent]->orient.tz+=transformVec.z;
	}
}


//Rotates the currently selected model by the given amounts.
void Game::rotateModel(std::vector<bool> b,float x, float y, float z)
{
	if(b.size() == 3 && m_nCurrent >= 0)
	{
		Matrix4x3 tempRotX;
		Matrix4x3 tempRotY;
		Matrix4x3 tempRotZ;
		Matrix4x3 finalRot;
		if(b[0])	//if x is valid, setup rotation
			tempRotX.setupRotateX(DEGTORAD(x));
		else
			tempRotX.identity();
		if(b[1])
			tempRotY.setupRotateY(DEGTORAD(y));
		else
			tempRotY.identity();
		if(b[2])
			tempRotZ.setupRotateZ(DEGTORAD(z));
		else
			tempRotZ.identity();

		finalRot = tempRotY*tempRotX;
		finalRot = finalRot*tempRotZ;
		Matrix4x3 tempScale;
		Matrix4x3 finalMatrix;
		tempScale.setupScale(m_vModels[m_nCurrent]->scale);
		finalMatrix = tempScale*finalRot;
		Vector3 tempVec = m_vModels[m_nCurrent]->orient.getTranslation();
		finalMatrix.setTranslation(tempVec);


		m_vModels[m_nCurrent]->orient = finalMatrix;
		if(b[0])
			m_vModels[m_nCurrent]->rot.x = x;
		if(b[1])
			m_vModels[m_nCurrent]->rot.y = y;
		if(b[2])
			m_vModels[m_nCurrent]->rot.z = z;
	}
}

//Translates the model
//Params:
//n:	Represents which values to default and which to change
void Game::translateModel(std::vector<bool> b,float x, float y, float z)
{
	if(b.size() == 3 && m_nCurrent>= 0)
	{
		if(b[0])//if x value is valid, change it
			m_vModels[m_nCurrent]->orient.tx = x;
		if(b[1])//if y is valid....
			m_vModels[m_nCurrent]->orient.ty = y;
		if(b[2])//if z is valid...
			m_vModels[m_nCurrent]->orient.tz = z;
	}
}

void Game::scaleModel(std::vector<bool> b, float x, float y, float z)
{
	if(b.size() == 3 && m_nCurrent >= 0)
	{
		Vector3 scales;
		scales.set(1,1,1);
		if(b[0])
			scales.x = x;
		if(b[1])
			scales.y = y;
		if(b[2])
			scales.z = z;
		Matrix4x3 tempMatrix;
		//tempMatrix.setupScale(scales);
		tempMatrix.identity();
		tempMatrix.m11 = scales.x;
		tempMatrix.m22 = scales.y;
		tempMatrix.m33 = scales.z;
		m_vModels[m_nCurrent]->scale = scales;
		Matrix4x3 tempRotX;
		Matrix4x3 tempRotY;
		Matrix4x3 tempRotZ;
		Matrix4x3 tempRotFinal;
		tempRotX.setupRotateX(DEGTORAD(m_vModels[m_nCurrent]->rot.x));
		tempRotY.setupRotateY(DEGTORAD(m_vModels[m_nCurrent]->rot.y));
		tempRotZ.setupRotateZ(DEGTORAD(m_vModels[m_nCurrent]->rot.z));
		tempRotFinal = tempRotX*tempRotY;
		tempRotFinal = tempRotFinal*tempRotZ;
		Matrix4x3 finalMatrix = tempMatrix*tempRotFinal;
		Vector3 tempVec = m_vModels[m_nCurrent]->orient.getTranslation();
		finalMatrix.setTranslation(tempVec);
		m_vModels[m_nCurrent]->orient = finalMatrix;
	}
}

void Game::saveLevel()
{
	for(unsigned int b = 0; b < m_vModels.size(); b++)
	{
		char fileName[100];
		/*std::string tempString = m_vModels[b].path;
		const char * c = m_vModels[b].path.c_str();*/
		sprintf_s(fileName,"Models\\%s",m_vModels[b]->path.c_str());
		std::ofstream tempStream;
		if(!checkExist(fileName))	//if the model we're saving isn't in the models folder yet
		{
			//put it there
			char writeName[MAX_PATH];
			char copyLine[500];
			sprintf_s(writeName,"%s\\%s",fileDir,fileName);
			std::ofstream modWrite;
			std::ifstream modRead;
			modWrite.open(writeName);
			modRead.open(m_vModels[b]->fullPath);
			while(!modRead.eof())
			{
				modRead.getline(copyLine,500);
				modWrite << copyLine << std::endl;
			}
			modWrite.close();
			modRead.close();
		}
	}
	std::ofstream testfile;
	std::string filename;
	std::string directory;
	std::string path;
  gWindowsWrapper.showSaveLevelDialog("lvl Files (*.lvl)\0*.lvl\0", directory, filename, path);
	
	std::ofstream writeFile;
	writeFile.open(path);
	writeFile << "<models>" << std::endl;
	for(unsigned int a = 0; a < m_vModels.size(); a++)
	{
		char buff[100];
		char buff2[100];
		char buff3[100];
		char buff4[100];
		char buff5[100];
		char buff6[100];
		//sprintf_s(buff,
		writeFile << "<mod>" << std::endl;
		writeFile << m_vModels[a]->path << std::endl;
		sprintf_s(buff,"%f %f %f",m_vModels[a]->orient.m11,m_vModels[a]->orient.m12,m_vModels[a]->orient.m13);
		sprintf_s(buff2,"%f %f %f",m_vModels[a]->orient.m21,m_vModels[a]->orient.m22,m_vModels[a]->orient.m23);
		sprintf_s(buff3,"%f %f %f",m_vModels[a]->orient.m31,m_vModels[a]->orient.m32,m_vModels[a]->orient.m33);
		sprintf_s(buff4,"%f %f %f",m_vModels[a]->orient.tx,m_vModels[a]->orient.ty,m_vModels[a]->orient.tz);
		sprintf_s(buff5,"%f %f %f",m_vModels[a]->rot.x,m_vModels[a]->rot.y,m_vModels[a]->rot.z);
		sprintf_s(buff6,"%f %f %f",m_vModels[a]->scale.x,m_vModels[a]->scale.y,m_vModels[a]->scale.z);

		writeFile << buff << std::endl
			<< buff2 << std::endl
			<< buff3 << std::endl
			<< buff4 << std::endl
			<< buff5 << std::endl
			<< buff6 << std::endl
			<< "</mod>" << std::endl;
		//std::ofstream teststream;
	}
	writeFile << "</models>";
	writeFile.close();

}

void Game::loadLevel()
{
	std::list<std::string> filenames;
	std::string directory;
	gWindowsWrapper.showOpenFileDialog(false, "lvl Files (*.lvl)\0*.lvl\0", directory, filenames);
	// make sure the user didn't press cancel
  if((int)filenames.size() == 0)
    return;
	while(!m_vModels.empty())	//Clear current work.
	{
		delete m_vModels.back()->buffer;
		delete m_vModels.back()->model;
		delete m_vModels.back();
		m_vModels.back() = NULL;
		m_vModels.pop_back();
	}
	std::string *fullpath = new std::string();
  *fullpath = directory + "\\" + (*(filenames.begin()));

	std::ifstream readFile;
	readFile.open(*fullpath);
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
	delete fullpath;
}

bool Game::copyRow(Vector3 *v, char* c)
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
				default:
					break;
			}
			ch = c[++j];
		}
		
	}
	return validString;		
		
}


bool Game::copyRow(Matrix4x3 *m,char* c,int row)
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

//Finds a tag in the given file.
//tag: tag to be found.
//file: the file to be searched.
//start: whether to search from the current iterator position or the start.
bool Game::findTag(char* tag, char* store, std::ifstream *file, bool start)
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

bool Game::checkExist(const char* filename)
{
	char testchar[MAX_PATH];
	sprintf_s(testchar,"%s\\%s",fileDir,filename);
	std::ifstream test(testchar);
	return test.is_open();
}

void Game::loadModelFromFile(Matrix4x3 m, Vector3 vRot, Vector3 vScale, char* pathName)
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

void Game::loadModel()
{
  // get file names of frames to animate
  std::list<std::string> filenames;
  std::string directory;
  gWindowsWrapper.showOpenFileDialog(false, "S3D Files (*.s3d)\0*.s3d\0", directory, filenames);

  // make sure the user didn't press cancel
  if((int)filenames.size() == 0)
    return;

  /*delete m_curModel; m_curModel = NULL;
  delete m_vertexBuffer; m_vertexBuffer = NULL;*/
	std::string pathName = (*(filenames.begin()));
  std::list<const char*> frames;
  while(filenames.begin() != filenames.end())
  {
    std::string *fullpath = new std::string();
    *fullpath = directory + "\\" + (*(filenames.begin()));
    frames.push_back(fullpath->c_str());
    filenames.pop_front();
		//delete fullpath;
  }
	AnimatedModel *tempMod = new AnimatedModel((int)frames.size());
	tempMod->m_bModelLerp = true;
	tempMod->importS3d(frames, false);
	ModelNode *tempModelNode = new ModelNode();
	tempModelNode->fullPath = (*(frames.begin()));
	/*if(tempTrack == 0)
	{
	  m_curModel = new AnimatedModel((int)frames.size());
	  m_curModel->m_bModelLerp = true;
	  m_curModel->importS3d(frames, false);
	}
	else if(tempTrack == 1)
	{
		m_curModel2 = new AnimatedModel((int)frames.size());
	  m_curModel2->m_bModelLerp = true;
	  m_curModel2->importS3d(frames, false);
	}*/

	if(!tempMod->isValid())
  {
    delete tempMod;
    tempMod = NULL;
    //m_vertexBuffer = NULL;
    return;
  }

 /* if(!m_curModel->isValid())
  {
    delete m_curModel;
    m_curModel = NULL;
    m_vertexBuffer = NULL;
    return;
  }*/
	tempModelNode->path = pathName;
  while(frames.begin() != frames.end())
  {
    delete (*(frames.begin()));
    frames.pop_front();
  }
	StandardVertexBuffer *tempBuf = tempMod->getNewVertexBuffer();
	Vector3 tempRot;
	tempRot.zero();
	Vector3 tempScale;
	tempScale.x = 1.0f;
	tempScale.y = 1.0f;
	tempScale.z = 1.0f;
	tempModelNode->model = tempMod;
	tempModelNode->buffer = tempBuf;
	tempModelNode->orient.identity();
	tempModelNode->rot = tempRot;
	tempModelNode->scale = tempScale;
	//tempModelNode->path 
	m_vModels.push_back(tempModelNode);
	/*tempNode->index = m_nListSize;
	tempNode->buffer = tempBuf;
	tempNode->model = tempMod;
	tempNode->previous = NULL;
	tempNode->next = NULL;*/
	/*if(m_nListSize == 0)
	{
		m_Head = tempNode;
		m_nListSize++;
	}
	else
	{
		ModNode *tempPoint = m_Head;
		while(tempPoint->index < m_nListSize-1)
		{
			tempPoint = tempPoint->next;
		}
		tempPoint->next = tempNode;
		tempNode->previous = tempPoint;
		m_nListSize++;
	}*/
}
