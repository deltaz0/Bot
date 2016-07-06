#ifndef __CONTROLMENU_H_INCLUDED__
#define __CONTROLMENU_H_INCLUDED__


#include "GameState.h"

/// \brief Represents the menu game state.
class ControlMenu : public GameState
{
public:
  ControlMenu();

  void initiate();

  void enterState();

  void process(float dt);
  void renderScreen();

private:
  int m_textureHandle;


};


#endif