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

/// \file StateMenu.cpp
/// \brief Code for the StateMenu class.

/// Edited by the SAGE team, 2005
/// Last updated June 13, 2006

#include "Common/Renderer.h"
#include "StateMenu.h"
#include "ControlMenu.h"
#include "Input/Input.h"
#include "WindowsWrapper/WindowsWrapper.h"
#include "StatePlaying.h"
#include "game.h"

extern StatePlaying gStatePlaying;
extern bool fromMenu;
extern bool fromDeath;
extern bool fromVictory;
extern bool hitPause;
extern std::vector<ModelNode*> m_vModels;
extern XINPUT_STATE state;

StateMenu::StateMenu():
m_textureHandle(-1)
{


}

void StateMenu::initiate()
{
  // load the texture
  m_textureHandle = gRenderer.cacheTexture("newMenu.tga");


}

void StateMenu::enterState()
{
  // make sure wireframe is off
  gRenderer.setWireframe(false);

}

void StateMenu::process(float dt)
{
  XInputGetState(0, &state);
  if(!state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK && hitPause)
	  hitPause = false;
  if (gInput.keyJustDown(DIK_ESCAPE, true) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK && !hitPause))
    gWindowsWrapper.quit();
  if (gInput.keyJustDown(DIK_R, true) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && m_vModels.size() > 0)
    gGame.changeState(eGameStatePlaying);

  if (gInput.keyJustDown(DIK_C, true) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_B))
	gGame.changeState(eGameControlMenu);

  if (gInput.keyJustDown(DIK_S, true) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_START))
  {
		gGame.changeState(eGameStatePlaying);
		fromMenu = true;
		fromDeath = false;
		fromVictory = false;
    gGame.m_statePlaying.resetGame();

		
  }
  

}

void StateMenu::renderScreen()
{
  gRenderer.setARGB(0XFFFFFFFF);
  gRenderer.selectTexture(m_textureHandle);
  gRenderer.renderTextureOverScreen();
}