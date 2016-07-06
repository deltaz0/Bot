#include <assert.h>
#include "ObjectTypes.h"
#include "SaveObject.h"



SaveObject::SaveObject(Model *m): 
   GameObject(m,1,1)
{
  assert(m);
  assert(m->getPartCount() >= 1);
  m_className = "Save";
  m_type = ObjectTypes::SILO;

  m_allParticles.resize(3);
  m_allParticles[0] = "saveglow1";   // when hp = 0
  m_allParticles[1] = "saveglow2";   // when hp = 0
  m_allParticles[2] = "saveglow3";   // when hp = 0


}



void SaveObject::process(float dt)
{
}

void SaveObject::move(float dt)
{
  GameObject::move(dt);
}
