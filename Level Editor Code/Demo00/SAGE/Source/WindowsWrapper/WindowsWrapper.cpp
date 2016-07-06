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

/// \file WindowsWrapper.cpp
/// Define code declared in WindowsWrapper.h.  Isolates the game engine user
/// from windows.
/// Last updated June 26th, 2005

#include <assert.h>

#include "game.h"
#include "WindowsWrapper.h"
#include "common/Renderer.h"
#include "Resource/ResourceManager.h"
#include "input/Input.h"
#include "directorymanager/directorymanager.h"
#include "Common/Renderer.h"

/// \brief WindowsWrapper global instance.
//
/// This can be accessed by anyone to get information such as the windows
/// handle or window isntance.
WindowsWrapper gWindowsWrapper;	
IDMan gIDMan;

extern Renderer gRenderer;
extern Game gGame;
extern GameBase* gGameBase;
extern HWND hWndPop;
/// WindowsWrapper Constructor.
/// Clears all member variables to their default values
WindowsWrapper::WindowsWrapper()
{
	hWndApp = NULL;
	hWndPop = NULL;
	hWndCam = NULL;
	hInstApp = NULL;
	idleInBackground = NULL;
	appInForeground = true;
	quitFlag = false;
	m_pGame = NULL;
	inEditX = false;
	inEditY = false;
	inEditZ = false;
	inMainWindow = false;
}

/// WindowsWrapper Destructor.  Does nothing.  Shutdown logic is in the
/// Shutdown method.
WindowsWrapper::~WindowsWrapper()
{
	
}

/// WindowsWrapper Shutdown.
/// Shuts down the game engine and Destroys the window.  Called in WinMainWrap.
void WindowsWrapper::Shutdown()
{
  gInput.shutdown();
	gRenderer.shutdown();

	destroyAppWindow();

	return;
}

/// WindowsWrapper Initiate
/// Asks user to select windowed or none windowed mode.  Creates the window.
/// Initiates basic engine objects (ex. Renderer, Input ...)
/// \param shaderDebugging Specifies whether or not shader debugging is
/// required.
/// \param loadingTexture Name of an image file to be displayed on the screen
/// during loading.  This image should be located in the default image
/// directory.
void WindowsWrapper::Initiate(bool shaderDebugging, const char* loadingTexture)
{

	char directory[2048];
	GetCurrentDirectory(2048, directory);

	gDirectoryManager.initiate(directory,"directories.xml");

	createAppWindow("Ultimate Model Viewer");
  
	// Create the main application window
	VideoMode mode;
	mode.xRes = 1024;
	mode.yRes = 768;
	mode.bitsPerPixel = 24;
	mode.refreshHz = kRefreshRateDefault;

	// Set the mode

  bool bWindowed = true; //set true for windowed, false for full screen

  gRenderer.init(mode, shaderDebugging, bWindowed); //set last param to true for windowed mode
  gInput.initiate(hInstApp, hWndApp);
  gDirectoryManager.setDirectory(eDirectoryXML);
	
	return;
}

/// WindowsWrapper RunProgram.
/// Calls Initiate on the game.  Repetitively calls main on the game until
/// quitFlag is true.  Calls Shutdown on the game when finished.
void WindowsWrapper::RunProgram()
{

	// if the game is null then leave
	if (m_pGame == NULL) 
  {
    assert("Game pointer is NULL. Must bail!");
		return;
  }
		
	// call initiate, if it signals an error then leave
	if (m_pGame->initiate() == false) 
  {
    assert("GameBase::initiate Failed");
		return;
  }
  
  // Reset renderer's page flip timer
  for(int i = 0; i < 2; ++i)
    gRenderer.flipPages();

	while(!quitFlag)
	{
		// processes messages
		idle(); 
    
    // if the quit flag was set then leave
    if (quitFlag) break;

		// call the main function for the GameBase Object passed in
		// if it signals an error then leave
		if (m_pGame->main() == false) break;

  } // end while
	
	
	m_pGame->shutdown();
	
	return;
}


/////////////////////////////////////////////////////////////////////////////
//
// global variables
//
/////////////////////////////////////////////////////////////////////////////

extern char g_errMsg[]; //error message if we had to abort
extern bool g_errorExit; // true if we had to abort

/////////////////////////////////////////////////////////////////////////////
//
// local stuff
//
/////////////////////////////////////////////////////////////////////////////


/// WindowsWrapper windows procedure.
/// Handler for messages from the Windows API. 
/// This function is called by the windows message pump
/// to process messages.  Do not call this function from your code!  Only
/// windows should call this function!
///  \param hWnd window handle
///  \param message message code
///  \param wParam parameter for message 
///  \param lParam second parameter for message
///  \return 0 if message is handled
LRESULT CALLBACK WindowsWrapper::WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{

	// Make sure this message is for our app
	if (hWnd != gWindowsWrapper.hWndApp && hWnd != gWindowsWrapper.hWndPop && hWnd != gWindowsWrapper.hWndCam
		 && hWnd != gWindowsWrapper.hWndTrn && hWnd != gWindowsWrapper.hWndScl) {
		return DefWindowProc(hWnd,message,wParam,lParam);
	}

	// Process the message
	switch(message)	
  {
		case WM_CLOSE:
			break;
		case WM_DESTROY:
    {
			gIDMan.removeID(LOWORD(wParam));
			// Display error message dialog box, if there is one
			if(g_errorExit)
				MessageBox(NULL, g_errMsg, "FATAL ERROR", MB_OK | MB_ICONERROR);
			// Now gettaoutahere
			if(hWnd == gWindowsWrapper.hWndApp)
				gWindowsWrapper.quitFlag = true;
			if(hWnd == gWindowsWrapper.hWndCam)
				gGame.toggleCamMenu(false);
			else if(hWnd == gWindowsWrapper.hWndPop)
			break;
    }
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case BUTTON_CANCEL:
				{
					if(gWindowsWrapper.hWndPop != NULL && hWnd == gWindowsWrapper.hWndPop)
					{
						DestroyWindow(gWindowsWrapper.hWndPop);
					}
					else if(gWindowsWrapper.hWndTrn != NULL && hWnd == gWindowsWrapper.hWndTrn)
					{
						DestroyWindow(gWindowsWrapper.hWndTrn);
						gGame.translateOpen = false;
					}
					else if(gWindowsWrapper.hWndScl != NULL && hWnd == gWindowsWrapper.hWndScl)
					{
						DestroyWindow(gWindowsWrapper.hWndScl);
					}
					break;
				}
				case BUTTON_OK:
				{
					char textx[100];
					char texty[100];
					char textz[100];
					//HWND *tempHandle = gIDMan.getID(EDITX);
					HWND tempHandle;
					if(hWnd == gWindowsWrapper.hWndPop)
					{
						if(gIDMan.getID(EDITX,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textx);
						if(gIDMan.getID(EDITY,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)texty);
						if(gIDMan.getID(EDITZ,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textz);
					}
					else if(hWnd == gWindowsWrapper.hWndTrn)
					{
						if(gIDMan.getID(TRANSX,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textx);
						if(gIDMan.getID(TRANSY,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)texty);
						if(gIDMan.getID(TRANSZ,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textz);
					}
					else if(hWnd == gWindowsWrapper.hWndScl)
					{
						if(gIDMan.getID(SCALEX,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textx);
						if(gIDMan.getID(SCALEY,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)texty);
						if(gIDMan.getID(SCALEZ,tempHandle))
							SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textz);
					}
					/*SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)texty);
					SendMessage(tempHandle,WM_GETTEXT,(WPARAM)100,(LPARAM)textz);*/
				
					bool xValidString = true;
					bool yValidString = true;
					bool zValidString = true;
					for(int a = 0; a < 100; a++)
					{
						if(textx[a] == 0)
							a = 100;
						else if(!(xValidString && ((textx[a] >= '0' && textx[a] <= '9') || textx[a] == '.' || textx[a] == '-')))
						{
							xValidString = false;
							a = 100;
						}
					}
					for(int a = 0; a < 100; a++)
					{
						if(texty[a] == 0)
							a = 100;
						else if(!(yValidString && ((texty[a] >= '0' && texty[a] <= '9') || texty[a] == '.' || texty[a] == '-')))
						{
							yValidString = false;
							a = 100;
						}
					}
					for(int a = 0; a < 100; a++)
					{
						if(textz[a] == 0)
							a = 100;
						else if(!(zValidString && ((textz[a] >= '0' && textz[a] <= '9') || textz[a] == '.' || textz[a] == '-')))
						{
							zValidString = false;
							a = 100;
						}
					}
					float floatX;
					float floatY;
					float floatZ;
					if(xValidString)
					{
						char ch = textx[0];
						int j = 0;
						float num2 = 0;
						int counter = 0;
						float deciNum = -1;
						int multiplier = 1;
						while((ch >= '0' && ch <= '9') || ch == '.' || ch == '-')
						{
							if(ch == '.')
							{
								deciNum = 0;
								ch = textx[++j];
							}
							else if(ch == '-')
							{
								multiplier *= -1;
								ch = textx[++j];
							}
							else if(deciNum < 0)
							{
								num2 = num2*10 + ch - '0';
								ch = textx[++j];
							}
							else
							{
								counter++;
								deciNum = deciNum + ((ch - '0')/(pow(10.0f,counter))); //handle decimals...
								ch = textx[++j];
							}
						}
					
						if(deciNum == -1)
							floatX = (num2)*multiplier;
						else
							floatX = (num2+deciNum)*multiplier;
					}
					if(yValidString)
					{
						char ch = texty[0];
						int j = 0;
						int num2 = 0;
						int counter = 0;
						float deciNum = -1;
						int multiplier = 1;
						while((ch >= '0' && ch <= '9') || ch == '.' || ch == '-')
						{
							if(ch == '.')
							{
								deciNum = 0;
								ch = texty[++j];
							}
							else if(ch == '-')
							{
								multiplier *= -1;
								ch = texty[++j];
							}
							else if(deciNum < 0)
							{
								num2 = num2*10 + ch - '0';
								ch = texty[++j];
							}
							else
							{
								counter++;
								deciNum = deciNum + ((ch - '0')/(pow(10.0f,counter))); //handle decimals...
								ch = texty[++j];
							}
						}
					
						if(deciNum == -1)
							floatY = (float)(num2)*multiplier;
						else
							floatY = (float)(num2+deciNum)*multiplier;
					}
					if(zValidString)
					{
						char ch = textz[0];
						int j = 0;
						int num2 = 0;
						int counter = 0;
						float deciNum = -1;
						int multiplier = 1;
						while((ch >= '0' && ch <= '9') || ch == '.' || ch == '-')
						{
							if(ch == '.')
							{
								deciNum = 0;
								ch = textz[++j];
							}
							else if(ch == '-')
							{
								multiplier *= -1;
								ch = textz[++j];
							}
							else if(deciNum < 0)
							{
								num2 = num2*10 + ch - '0';
								ch = textz[++j];
							}
							else
							{
								counter++;
								deciNum = deciNum + ((ch - '0')/(pow(10.0f,counter))); //handle decimals...
								ch = textz[++j];
							}
						}
					
						if(deciNum == -1)
							floatZ = (float)(num2)*multiplier;
						else
							floatZ = (float)(num2+deciNum)*multiplier;
					}
					if(!xValidString)
						floatX = 0;
					if(!yValidString)
						floatY = 0;
					if(!zValidString)
						floatZ = 0;
					std::vector<bool> tempVec;
					tempVec.push_back(xValidString);
					tempVec.push_back(yValidString);
					tempVec.push_back(zValidString);
					if(hWnd == gWindowsWrapper.hWndPop)
					{
						gGame.rotateModel(tempVec,floatX,floatY,floatZ);
					}
					else if(hWnd == gWindowsWrapper.hWndTrn)
					{
						gGame.translateModel(tempVec,floatX,floatY,floatZ);
					}
					else if(hWnd == gWindowsWrapper.hWndScl)
					{
						gGame.scaleModel(tempVec,floatX,floatY,floatZ);
					}
					
					break;
				}
				
				case EDITX:
				{
					if(!gWindowsWrapper.inEditX)
					{
						gWindowsWrapper.inEditX = true;
					}
					else
					{
						gWindowsWrapper.inEditX = false;
					}
					break;
				}
				case BUTTON_XYPLANEZPOS:
				{
					/*if(curDir == XYPLANEZNEG)
						gGameBase->m_currentCam->cameraPos.z -= 70;
					else if(curDir == XZPLANEYPOS)
					{
						gGameBase->m_currentCam->cameraPos.z += 70;
						gGameBase->m_currentCam->cameraPos.y -= 70;
					}*/
					gGameBase->m_currentCam->dir = XYPLANEZPOS;
					Vector3 newOrient;
					newOrient.x = 0; newOrient.y = 0; newOrient.z = 1;
					gGameBase->m_currentCam->setOrientation(newOrient);
					gRenderer.updateModelToWorldMatrix();
					SetActiveWindow(gWindowsWrapper.getHandle());
					gGame.updateCamMenu();
					break;
				}
				case BUTTON_XYPLANEZNEG:
				{
					//gGameBase->m_currentCam->cameraPos.z += 70;
					gGameBase->m_currentCam->dir = XYPLANEZNEG;
					Vector3 newOrient;
					newOrient.x = 0; newOrient.y = 0; newOrient.z = -1;
					gGameBase->m_currentCam->setOrientation(newOrient);
					gRenderer.updateModelToWorldMatrix();
					SetActiveWindow(gWindowsWrapper.getHandle());
					gGame.updateCamMenu();
					break;
				}
				case BUTTON_XZPLANEYPOS:
				{
					gGameBase->m_currentCam->dir = XZPLANEYPOS;
					Vector3 newOrient;
					newOrient.x = 0; newOrient.y = 1; newOrient.z = 0;
					gGameBase->m_currentCam->setOrientation(newOrient);
					gRenderer.updateModelToWorldMatrix();
					SetActiveWindow(gWindowsWrapper.getHandle());
					gGame.updateCamMenu();
					break;
				}
				case BUTTON_XZPLANEYNEG:
				{
					gGameBase->m_currentCam->dir = XZPLANEYNEG;
					Vector3 newOrient;
					newOrient.x = 0; newOrient.y = -1; newOrient.z = 0;
					gGameBase->m_currentCam->setOrientation(newOrient);
					gRenderer.updateModelToWorldMatrix();
					SetActiveWindow(gWindowsWrapper.getHandle());
					gGame.updateCamMenu();
					break;
				}
				case BUTTON_YZPLANEXPOS:
				{
					gGameBase->m_currentCam->dir = YZPLANEXPOS;
					Vector3 newOrient;
					newOrient.x = 1; newOrient.y = 0; newOrient.z = 0;
					gGameBase->m_currentCam->setOrientation(newOrient);
					gRenderer.updateModelToWorldMatrix();
					SetActiveWindow(gWindowsWrapper.getHandle());
					gGame.updateCamMenu();
					break;
				}
				case BUTTON_YZPLANEXNEG:
				{
					gGameBase->m_currentCam->dir = YZPLANEXNEG;
					Vector3 newOrient;
					newOrient.x = -1; newOrient.y = 0; newOrient.z = 0;
					gGameBase->m_currentCam->setOrientation(newOrient);
					gRenderer.updateModelToWorldMatrix();
					SetActiveWindow(gWindowsWrapper.getHandle());
					gGame.updateCamMenu();
					break;
				}
					
				default:
					break;
			}
			case WM_KEYDOWN:
			{
				if(wParam == VK_RETURN)
				{
					if(gWindowsWrapper.inEditX)
					{
						int stop = 0;
					}
				}
				break;
			}
			case WM_GETTEXT:
			{
				if(LOWORD(wParam) == EDITX)
				{
					int stop = 0;
				}
				break;
			}
			break;
		}
	}

	// Return the default window procedure
	return DefWindowProc(hWnd,message,wParam,lParam);
}

/// WindowsWrapper idle.
/// Contains windows message pump.  Gives time back to windows to process 
/// other programs.
void WindowsWrapper::idle() 
{

	// Check if there is a message for us, repeat if program is not
	// in the foreground

	for (;;) 
  {

		// Process all pending Windows messages

		MSG	msg;
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}

		// If we're in the foreground, return to normal message processing
		if (appInForeground) break;

		// If we've been told to quit the app, then bail this loop
		if (quitFlag) break;

		// We're in the background - always give back some time
		Sleep(20);

		// Check if we're supposed to idle in the background
		if (!idleInBackground) break;

		// Repeat until program is in the foreground,
		// or we signal program termination
	}

}

/// WindowsWrapper createAppWindow
/// Creates a new window for the application
void	WindowsWrapper::createAppWindow(const char *title) 
{

	// Register the window class

	
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstApp;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = title;
	RegisterClass(&wc);

	// Create our window

	hWndApp = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME,
		title,
		title,
		WS_POPUP | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
		0,
		0,
		0,
		0,
		NULL,
		NULL,
		hInstApp,
		NULL
	);
}


/// WindowsWrapper destroyAppWindow.
/// Destroys the application window.  Called from shutdown.
void	WindowsWrapper::destroyAppWindow() 
{
	// Make sure we exist
	if (hWndApp != NULL) 
  {
		// Destroy it
		DestroyWindow(hWndApp);
		hWndApp = NULL;
	}
}

bool WindowsWrapper::showSaveLevelDialog(char* fileFilter, std::string &directory, std::string &fileName, std::string &fullPath)
{
	char *name = new char[10000];
	name[0] = 0;

	OPENFILENAME ofn;
	const char* szFilter = fileFilter;

	DWORD flags = OFN_HIDEREADONLY | OFN_EXPLORER;

	ofn.lStructSize = sizeof(OPENFILENAME); // The API insists on this
  ofn.hwndOwner = hWndApp; // None
  ofn.hInstance = hInstApp; // None
  ofn.lpstrFilter = szFilter; // Show only s3d files
  ofn.lpstrCustomFilter = NULL; // None
  ofn.nMaxCustFilter = 0; // None
  ofn.nFilterIndex = 0; // None
  ofn.lpstrFile = name; // Where to put the file name from the dialog box
  ofn.nMaxFile = 10000; // Maximum length of file path
  ofn.lpstrFileTitle = NULL; // None
  ofn.nMaxFileTitle = 0; // None
  ofn.lpstrInitialDir = NULL; // Here
  ofn.lpstrTitle = NULL; // None
  ofn.Flags = flags;
  ofn.nFileOffset = 0;  // None
  ofn.nFileExtension = 0; // None
  ofn.lpstrDefExt = "lvl"; // Default file extension is s3d
  ofn.lCustData=0L; // None
  ofn.lpfnHook = NULL; // None
  ofn.lpTemplateName = NULL; // None

	BOOL result = GetSaveFileName(&ofn);

	if(!result)
	{
    return false;
  }

	fullPath = name;

	char c = name[ofn.nFileOffset];
  name[ofn.nFileOffset] = 0;

  directory = name;
  name[ofn.nFileOffset] = c;

	std::string file = &(name[ofn.nFileOffset]);
	fileName = file;
	return true;
}

/// This function displays a dialog to query the user for a file name (or multiple names).
///  \param multiFile Whether the dialog should allow multiple files to be selected
///  \param fileFilter File extension(s) to filter for
///  \param filePath Resulting file path(s) selected by the user
bool WindowsWrapper::showOpenFileDialog(
  bool multiFile,
  char* fileFilter,
  std::string &directory,
  std::list<std::string> &fileNames)
{
  // Pop up dialog box to ask user which model to load
  // Return name in string pointed to by "name"

  char *name = new char[10000];
  name[0] = 0; // Name is empty by default

  OPENFILENAME ofn; // Windows struct for storing file name
  const char* szFilter = fileFilter; //"S3D Files (*.s3d)\0*.s3d\0"; // Filter for s3d files

  // Initialize ofn structure to let user choose from s3d files
  DWORD flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
  if(multiFile)
    flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;

  ofn.lStructSize = sizeof(OPENFILENAME); // The API insists on this
  ofn.hwndOwner = hWndApp; // None
  ofn.hInstance = hInstApp; // None
  ofn.lpstrFilter = szFilter; // Show only s3d files
  ofn.lpstrCustomFilter = NULL; // None
  ofn.nMaxCustFilter = 0; // None
  ofn.nFilterIndex = 0; // None
  ofn.lpstrFile = name; // Where to put the file name from the dialog box
  ofn.nMaxFile = 10000; // Maximum length of file path
  ofn.lpstrFileTitle = NULL; // None
  ofn.nMaxFileTitle = 0; // None
  ofn.lpstrInitialDir = NULL; // Here
  ofn.lpstrTitle = NULL; // None
  ofn.Flags = flags;
  ofn.nFileOffset = 0;  // None
  ofn.nFileExtension = 0; // None
  ofn.lpstrDefExt = "s3d"; // Default file extension is s3d
  ofn.lCustData=0L; // None
  ofn.lpfnHook = NULL; // None
  ofn.lpTemplateName = NULL; // None

  // Windows common dialog box for file name

  BOOL result = GetOpenFileName(&ofn);

  if(!result)
  {
    fileNames.clear();
    return false;
  }
  
  char c = name[ofn.nFileOffset];
  name[ofn.nFileOffset] = 0;

  directory = name;
  name[ofn.nFileOffset] = c;

  int i = ofn.nFileOffset;

  if(!multiFile) // we only have one file to return
  {
    std::string file = &(name[ofn.nFileOffset]);
		fileNames.push_back(file);
  }
  else
  {
    // loop through all the filenames
    while(name[i] != 0)
    {
      std::string file = &(name[i]);
      fileNames.push_back(file);

      i += (int)file.size() + 1;
    }
  }

  fileNames.sort();

  return true;
}



/// WindowsWrapper WinMainWrap
/// This function should be called from the user's WinMain Function.
/// It handles initiating, running and shutting down the engine and game.
/// \param hInstance Handle to the window.
/// \param pGame Pointer to an object derived from GameBase.
/// \param loadingTexture A image name to be used as the loading screen.
/// The texture will be loaded from the default texture directory.  This
/// parameter can be se to NULL resulting in no loading screen.
/// \param shaderDebugging Optional parameter specifying whether or not you 
/// will be debugging a pixel/vertex shader.  Setting this to true will create
/// the device in software.  This parameter defaults to false.
void WindowsWrapper::WinMainWrap(HINSTANCE hInstance, GameBase* pGame, const char* loadingTexture, bool shaderDebugging)
{
	// save pointer to game object derived from class GameBase
	m_pGame = pGame;
  
  // save a global pointer to the game
  gGameBase = pGame;
	
	// save the instance of the window
	hInstApp = hInstance;

	// Create the window and initate the engine
	Initiate(shaderDebugging, loadingTexture);

	// Run the actual game
	RunProgram();

	// Shutdown the game engine and destroy the window
	Shutdown();

  return;
}
