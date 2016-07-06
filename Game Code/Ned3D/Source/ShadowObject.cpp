#include <assert.h>
#include "ObjectTypes.h"
#include "ShadowObject.h"
#include "Ned3DObjectManager.h"
#include "PlaneObject.h"
#include "TerrainObject.h"
#include "Terrain\Terrain.h"

Ned3DObjectManager m_objects;


ShadowObject::ShadowObject(Model *m): 
    GameObject(m,1)
{

    assert(m);
    assert(m->getPartCount() >= 1);
    m_className = "Shadow";
    m_type = ObjectTypes::SHADOW;
}

void ShadowObject::process(float dt)
{
    //this->m_oldPosition = this->getPosition();
}

void ShadowObject::move(float dt)
{
    /*
    //m_objects = new Ned3DObjectManager;
    PlaneObject* plane = m_objects.getPlaneObject();
    TerrainObject* terr = m_objects.getTerrainObject();
    Terrain *ter = terr->getTerrain();
    Vector3 v = plane->getPosition();
    v.y = ter->getHeight(v.x,v.z) + 0.05f;
    v.x = 1.0f;
    v.z = 1.0f;
    this->setPosition(v);
    */
    GameObject::move(dt);
}
