#include "Common/Renderer.h"
#include "NextLevel.h"
#include "Input/Input.h"
#include "WindowsWrapper/WindowsWrapper.h"
#include "game.h"

extern bool fromMenu;
extern bool fromDeath;
extern bool fromVictory;
extern std::vector<ModelNode*> m_vModels;
extern XINPUT_STATE state;


NextLevel::NextLevel():
m_textureHandle(-1)
{


}

void NextLevel::initiate()
{
  m_textureHandle = gRenderer.cacheTexture("nextlevel.tga");
}

void NextLevel::enterState()
{
  gRenderer.setWireframe(false);
}

void NextLevel::process(float dt)
{
  if (gInput.keyJustDown(DIK_ESCAPE, true))
	{
		fromDeath = false;
    gGame.changeState(eGameStateMenu);
	}
   XInputGetState(0, &state);

  if (gInput.keyJustDown(DIK_N, true) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_A))
  {
		if(m_vModels.size() > 0)
		{
			gGame.changeState(eGameStatePlaying);
			//fromMenu = true;
			fromDeath = false;
		}
    gGame.m_statePlaying.resetGame();
		
  }
}

void NextLevel::renderScreen()
{
  gRenderer.setARGB(0XFFFFFFFF);
  gRenderer.selectTexture(m_textureHandle);
  gRenderer.renderTextureOverScreen();
}