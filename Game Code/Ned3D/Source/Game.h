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
/// \brief Code to run game logic by deriving an object from class GameBase.

#ifndef __GAME_H_INCLUDED__
#define __GAME_H_INCLUDED__

#include "StatePlaying.h"
#include "StateMenu.h"
#include "StateIntro.h"
#include "ControlMenu.h"
#include "NextLevel.h"
#include "Game/GameBase.h"
#include "GameState.h"

/// All the game states
enum EGameState {eGameStateNone,eGameStatePlaying, eGameStateIntro, eGameStateMenu, eGameNextLevel, eGameControlMenu};

/// \brief The Ned Game class.
//
/// Controls all game logic.  Game is derived from base class GameBase.
/// 4 Member functions are then overridden ( initiate(), shutdown(), 
/// renderScreen(), and process() ).  Although not reccomended, main() can
/// be overriden for more flexibility but we don't need to for NED3D.
/// The WindowsWrapper class will initiate the game engine and then call the 
/// initiate function in this calss.  It will then repetitively call the Main 
/// method (overridable) until the program terminates.  The main
/// method in GameBase simply checks for a lost device, updates input, calls 
/// process() (which is overriden here), and then calls renderScreen()
/// (also overriden).  Upon termination, the Shutdown method will be called.
class Game: GameBase
{
public:
  friend StatePlaying;
	Game();	///< Constructor
	
  /// \brief Called by the windows wrapper after the game engine is initiated
	bool initiate();	
	void shutdown();	///< Called when the program is exiting

	void renderScreen(); ///< Renders entire screen
  
  void process(); ///< Called once a frame by GameBase::Main()
  void changeState(EGameState state); ///< Can be called to change the game state
  
  StatePlaying m_statePlaying;
  StateMenu m_stateMenu;
  StateIntro m_stateIntro;
  NextLevel m_nextLevel;
  ControlMenu m_controlMenu;
  
private:
  
  EGameState m_gameState; ///< Current state of the game
  GameState* m_state; ///< Pointer to current state
  EGameState m_nextGameState; ///< State that the game will transition to next
  /// \brief Flag that remembers if the game state needs to be changed
  bool m_mustChangeGameState; 
  
  std::vector<GameState*> m_allStates; ///< Array of pointers to all states
  
}; 

extern Game gGame; // extern a global instance of this object

#endif