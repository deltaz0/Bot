#ifndef __SHADOWOBJECT_H_INCLUDED__
#define __SHADOWOBJECT_H_INCLUDED__

#include "Objects/GameObject.h"


class Shadow;

class ShadowObject : public GameObject
{
public:
  friend class Ned3DObjectManager;
  
  ShadowObject(Model *m);
  
  virtual void process(float dt);
  virtual void move(float dt);

  Shadow *getShadow() { return m_shadow; }

protected:
	Shadow *m_shadow;
};

#endif