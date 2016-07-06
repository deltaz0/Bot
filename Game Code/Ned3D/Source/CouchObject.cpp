#include <assert.h>
#include "ObjectTypes.h"
#include "CouchObject.h"

CouchObject::CouchObject(Model *m)
  : GameObject(m,1,1)
{
  assert(m);
  assert(m->getPartCount() >= 1);
  m_className = "Couch";
  m_type = ObjectTypes::COUCH;
}

void CouchObject::process(float dt)
{
}

void CouchObject::move(float dt)
{
  GameObject::move(dt);
}
