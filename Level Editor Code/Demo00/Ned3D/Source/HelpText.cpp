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

#include "HelpText.h"
#include "Input/Input.h"
#include "Game/GameBase.h"
#include "Common/Renderer.h"
#include "Common/Camera.h"
#include "WindowsWrapper\WindowsWrapper.h"
#include <d3dx9.h>

extern WindowsWrapper gWindowsWrapper;
extern Renderer gRenderer;
extern InputManager gInput;
extern LPDIRECT3DDEVICE9 pD3DDevice;
extern GameBase* gGameBase;

HelpText::HelpText(Renderer* pRenderer) :
  m_pRenderer (pRenderer),
  m_bVisible  (true)
{}

bool HelpText::IsVisible(void)
{
  return m_bVisible;
}

void HelpText::Draw(void)
{
	HWND tempWindow = gWindowsWrapper.getHandle();
	Camera tempCam = gGameBase->getCam();
	Matrix4x3 viewMatrix1;
	tempCam.getViewMatrix(&viewMatrix1);

	D3DXMATRIX projMatrix;
	D3DVIEWPORT9 tempView;
	pD3DDevice->GetTransform(D3DTS_PROJECTION,&projMatrix);
	pD3DDevice->GetViewport(&tempView);
	Matrix4x3 tempMatrix = gRenderer.getWorldToCameraMatrix();
	
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

	char mouseX[10];
	char mouseY[10];
	POINT tempMouse;
	GetCursorPos(&tempMouse);
	ScreenToClient(tempWindow,&tempMouse);
	_itoa_s(tempMouse.x, mouseX, 10);
	_itoa_s(tempMouse.y, mouseY, 10);
	char mouseWX[10];
	char mouseWY[10];
	char mouseWZ[10];
	/*itoa(mouseVector.x, mouseWX, 10);
	itoa(mouseVector.y, mouseWY, 10);
	itoa(mouseVector.z, mouseWZ, 10);*/


	D3DXVECTOR4 testVec;
	
	testVec.x = (((2.0f * tempMouse.x) / tempView.Width ) - 1.0f );
	testVec.x = testVec.x / projMatrix._11;
	testVec.y = -(((2.0f * tempMouse.y) / tempView.Height ) - 1.0f );
	testVec.y = testVec.y / projMatrix._22;
	testVec.z = 1.0;

	D3DXMATRIX invView;
	D3DXMatrixInverse(&invView, NULL, &viewMatrix);
	D3DXVECTOR3 finalVec;
	D3DXVECTOR3 originVec;
	finalVec.x = testVec.x*invView._11 + testVec.y*invView._21 + testVec.z*invView._31;
	finalVec.y = testVec.x*invView._12 + testVec.y*invView._22 + testVec.z*invView._32;
	finalVec.z = testVec.x*invView._13 + testVec.y*invView._23 + testVec.z*invView._33;
	originVec.x = invView._41;
	originVec.y = invView._42;
	originVec.z = invView._43;

	D3DXVECTOR3 compVec;
	compVec = originVec + 200*finalVec;

	_itoa_s(compVec.x, mouseWX, 10.0f);
	_itoa_s(compVec.y, mouseWY, 10.0f);
	_itoa_s(compVec.z, mouseWZ, 10.0f);






	//testVec.x = tempMouse.x;
	//testVec.y = tempMouse.y;
	//testVec.z = 1.0;
	//testVec.w = 1.0;
	/*D3DXMATRIX invProj;
	D3DXMatrixInverse(&invProj, NULL, &projMatrix);*/
	/*multVec.x = testVec.x*invProj._11 + testVec.y*invProj._12 + testVec.z*invProj._13 + testVec.w*invProj._14;
	multVec.y = testVec.x*invProj._21 + testVec.y*invProj._22 + testVec.z*invProj._23 + testVec.w*invProj._24;
	multVec.z = testVec.x*invProj._31 + testVec.y*invProj._32 + testVec.z*invProj._33 + testVec.w*invProj._34;
	multVec.w = testVec.x*invProj._41 + testVec.y*invProj._42 + testVec.z*invProj._43 + testVec.w*invProj._44;*/

	/*viewMatrix = viewMatrix.inverse();
	D3DXVECTOR3 vec2;
	vec2.x = multVec.x*viewMatrix.m11 + multVec.y*viewMatrix.m21 + multVec.z*viewMatrix.m31;
	vec2.y = multVec.x*viewMatrix.m12 + multVec.y*viewMatrix.m22 + multVec.z*viewMatrix.m32;
	vec2.z = multVec.x*viewMatrix.m13 + multVec.y*viewMatrix.m23 + multVec.z*viewMatrix.m33;*/
	
	/*itoa(vec2.x, mouseWX, 10);
	itoa(vec2.y, mouseWY, 10);
	itoa(vec2.z, mouseWZ, 10);*/

  const int LEFT = 650; //left edge of help text
  if(m_bVisible)
  {
    m_pRenderer->drawText("ESCAPE - Exit Program"           , LEFT, 200);
    m_pRenderer->drawText("F2 - Load Model"                 , LEFT, 215);
    m_pRenderer->drawText("F3 - Change Background Color"    , LEFT, 230);
    m_pRenderer->drawText("Left Mouse Button - Rotate Model", LEFT, 245);
    m_pRenderer->drawText("Right Mouse Button - Zoom In/Out", LEFT, 260);
    m_pRenderer->drawText("TAB - Hide/Show Help Text"       , LEFT, 275);
		m_pRenderer->drawText("c - Open Camera menu"            , LEFT, 290);
		m_pRenderer->drawText("t - Open Translate menu"         , LEFT, 305);
		m_pRenderer->drawText("r - Open rotate menu"            , LEFT, 320);
		m_pRenderer->drawText("s - Open scale menu"             , LEFT, 335);
		m_pRenderer->drawText("F5 - Save Level"                 , LEFT, 350);
		m_pRenderer->drawText("F7 - Load Level"                 , LEFT, 365);
		
  }
  else
  {
    m_pRenderer->drawText("TAB - Hide/Show Help Text"       , LEFT, 10);
  }
}

void HelpText::Show(void)
{
  m_bVisible = true;
}

void HelpText::Hide(void)
{
  m_bVisible = false;
}

void HelpText::ToggleVisibility(void)
{
  m_bVisible = !m_bVisible;
}

void HelpText::ReverseColor(void)
{

}