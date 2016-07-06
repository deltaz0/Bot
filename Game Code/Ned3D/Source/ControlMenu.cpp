#include "Common/Renderer.h"
#include "ControlMenu.h"
#include "Input/Input.h"
#include "WindowsWrapper/WindowsWrapper.h"
#include "game.h"

extern std::vector<ModelNode*> m_vModels;
extern XINPUT_STATE state;

ControlMenu::ControlMenu():
m_textureHandle(-1)
{


}

void ControlMenu::initiate()
{
  // load the texture
  m_textureHandle = gRenderer.cacheTexture("controlmenu.tga");


}

void ControlMenu::enterState()
{
  // make sure wireframe is off
  gRenderer.setWireframe(false);

}

void ControlMenu::process(float dt)
{
  XInputGetState(0, &state);
  if ((gInput.keyJustDown(DIK_ESCAPE, true)) || (gInput.keyJustDown(DIK_C, true)) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_B))
     gGame.changeState(eGameStateMenu);
}

void ControlMenu::renderScreen()
{
  gRenderer.setARGB(0XFFFFFFFF);
  gRenderer.selectTexture(m_textureHandle);
  gRenderer.renderTextureOverScreen();
}