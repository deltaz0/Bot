#include <assert.h>
#include "ObjectTypes.h"
#include "SwingObject.h"

SwingObject::SwingObject(Model *m)
  : GameObject(m,1,1)
{
  assert(m);
  assert(m->getPartCount() >= 1);
  m_className = "Swing";
  m_type = ObjectTypes::SWING;
}

void SwingObject::process(float dt)
{
}

void SwingObject::move(float dt)
{
  GameObject::move(dt);
}
