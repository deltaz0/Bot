#ifndef __NEXTLEVEL_H_INCLUDED__
#define __NEXTLEVEL_H_INCLUDED__


#include "GameState.h"

/// \brief Represents the menu game state.
class NextLevel: public GameState
{
public:
  NextLevel();

  void initiate();

  void enterState();

  void process(float dt);
  void renderScreen();

private:
  int m_textureHandle;


};


#endif