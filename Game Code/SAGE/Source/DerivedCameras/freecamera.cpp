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

/// \file FreeCamera.cpp
/// \brief Code for the FreeCamera class.

#include "freecamera.h"
#include "input/input.h"
#include "common/mathutil.h"
#include "common/matrix4x3.h"
#include "common/vector3.h"
extern XINPUT_STATE state; //State for xinput controller



// processes movement and input
/// \param elapsed time in seconds since the last call to this function
void FreeCamera::process(float elapsed)
{  
  Matrix4x3 view;

  Vector3 movement = Vector3(0.0f,0.0f,0.0f);

  if (gInput.keyDown(DIK_UPARROW))
    m_cameraOrientMoving.pitch -= (kPi * 0.055f);
  
  if (gInput.keyDown(DIK_DOWNARROW))
    m_cameraOrientMoving.pitch += (kPi * 0.005f);
  
  if (gInput.keyDown(DIK_LEFTARROW))  
    m_cameraOrientMoving.heading -= (kPi * 0.005f);  

  if (gInput.keyDown(DIK_RIGHTARROW))  
    m_cameraOrientMoving.heading += (kPi * 0.005f); 

  if (gInput.keyDown(DIK_PGUP))
    movement.y = 1.0f;

  if (gInput.keyDown(DIK_PGDN))
    movement.y = -1.0f;


  // get mouse movement from input manager
  float lx = gInput.getMouseLX();
  float ly = gInput.getMouseLY();
  
  float cx = state.Gamepad.sThumbRX/5000;
  float cy = state.Gamepad.sThumbRY/5000;
  if(state.Gamepad.sThumbRX <= 5000 && state.Gamepad.sThumbRX >= -5000)
  {
	  cx = 0;
  }
  if(state.Gamepad.sThumbRY <= 5000 && state.Gamepad.sThumbRY >= -5000)
  {
	  cy = 0;
  }
  m_cameraOrientMoving.pitch += cy / 100.0f;
  m_cameraOrientMoving.heading += cx / 100.0f;  
    
  // add 80% of cameraOrientMoving to cameraOrient
  // then remove 80% of cameraOrientMoving
  cameraOrient += m_cameraOrientMoving * 0.8f;  
  m_cameraOrientMoving += m_cameraOrientMoving * -0.8f;

  // constrain pitch
  if (cameraOrient.pitch > kPiOver2)
    cameraOrient.pitch = kPiOver2;
  if (cameraOrient.pitch < -kPiOver2)
    cameraOrient.pitch = -kPiOver2;

  // wrap heading btw -pi and pi
  cameraOrient.heading = wrapPi(cameraOrient.heading);
    
  // create a matrix to transform the movement to world space
  view.setupParentToLocal(Vector3::kZeroVector, cameraOrient);
  view = view.inverse();
  movement = movement*view;

  cameraPos += movement * elapsed * m_speed;
}

// resets speed, oreination, and speed
void FreeCamera::reset()
{
  Camera::reset();
  m_cameraOrientMoving = EulerAngles::kEulerAnglesIdentity;
  m_speed = 400.0f;
}
