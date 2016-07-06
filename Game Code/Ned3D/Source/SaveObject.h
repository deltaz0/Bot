#ifndef __SAVEOBJECT_H_INCLUDED__
#define __SAVEOBJECT_H_INCLUDED__

#include "Objects/GameObject.h"

/// \brief Represents a silo object

class Save;

class SaveObject : public GameObject
{
public:
  friend class Ned3DObjectManager;
  
  SaveObject(Model *m);
  
  virtual void process(float dt);
  virtual void move(float dt);
  void SetSaveParticle();


  int savepartHndl;
  int savepartHnd2;
  int savepartHnd3;


  std::vector<std::string> m_allParticles; 

  Save *getSave() { return m_save; }

protected:
	Save *m_save;
};

#endif