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

/// \file Ned3DObjectManager.cpp
/// \brief Code for the Ned3DObjectManager class, which is responsible for all
/// objects specific to this demo.

/// Edited by the SAGE team, 2005
/// Last updated June 13, 2006

#define DEGTORAD(degree) ((D3DX_PI / 180.0f) * (degree)) // converts from degrees to radians

#include <algorithm>
#include "Common/MathUtil.h"
#include "Common/RotationMatrix.h"
#include "Graphics/ModelManager.h"
#include "Objects/GameObject.h"
#include "Particle/ParticleEngine.h"
#include "Sound/SoundManager.h"
#include "Terrain/Terrain.h"
#include "Water/Water.h"
#include "Ned3DObjectManager.h"
#include "Objects.h"
#include "ObjectTypes.h"
#include "DirectoryManager/DirectoryManager.h"
#include "math.h"
#include "SaveObject.h"
#include "CouchObject.h"
#include "StatePlaying.h"
#include "PlaneObject.h"

extern bool fromVictory;
extern StatePlaying gStatePlaying;
//


PlaneObject* gPlane;

Ned3DObjectManager::Ned3DObjectManager() :
  m_models(NULL),
  m_planeModel(NULL),
  m_crowModel(NULL),
  m_bulletModel(NULL),
  m_siloModel(NULL),
  m_swingModel(NULL),
 // m_couchModel(NULL),
  m_saveModel(NULL),
  m_shadowModel(NULL),
  m_windmillModel(NULL),
  m_plane(NULL),
  m_save(NULL),
  m_shadow(NULL),
  m_terrain(NULL),
  m_water(NULL),
  onFurn(FALSE),
  shadHeight(0.0f),
	savePos(0.0f,90.0f,0.0f),
	winTime(0.0f)
{
}

void Ned3DObjectManager::setModelManager(ModelManager &models)
{
  if(m_models == &models)
    return;
  m_models = &models;
  m_planeModel = NULL;
  m_crowModel = NULL;
  m_bulletModel = NULL;
  m_siloModel = NULL;
  m_swingModel = NULL;
  m_saveModel = NULL;
  m_shadowModel = NULL;
  m_windmillModel = NULL;
}

void Ned3DObjectManager::clear()
{
  m_planeModel = NULL;
  m_crowModel = NULL;
  m_bulletModel = NULL;
  m_siloModel = NULL;
  m_swingModel = NULL;
  m_saveModel = NULL;
  m_shadowModel = NULL;
  m_windmillModel = NULL;
  m_plane = NULL;
  m_save = NULL;
  m_shadow = NULL;
  m_terrain = NULL;
  m_water = NULL;
  m_crows.clear();
  m_bullets.clear();
  m_furniture.clear();
  m_swings.clear();
  //m_couches.clear();
  m_saves.clear();
  GameObjectManager::clear();
}

void Ned3DObjectManager::handleInteractions()
{
    onFurn = false;

    Vector3 tPPos = m_plane->getPosition();
    shadHeight = m_terrain->getTerrain()->getHeight(tPPos.x,tPPos.z);

  for(ObjectSetIter fit = m_furniture.begin(); fit != m_furniture.end(); ++fit)
    interactPlaneFurniture(*m_plane, **fit);
  
  for(ObjectSetIter saveit = m_saves.begin(); saveit != m_saves.end(); ++saveit){
	 interactSave(*m_plane, **saveit);
  }

  for(ObjectSetIter swit = m_swings.begin(); swit != m_swings.end(); ++swit)
  {
      //SwingObject &swing1 = (SwingObject &)**swit;
      interactPlaneSwing(*m_plane, **swit);
  }

  for(ObjectSetIter chit = m_couches.begin(); chit != m_couches.end(); ++chit)
  {
	  interactPlaneCouch(*m_plane, **chit);
  }

  
  for(ObjectSetIter bit = m_bullets.begin(); bit != m_bullets.end(); ++bit)
  {
    BulletObject &bullet = (BulletObject &)**bit;
    if(!bullet.isAlive()) continue;
      
    // Check for bullets hitting stuff
    for(ObjectSetIter cit = m_crows.begin(); cit != m_crows.end(); ++cit)
    {
      CrowObject &crow = (CrowObject &)**cit;
      if(!crow.isAlive()) continue;
      interactCrowBullet(crow, bullet);
    }
    GameObject *victim = bullet.getVictim();
    if(victim != NULL)
    {
      // Bullet hit something
      switch(victim->getType())
      {
        case ObjectTypes::CROW :
        {
          shootCrow((CrowObject &)*victim);
        } break;
      }
    }
  }
  
  // Handle crow-crow interactions (slow....) and crow-plane interactions
  
  for(ObjectSetIter cit1 = m_crows.begin(); cit1 != m_crows.end(); ++cit1)
  {
    CrowObject &crow1 = (CrowObject &)**cit1;
    if(!crow1.isAlive()) continue;
    
    interactPlaneCrow(*m_plane,crow1);
    interactCrowTerrain(crow1,*m_terrain);
    
    ObjectSetIter cit2 = cit1;
    for(++cit2; cit2 != m_crows.end(); ++cit2)
    {
      CrowObject &crow2 = (CrowObject &)**cit2;
      if(!crow2.isAlive()) continue;
      interactCrowCrow(crow1, crow2);
    }
  }
  
  // Handle plane crashes
  interactJump(*m_plane, *m_terrain);
  interactPlaneTerrain(*m_plane, *m_terrain);
  interactPlaneWater(*m_plane, *m_water);
}

void Ned3DObjectManager::lockMovement()
{
	m_plane->lockMovement = true;
	m_plane->setSpeed(0);
	m_plane->setySpeed(0);
}

unsigned int Ned3DObjectManager::spawnPlane(const Vector3 &position, const EulerAngles &orientation)
{
  if(m_plane != NULL)
    return 0;  // Only one plane allowed
  if(m_planeModel == NULL)
    m_planeModel = m_models->getModelPointer("Plane"); // Cache plane model
  if(m_planeModel == NULL)
    return 0;  // Still NULL?  No such model
  m_plane = new PlaneObject(m_planeModel);
  m_plane->setPosition(position);
  m_plane->setOrientation(orientation);
  unsigned int id = addObject(m_plane, "Plane");
  return id;
}

unsigned int Ned3DObjectManager::spawnCrow(const Vector3 &position, const EulerAngles &orientation, float speed)
{
  if(m_crowModel == NULL)
    m_crowModel = m_models->getModelPointer("Crow"); // Cache crow model
  if(m_crowModel == NULL)
    return 0;  // Still NULL?  No such model
  CrowObject *crow = new CrowObject(m_crowModel);
  crow->setSpeed(speed);
  crow->setPosition(position);
  crow->setOrientation(orientation);
  crow->setMovementPattern(CrowObject::MP_STRAIGHT);
  unsigned int id = addObject(crow);
  m_crows.insert(crow);
  return id;
}

unsigned int Ned3DObjectManager::spawnCrow(const Vector3 &position, const Vector3 &circleCenter, float speed, bool flyLeft)
{
  if(m_crowModel == NULL)
    m_crowModel = m_models->getModelPointer("Crow"); // Cache crow model
  if(m_crowModel == NULL)
    return 0;  // Still NULL?  No such model
  CrowObject *crow = new CrowObject(m_crowModel);
  crow->setSpeed(speed);
  crow->setPosition(position);
  crow->setCirclingParameters(circleCenter, flyLeft);
  crow->setMovementPattern(CrowObject::MP_CIRCLING);
  unsigned int id = addObject(crow);
  m_crows.insert(crow);
  return id;
}

unsigned int Ned3DObjectManager::spawnBullet(const Vector3 &position, const EulerAngles &orientation)
{
  BulletObject *bullet = new BulletObject();
  bullet->setPosition(position);
  bullet->setOrientation(orientation);
  unsigned int id = addObject(bullet);
  m_bullets.insert(bullet);
  return id;
}

unsigned int Ned3DObjectManager::spawnTerrain(Terrain *terrain)
{
  m_terrain = new TerrainObject(terrain);
  return addObject(m_terrain, false, false, false, "Terrain");
}

unsigned int Ned3DObjectManager::spawnWater(Water *water)
{
  m_water = new WaterObject(water);
  return addObject(m_water, false, false, false, "Water");
}

unsigned int Ned3DObjectManager::spawnObject(Matrix4x3 pos, int index, Vector3 rot, std::string modName)
{
	std::string tempName = modName;
	if(modName == "swing.s3d" || modName == "sphere.s3d")
	{
		EulerAngles angles;
		angles.set(DEGTORAD(rot.y),DEGTORAD(rot.x),DEGTORAD(rot.z));
		return spawnObjectNF(pos.getTranslation(),true,angles,modName);
	}
	else
	{
		if(modName == "cylo1.s3d")
		{
			tempName = "Silo1";
		}
		else if(modName == "cylo3.s3d")
		{
			tempName = "Silo3";
		}
		Model *tempModel = m_models->getModelPointer(tempName);
		if(tempModel == NULL)
			return 0;	//no model.
		GameObject *newObjectc = new GameObject(tempModel);
		Vector3 tempVecPos = pos.getTranslation();
		newObjectc->setPosition(tempVecPos);
		EulerAngles tempAngles;

		tempAngles.pitch = DEGTORAD(rot.x);
		tempAngles.heading = DEGTORAD(rot.y);
		tempAngles.bank = DEGTORAD(rot.z);
		newObjectc->setOrientation(tempAngles);
		newObjectc->setLoadedFromFile();
		newObjectc->setPosMatrix(&pos);
		newObjectc->setIndex(index);
		unsigned int id = addObject(newObjectc,false,false,true,modName);
		m_furniture.insert(newObjectc);

		return id;
		}

}

unsigned int Ned3DObjectManager::spawnObjectNF(const Vector3 &position, bool rotate, const EulerAngles &orientation, std::string modName)
{
	std::string tempName;
	if(modName == "swing.s3d")
		tempName = "Swing";
	else if(modName == "sphere.s3d")
		tempName = "Save";
 	//else if(modName == "AR_COUCH.s3d")
	//	tempName = "Couch";
  //if(m_swingModel == NULL)
  //  m_swingModel = m_models->getModelPointer("Swing"); // Cache silo model
	Model *tempModel = m_models->getModelPointer(tempName);
	if(tempModel == NULL)
		return 0;	//no model.

	/*
	GameObject *newObject = new GameObject(tempModel);
	Vector3 tempVecPos = pos.getTranslation();
	newObject->setPosition(tempVecPos);
	EulerAngles tempAngles;

	tempAngles.pitch = DEGTORAD(rot.x);
	tempAngles.heading = DEGTORAD(rot.y);
	tempAngles.bank = DEGTORAD(rot.z);
	newObject->setOrientation(tempAngles);
	newObject->setLoadedFromFile();
	newObject->setPosMatrix(&pos);
	newObject->setIndex(index);
	unsigned int id = addObject(newObject,false,false,true,modName);
	*/
	unsigned int id = 0;
	if(tempName == "Swing")
	{
		SwingObject *swing = new SwingObject(tempModel);
		swing->setPosition(position);
		EulerAngles orient = orientation;
		if(rotate==true)
				orient.heading = orientation.heading + (kPi * 0.5f);
		swing->setOrientation(orient);
		id = addObject(swing);
		m_swings.insert(swing);
	}
	else if(tempName == "Save")
	{
		/*SaveObject *save = new SaveObject(m_saveModel);
		save->setPosition(position);
		save->setOrientation(orientation);
		id = addObject(save);*/
		SaveObject *tempSave = new SaveObject(tempModel);
		tempSave->setPosition(position);
		EulerAngles orient = orientation;
		orient.pitch += kPiOver2;
		tempSave->setOrientation(orientation);
		unsigned int id = addObject(tempSave);
		m_saves.insert(tempSave);
	}
	/*else if(tempName == "Couch")
	{
		CouchObject *couch = new CouchObject(tempModel);
		couch->setPosition(position);
		EulerAngles orient = orientation;
		orient.pitch += kPiOver2;
		couch->setOrientation(orientation);
		unsigned int id = addObject(couch);
		m_couches.insert(couch);

	}*/
		//
	return id;
	/*if(tempName == "Swing")
	{
		SwingObject *swing = new SwingObject(tempModel);
		Vector3 tempVecPos = pos.getTranslation();
		swing->setPosition(tempVecPos);
		EulerAngles orient = orientation;
		if(rotate==true)
		    orient.heading = orientation.heading + (kPi * 0.5f);
		swing->setOrientation(orient);
		unsigned int id = addObject(swing);
		m_swings.insert(swing);
	}
  return id;*/
}

unsigned int Ned3DObjectManager::spawnSilo(const Vector3 &position, const EulerAngles &orientation)
{
  static const std::string silos[] = {"Silo1","Silo2","Silo3","Silo4"};
  static int whichSilo = 0;
  m_siloModel = m_models->getModelPointer(silos[whichSilo]); // Cache silo model
  if(m_siloModel == NULL)
    return 0;  // Still NULL?  No such model
  SiloObject *silo = new SiloObject(m_siloModel);
  silo->setPosition(position);
  silo->setOrientation(orientation);
  unsigned int id = addObject(silo);
  m_furniture.insert(silo);
  //whichSilo = ++whichSilo % 4;
  whichSilo = 0;
  return id;
}


unsigned int Ned3DObjectManager::spawnSwing(const Vector3 &position, bool rotate, const EulerAngles &orientation)
{
  if(m_swingModel == NULL)
    m_swingModel = m_models->getModelPointer("Swing"); // Cache silo model
  if(m_swingModel == NULL)
    return 0;  // Still NULL?  No such model
  SwingObject *swing = new SwingObject(m_swingModel);
  swing->setPosition(position);
  EulerAngles orient = orientation;
  if(rotate==true)
      orient.heading = orientation.heading + (kPi * 0.5f);
  swing->setOrientation(orient);
  unsigned int id = addObject(swing);
  m_swings.insert(swing);
  return id;
}


unsigned int Ned3DObjectManager::spawnShadow(const Vector3 &position, const EulerAngles &orientation)
{
  if(m_shadowModel == NULL)
    m_shadowModel = m_models->getModelPointer("Shadow"); // Cache windmill model
  if(m_shadowModel == NULL)
    return 0;  // Still NULL?  No such model
  m_shadow = new ShadowObject(m_shadowModel);
  m_shadow->setPosition(position);
  EulerAngles orient = orientation;
  orient.pitch += kPiOver2;
  m_shadow->setOrientation(orient);
  unsigned int id = addObject(m_shadow);
  m_furniture.insert(m_shadow);
  return id;
}

unsigned int Ned3DObjectManager::spawnSave(const Vector3 &position, const EulerAngles &orientation){

  if(m_saveModel == NULL)
    m_saveModel = m_models->getModelPointer("Save"); // Cache windmill model
  if(m_saveModel == NULL)
    return 0;  // Still NULL?  No such model
  m_save = new SaveObject(m_saveModel);
  m_save->setPosition(position);
  EulerAngles orient = orientation;
  orient.pitch += kPiOver2;
  m_save->setOrientation(orientation);
  unsigned int id = addObject(m_save);
  m_furniture.insert(m_save);
  return id;

}

unsigned int Ned3DObjectManager::spawnWindmill(const Vector3 &position, const EulerAngles &orientation)
{
  if(m_windmillModel == NULL)
    m_windmillModel = m_models->getModelPointer("Windmill"); // Cache windmill model
  if(m_windmillModel == NULL)
    return 0;  // Still NULL?  No such model
  WindmillObject *windmill = new WindmillObject(m_windmillModel);
  windmill->setPosition(position);
  windmill->setPosition(Vector3(0,27.0f,-0.5f),1);
  windmill->setRotationSpeedBank(kPiOver2,1);
  windmill->setOrientation(orientation);
  unsigned int id = addObject(windmill);
  m_furniture.insert(windmill);
  return id;
}

// Returns a handle to the first crow in the list
unsigned int Ned3DObjectManager::getCrow()
{
  ObjectSetIter cit = m_crows.begin();
  if (cit == m_crows.end())
    return - 1;

  return (*cit)->getID();
}

// Returns true if a crow intersects the ray
bool Ned3DObjectManager::rayIntersectCrow(const Vector3 &position, const Vector3 direction)
{
  
  // Check for bullets hitting crows
  for(ObjectSetIter cit = m_crows.begin(); cit != m_crows.end(); ++cit)
  {
    CrowObject &crow = (CrowObject &)**cit;
    if(!crow.isAlive()) continue;
    
    float t = crow.getBoundingBox().rayIntersect(position,direction);
    if(t <= 1.0f) return true;        
  }

  return false;
}

void Ned3DObjectManager::deleteObject(GameObject *object)
{
  if(object == m_plane)
    m_plane = NULL;
  else if(object == m_terrain)
    m_terrain = NULL;
  else if(object == m_water)
    m_water = NULL;
  m_crows.erase(object);
  m_bullets.erase(object);
  m_furniture.erase(object);
  m_swings.erase(object);
 // m_couches.erase(object);
  m_saves.erase(object);
  GameObjectManager::deleteObject(object);
}

bool Ned3DObjectManager::interactPlaneCrow(PlaneObject &plane, CrowObject &crow)
{
  bool collided = enforcePositions(plane, crow);
  if(collided && !crow.isDying())
  {
    shootCrow(crow);
    plane.damage(1);
  }
  return collided;
}



bool Ned3DObjectManager::interactPlaneSwing(PlaneObject &plane, GameObject &swing)
{
    const AABB3 &box1 = plane.getBoundingBox(), &box2 = swing.getBoundingBox();
    AABB3 intersectBox;
    if(AABB3::intersect(box1, box2, &intersectBox))
    {
        if((gRenderer.getTime()>(plane.swingTimer + 300))&&(plane.swingState==0))
        {
            Vector3 sPos = swing.getPosition();
            plane.setSwingPos(sPos);
            float pH = plane.getOrientation().heading;
            pH = plane.simplifyAngle(pH);
            if(box2.size().z<box2.size().x)
            {
                if((pH>(kPi*0.5f))&&(pH<(kPi*1.5f)))
                    plane.swingState = 2;
                else
                    plane.swingState = 4;
            }
            else
            {
                if(pH<kPi)
                    plane.swingState = 1;
                else
                    plane.swingState = 3;
            }
            plane.setSwingTimer(gRenderer.getTime());
        }
	    int partHndl = gParticle.createSystem("swingspark");
        gParticle.setSystemPos(partHndl, swing.getPosition());
        return true;
    }
    else
        return false;
}

bool Ned3DObjectManager::interactPlaneCouch(PlaneObject &plane, GameObject &couch)
{
  Vector3 planePos = plane.getPosition();
  EulerAngles planeOrient = plane.getOrientation();
  Vector3 disp = planePos - disp;
  RotationMatrix planeMatrix;
  planeMatrix.setup(plane.getOrientation()); // get plane's orientation
  Vector3 couchPos;
  couchPos = couch.getPosition();



  const AABB3 &box1 = plane.getBoundingBox();
  const AABB3 &box2 = couch.getBoundingBox();
  AABB3 intersectBox;

  if(AABB3::intersect(box1,box2, &intersectBox)){
		int partHndl = gParticle.createSystem("saveglow1");
        int partHnd2 = gParticle.createSystem("saveglow2");
        gParticle.setSystemPos(partHndl, couchPos);
        gParticle.setSystemPos(partHnd2, couchPos);
	  //int textY = gRenderer.getScreenY()/2;
   //   IRectangle rect = IRectangle(0,textY,gRenderer.getScreenX()-1, textY + 30);
   //   gRenderer.drawText("Saving...",&rect, eTextAlignModeCenter, false);

		 char text[1024];      
  //SECURITY-UPDATE:2/3/07
  //sprintf(text, "FPS: %d\nTriangles Per Frame: %d", m_fps, tri, 2);
  sprintf_s(text,sizeof(text), "You Completed the Level!");
  // draw the text
  gRenderer.drawText(text, 30,30);

  }
  return true;


}

bool Ned3DObjectManager::interactPlaneTerrain(PlaneObject &plane, TerrainObject &terrain)
{
  Terrain *terr = terrain.getTerrain();
  if(terr == NULL) return false;

  //test for plane collision with terrain
  Vector3 planePos = plane.getPosition();
  EulerAngles planeOrient = plane.getOrientation();
  Vector3 disp = planePos - disp;
  RotationMatrix planeMatrix;
  planeMatrix.setup(plane.getOrientation()); // get plane's orientation

  float planeBottom = plane.getBoundingBox().min.y;
  float terrainHeight = terr->getHeight(planePos.x,planePos.z);

  if(plane.isPlaneAlive() && planeBottom < terrainHeight)
  { //collision
    Vector3 viewVector = planeMatrix.objectToInertial(Vector3(0,0,1));
    if(viewVector * terr->getNormal(planePos.x,planePos.z) < -0.5f // dot product
      || plane.isCrashing())
    { 
      
      plane.killPlane();
      int partHndl = gParticle.createSystem("planeexplosion");
      gParticle.setSystemPos(partHndl, plane.getPosition());
      int boomHndl = gSoundManager.requestSoundHandle("Boom.wav");
      int boomInst = gSoundManager.requestInstance(boomHndl);
      if(boomInst != SoundManager::NOINSTANCE)
      {
        gSoundManager.setPosition(boomHndl,boomInst,plane.getPosition());
        gSoundManager.play(boomHndl,boomInst);
        gSoundManager.releaseInstance(boomHndl,boomInst);
      }
      plane.setSpeed(0.0f);
      planePos += 2.0f * viewVector;
      planeOrient.pitch = kPi / 4.0f;
      planeOrient.bank = kPi / 4.0f;
      plane.setOrientation(planeOrient);
      
      /*
      plane.jumpSpeedRatio = 0.0f;
      plane.m_fSpeed = 0.0f;
      enforcePosition(plane, terrain);
      */
    }
    else
    {
        planePos.y = terrainHeight + planePos.y - planeBottom;
        plane.setAir(FALSE);
    }
    plane.setPosition(planePos);
	return true;
  }
  if(onFurn==false)
      plane.setAir(TRUE);
  else
      plane.setAir(FALSE);
  return false;
}

bool Ned3DObjectManager::interactPlaneWater(PlaneObject &plane, WaterObject &water)
{
  Water *pWater = water.getWater();
  if(pWater == NULL) return false;
  
  // Test for plane collision with water
  
  Vector3 planePos = plane.getPosition();
  EulerAngles planeOrient = plane.getOrientation();
  Vector3 disp = planePos - disp;
  RotationMatrix planeMatrix;
  planeMatrix.setup(plane.getOrientation()); // get plane's orientation
  float planeBottom = plane.getBoundingBox().min.y;
  float waterHeight = pWater->getWaterHeight();
  
  if(plane.isPlaneAlive() && planeBottom < waterHeight)
  { //collision
    Vector3 viewVector = planeMatrix.objectToInertial(Vector3(0,0,1));
    plane.killPlane();
    plane.setSpeed(0.0f);
    planePos += 2.0f * viewVector;
    planeOrient.pitch = kPi / 4.0f;
    planeOrient.bank = kPi / 4.0f;
    plane.setOrientation(planeOrient);
    plane.setPosition(planePos);
    
    int partHndl = gParticle.createSystem("planeexplosion");
    gParticle.setSystemPos(partHndl, plane.getPosition());
    int boomHndl = gSoundManager.requestSoundHandle("Boom.wav");
    int boomInst = gSoundManager.requestInstance(boomHndl);
    if(boomInst != SoundManager::NOINSTANCE)
    {
      gSoundManager.setPosition(boomHndl,boomInst,plane.getPosition());
      gSoundManager.play(boomHndl,boomInst);
      gSoundManager.releaseInstance(boomHndl,boomInst);
    }
    return true;
  }
  return false;
}

bool Ned3DObjectManager::interactPlaneFurniture(PlaneObject &plane, GameObject &silo)
{
  const AABB3 &box1 = plane.getBoundingBox(), &box2 = silo.getBoundingBox();
  Vector3 pPos = plane.getPosition();

	if(silo.getName() == "AR_COUCH.s3d"){

		
		const AABB3 &box1 = plane.getBoundingBox();
		const AABB3 &box2 = silo.getBoundingBox();
		AABB3 intersectBox;

		Vector3 cPos = silo.getPosition();
		Vector3 sPos = silo.getPosition();
		sPos.y = sPos.y + 8;


		// if((planePos.x > savePos.x-20 && planePos.x < savePos.x+20) && (planePos.z > savePos.z-20 && planePos.z < savePos.z+20 ) )



		if(AABB3::intersect(box1,box2, &intersectBox)){
			if(!fromVictory)
				winTime = gRenderer.getTime();
			int partHndl = gParticle.createSystem("couch");
			gParticle.setSystemPos(partHndl, sPos);

		
		 char text[1024];      
		 sprintf_s(text,sizeof(text), "You Completed the Level!");
		// draw the text
		// int waittimer = gRenderer.getTime();
		 gRenderer.drawText(text, 30,30);
		 float time = gRenderer.getTime();


		 fromVictory = true;
		}
   }

  if(((pPos.x<box2.max.x)&&(pPos.x>box2.min.x))&&((pPos.z<box2.max.z)&&(pPos.z>box2.min.z)))
  {
      setShadHeight(box2.max.y);
  }

  AABB3 intersectBox;
  if(AABB3::intersect(box1, box2, &intersectBox))
  {

    if(plane.ledgeGrab==true)
    {
        return true;
    }

    // Collision:  Knock back obj1
    //   - Kludge method:  Push back on smallest dimension
    Vector3 delta = intersectBox.size();
    Vector3 obj2Pos = silo.getPosition();

    bool ledgeFlag = false;

    if(box1.max.y>box2.max.y)
    {
        if((delta.y<delta.x)&&(delta.y<delta.z))
        {
            onFurn = true;
            pPos.y = box2.max.y + pPos.y - box1.min.y;
            plane.setPosition(pPos);
            return true;
        }

        if((plane.inAir==true)&&(plane.m_ySpeed<1.0f))
        {
            if((plane.prevShiftHeld==false)&&(gRenderer.getTime()>(plane.ledgeTimer + 300)))
            {
                plane.ledgeGrab = true;
                ledgeFlag = true;
            }
            else
            {
                return true;
            }
        }
    }
    
    enforcePosition(plane, silo);
    if(plane.inAir==true)
    {
        if(plane.m_ySpeed<-0.8f)
        {
            plane.setySpeed(-0.8f);
        }
        
        if(delta.x <= delta.y)
            if(delta.x <= delta.z)
            {
                if(box1.min.x < box2.min.x)
                    plane.wallJumpDir = 1.5f;
                else
                    plane.wallJumpDir = 0.5f;
            }
            else
            {
                if(box1.min.z < box2.min.z)
                    plane.wallJumpDir = 1.0f;
                else
                    plane.wallJumpDir = 2.0f;
            }
            else if(delta.y <= delta.z)
            {
                //if(box1.min.y < box2.min.y)
                    //-delta.y;
                //else
                    //delta.y;
            }
            else
			{
                if(box1.min.z < box2.min.z)
                    plane.wallJumpDir = 1.0f;
                else
                    plane.wallJumpDir = 2.0f;
            }



        if((plane.jumpSpeedRatio>1.0f)&&(plane.ledgeGrab==false))
		{
            EulerAngles planeOrient = plane.getOrientation();
            if((plane.wallJumpDir!=0.0f)&&(plane.wallRun==0))
            {
                float pHead = plane.jumpHeading;
                if(pHead>(2.0f * kPi))
                    pHead = plane.simplifyAngle(pHead);
                if(pHead<0)
                    pHead = pHead + (kPi * 2.0f);
                float wMax = (plane.wallJumpDir - 0.5f) * kPi;
                float wMin = (plane.wallJumpDir - 1.5f) * kPi;
                if(wMax<=0.0f)
                    wMax = wMax + (kPi * 2.0f);
                if(wMin<=0.0f)
                    wMin = wMin + (kPi * 2.0f);
                if((pHead<=wMax)&&(pHead>(wMax-(0.5f*kPi))))
                {
                    plane.jumpHeading = (plane.wallJumpDir - 0.55f) * kPi;
                    planeOrient.heading = plane.jumpHeading;
                    plane.setOrientation(planeOrient);
                    plane.jumpSpeedRatio = 2.0f;
                    plane.wallRun = 2;
                }
                else if((pHead>=wMin)&&(pHead<(wMin+(0.5f*kPi))))
                {
                    plane.jumpHeading = (plane.wallJumpDir + 0.55f) * kPi;
                    planeOrient.heading = plane.jumpHeading;
                    plane.setOrientation(planeOrient);
                    plane.jumpSpeedRatio = 2.0f;
                    plane.wallRun = 1;
                }
            }
        }

		  int partHndl = gParticle.createSystem("walldust");
          gParticle.setSystemPos(partHndl, plane.getPosition());

    }

    if(ledgeFlag==true)
    {
        int dir = (int)(plane.wallJumpDir * 2.0f);
        switch(dir)
        {
        case 4:
            pPos.z = box2.max.z + (box1.size().z * 0.5f);
            plane.ledgeMax = box2.max.x - 3.0f;
            plane.ledgeMin = box2.min.x + 3.0f;
            break;
        case 3:
            pPos.x = box2.min.x - (box1.size().x * 0.5f);
            plane.ledgeMax = box2.max.z - 3.0f;
            plane.ledgeMin = box2.min.z + 3.0f;
            break;
        case 2:
            pPos.z = box2.min.z - (box1.size().z * 0.5f);
            plane.ledgeMax = box2.max.x - 3.0f;
            plane.ledgeMin = box2.min.x + 3.0f;
            break;
        case 1:
            pPos.x = box2.max.x + (box1.size().x * 0.5f);
            plane.ledgeMax = box2.max.z - 3.0f;
            plane.ledgeMin = box2.min.z + 3.0f;
            break;
        default:
            break;
        }
        pPos.y = box2.max.y - (box1.size().y * 0.5f);
        EulerAngles lOrient = EulerAngles::kEulerAnglesIdentity;
        lOrient.heading = (plane.wallJumpDir * kPi) + kPi;
        plane.setOrientation(lOrient);
        plane.setPosition(pPos);
        plane.ledgeTimer = gRenderer.getTime();
    }

    return true;
  }
  else
    return false;
}

bool Ned3DObjectManager::interactCrowBullet(CrowObject &crow, BulletObject &bullet)
{
  return bullet.checkForBoundingBoxCollision(&crow);
}

bool Ned3DObjectManager::interactCrowCrow(CrowObject &crow1, CrowObject &crow2)
{
  return enforcePositions(crow1, crow2);
}


bool Ned3DObjectManager::interactCrowTerrain(CrowObject &crow, TerrainObject &terrain)
{
  Terrain *terr = terrain.getTerrain();
  if(terr == NULL) return false;

  //test for crow collision with terrain
  Vector3 crowPos = crow.getPosition();
    
  float terrainHeight = terr->getHeight(crowPos.x,crowPos.z);
  if (crowPos.y < terrainHeight)
  {
    crowPos.y = terrainHeight;
    crow.setPosition(crowPos);       
    int tmpHndl = gParticle.createSystem("crowfeatherssplat");
    gParticle.setSystemPos(tmpHndl, crowPos);

    int thumpSound = gSoundManager.requestSoundHandle("Thump.wav");
    int instance = gSoundManager.requestInstance(thumpSound);
     if(thumpSound != SoundManager::NOINSTANCE)
  {
    gSoundManager.setPosition(thumpSound,instance,crowPos);
    gSoundManager.play(thumpSound,instance);
    gSoundManager.releaseInstance(thumpSound,instance);
  }
    
    crow.killObject();

    return true;
  }
  return false;
}

void Ned3DObjectManager::shootCrow(CrowObject &crow)
{
  int tmpHndl = gParticle.createSystem("crowfeathers");
  Vector3 crowPos = crow.getPosition();
  gParticle.setSystemPos(tmpHndl, crowPos);
  int deathSound = gSoundManager.requestSoundHandle("crowdeath.wav");
  int deathInstance = gSoundManager.requestInstance(deathSound);
  if(deathInstance != SoundManager::NOINSTANCE)
  {
    gSoundManager.setPosition(deathSound,deathInstance,crowPos);
    gSoundManager.play(deathSound,deathInstance);
    gSoundManager.releaseInstance(deathSound,deathInstance);
  }
  crow.setDying();
}


bool Ned3DObjectManager::interactSave(PlaneObject &plane, GameObject &save){

	
  Vector3 planePos = plane.getPosition();
  EulerAngles planeOrient = plane.getOrientation();
  Vector3 disp = planePos - disp;
  RotationMatrix planeMatrix;
  planeMatrix.setup(plane.getOrientation()); // get plane's orientation
  Vector3 saveLoc;
  saveLoc = save.getPosition();



	const AABB3 &box1 = plane.getBoundingBox();
	const AABB3 &box2 = save.getBoundingBox();
	AABB3 intersectBox;

	if(AABB3::intersect(box1,box2, &intersectBox)){
		 int partHndl = gParticle.createSystem("saveglow1");
         int partHnd2 = gParticle.createSystem("saveglow2");
        gParticle.setSystemPos(partHndl, saveLoc);
        gParticle.setSystemPos(partHnd2, saveLoc);
	  //int textY = gRenderer.getScreenY()/2;
	  //IRectangle rect = IRectangle(0,textY,gRenderer.getScreenX()-1, textY + 30);
   	  //gRenderer.drawText("Saving...",&rect, eTextAlignModeCenter, false);

		 char text[1024];      
		 sprintf_s(text,sizeof(text), "Saving..");
		 // draw the text
		 gRenderer.drawText(text, 30,30);

		 setSavePoint(planePos.x, planePos.y+10, planePos.z);
	}

///
 // Vector3 savePos = save.getPosition();
//  save.setPosition(savePos); 
	//10.0f,5.0f,0.0f



  // if((planePos.x > savePos.x-20 && planePos.x < savePos.x+20) && (planePos.z > savePos.z-20 && planePos.z < savePos.z+20 ) )
  //{
  //int partHndl = gParticle.createSystem("saveglow1");
  //int partHnd2 = gParticle.createSystem("saveglow2");
  //  gParticle.setSystemPos(partHndl, savePos);
  //  gParticle.setSystemPos(partHnd2, savePos);
	 // int textY = gRenderer.getScreenY()/2;
  //    IRectangle rect = IRectangle(0,textY,gRenderer.getScreenX()-1, textY + 30);
  //    gRenderer.drawText("Saving...",&rect, eTextAlignModeCenter, false);
  //}

  //if((planePos.x > savePos.x-2 && planePos.x < savePos.x+2) && (planePos.z > savePos.z-2 && planePos.z < savePos.z+2 ) )
  //{
  //  int partHnd3 = gParticle.createSystem("saveglow3");
  //  gParticle.setSystemPos(partHnd3, savePos);
  //}

 /*
 savePos.x = 10.0f;
  savePos.y = 10.0f;
  savePos.z = 0.0;
  int tmpHnd1 = gParticle.createSystem("smokeheavy");
  float planeTop = plane.getBoundingBox().max.y;
  float planeSide = plane.getBoundingBox().min.x;
  if(planeTop < 10.0f && (planeSide > 8 || planeSide < 11))
  gParticle.setSystemPos(tmpHnd1, planePos);
  */
return true;
}

bool Ned3DObjectManager::interactJump(PlaneObject &plane, TerrainObject &terrain){
  Terrain *terr = terrain.getTerrain();
  if(terr == NULL) return false; 

  Vector3 planePos = plane.getPosition();
  EulerAngles planeOrient = plane.getOrientation();
  Vector3 disp = planePos - disp;
  RotationMatrix planeMatrix;
  planeMatrix.setup(plane.getOrientation()); // get plane's orientation
  float planeBottom = plane.getBoundingBox().min.y;
  float terrainHeight = terr->getHeight(planePos.x,planePos.z);
  
  if(plane.isPlaneAlive() && planeBottom > terrainHeight)
  {     
	Vector3 jetpackLPos = plane.getPosition();
	jetpackLPos.x = jetpackLPos.x - 1;

	Vector3 jetpackRPos = plane.getPosition();
	jetpackRPos.x = jetpackRPos.x + 1;
//	jetpackRPos.z = jetpackRPos.z - 1;

    int partHndl = gParticle.createSystem("jumpdust");
	int partHnd2 = gParticle.createSystem("jumpdust");
    gParticle.setSystemPos(partHndl, jetpackLPos);
	gParticle.setSystemPos(partHnd2, jetpackRPos);
    /*
    int boomHndl = gSoundManager.requestSoundHandle("Boom.wav");
    int boomInst = gSoundManager.requestInstance(boomHndl);
    if(boomInst != SoundManager::NOINSTANCE)
    {
      gSoundManager.setPosition(boomHndl,boomInst,plane.getPosition());
      gSoundManager.play(boomHndl,boomInst);
      gSoundManager.releaseInstance(boomHndl,boomInst);
    }
    */
    return true;
  }
  return false;
}

bool Ned3DObjectManager::enforcePosition(GameObject &moving, GameObject &stationary)
{
  const AABB3 &box1 = moving.getBoundingBox(), &box2 = stationary.getBoundingBox();
  AABB3 intersectBox;
  if(AABB3::intersect(box1, box2, &intersectBox))
  {
    // Collision:  Knock back obj1
    //   - Kludge method:  Push back on smallest dimension
    Vector3 delta = intersectBox.size();
    Vector3 obj1Pos = moving.getPosition(), obj2Pos = stationary.getPosition();
    if(delta.x <= delta.y)
      if(delta.x <= delta.z)
      {
        // Push back on x
        obj1Pos.x += (box1.min.x < box2.min.x) ? -delta.x : delta.x;
      }
      else
      {
        // Push back on z
        obj1Pos.z += (box1.min.z < box2.min.z) ? -delta.z : delta.z;
      }
    else if(delta.y <= delta.z)
    {
      // Push back on y
      obj1Pos.y += (box1.min.y < box2.min.y) ? -delta.y : delta.y;
    }
    else
    {
      // Push back on z
      obj1Pos.z += (box1.min.z < box2.min.z) ? -delta.z : delta.z;
    }
    moving.setPosition(obj1Pos);
    return true;
  }
  return false;
}

bool Ned3DObjectManager::enforcePositions(GameObject &obj1, GameObject &obj2)
{
  const AABB3 &box1 = obj1.getBoundingBox(), &box2 = obj2.getBoundingBox();
  AABB3 intersectBox;
  if(AABB3::intersect(box1, box2, &intersectBox))
  {
    // Collision:  Knock back both objects
    //   - Kludge method:  Push back on smallest dimension
    Vector3 delta = intersectBox.size();
    Vector3 obj1Pos = obj1.getPosition(), obj2Pos = obj2.getPosition();
    if(delta.x <= delta.y)
      if(delta.x <= delta.z)
      {
        // Push back on x
        float dx = (box1.min.x < box2.min.x) ? -delta.x : delta.x;
        obj1Pos.x += dx;
        obj2Pos.x -= dx;
      }
      else
      {
        // Push back on z
        float dz = (box1.min.z < box2.min.z) ? -delta.z : delta.z;
        obj1Pos.z += dz;
        obj2Pos.z -= dz;
      }
    else if(delta.y <= delta.z)
    {
        // Push back on y
        float dy = (box1.min.y < box2.min.y) ? -delta.y : delta.y;
        obj1Pos.y += dy;
        obj2Pos.y -= dy;
    }
    else
    {
      // Push back on z
      float dz = (box1.min.z < box2.min.z) ? -delta.z : delta.z;
      obj1Pos.z += dz;
      obj2Pos.z -= dz;
    }
    obj1.setPosition(obj1Pos);
    obj2.setPosition(obj2Pos);
    return true;
  }
  return false;
}


void Ned3DObjectManager::setShadHeight(float height)
{
    if(height>shadHeight)
    {
        shadHeight = height;
    }
}


float Ned3DObjectManager::getShadHeight()
{
        return shadHeight;
}
