#ifndef __SWINGOBJECT_H_INCLUDED__
#define __SWINGOBJECT_H_INCLUDED__

#include "Objects/GameObject.h"

/// \brief Represents a silo object
class SwingObject : public GameObject
{
public:
  friend class Ned3DObjectManager;
  
  SwingObject(Model *m);
  
  virtual void process(float dt);
  virtual void move(float dt);

protected:
};

#endif