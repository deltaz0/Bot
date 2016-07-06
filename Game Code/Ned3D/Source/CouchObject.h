#ifndef __COUCHOBJECT_H_INCLUDED__
#define __COUCHOBJECT_H_INCLUDED__

#include "Objects/GameObject.h"

/// \brief Represents a silo object
class CouchObject : public GameObject
{
public:
  friend class Ned3DObjectManager;
  
  CouchObject(Model *m);
  
  virtual void process(float dt);
  virtual void move(float dt);

protected:
};

#endif