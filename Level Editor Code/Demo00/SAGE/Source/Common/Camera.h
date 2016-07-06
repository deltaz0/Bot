/// \file Camera.h
#ifndef __CAMERA_H_INCLUDED__
#define __CAMERA_H_INCLUDED__

#define XYPLANEZPOS 0
#define XYPLANEZNEG 1
#define XZPLANEYPOS 2
#define XZPLANEYNEG 3
#define YZPLANEXPOS 4
#define YZPLANEXNEG 5

#include "EulerAngles.h"
#include "vector3.h"

/// \brief Base camera class.
/// The free camera and tether camera are derived
/// from this.
class Camera
{ 
public:
  Camera(); ///< Basic Constructor

  /// \brief Returns the view matrix of the camera
  void getViewMatrix(Matrix4x3 * matrix); 
  
  /// \brief Sets this camera as the current camera through gRenderer.
  void setAsCamera();

  void setOrientation (Vector3 look);
  
  /// \brief Processes camera.  Is called once per frame.
  virtual void process(float elapsed) {}
  
  /// \brief Resets camera to default position.
  virtual void reset();
  
	int dir;
  Vector3 cameraPos; ///< Position of the camera
  EulerAngles cameraOrient; ///< Orientation of the camera
  float fov; ///< Field of view of the camera.

};

#endif