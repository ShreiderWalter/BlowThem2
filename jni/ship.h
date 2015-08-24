#ifndef SHIP_H
#define SHIP_H

#include <iostream>
#include <vector>
#include "functions.h"

#include <OgreAny.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include "OgreParticleSystem.h"


class Ship
{
protected:
    bool isEmmiting;
    std::vector<Ogre::SceneNode *> cannonNodes;
    std::vector<Ogre::Entity *> cannonEntities;

    Ogre::Vector3 currentPosition;
    Ogre::SceneNode * shipNode;
public:
    Ship(Ogre::SceneManager * pSceneMgr, float x, float y, float z, int leftCannons, int rightCannons);

    virtual void setEmitting(bool) = 0;
    virtual void setEmittingLeft(bool) = 0;
    virtual void setEmittingRight(bool) = 0;
    virtual void setCurrentPosition(Ogre::Vector3) = 0;
    virtual void setTurningAngle(int degree) = 0;
};

#endif // SHIP_H
