#include "dutchfrigate.h"

#include <android/log.h>

DutchFrigate::DutchFrigate(Ogre::SceneManager * pSceneMgr, Ogre::Vector3 initialPosition,
                           int leftCannons, int rightCannons) : Ship(pSceneMgr, initialPosition.x,
                                                                     initialPosition.y, initialPosition.z,
                                                                     leftCannons, rightCannons)
{
    dutchFrigate = pSceneMgr->createEntity("DutchFrigate", "Ship.mesh");
    shipNode->attachObject(dutchFrigate);
    currentTurn = 0;
    prevousTurn = 0;

    for(size_t i = 0; i < 18; ++i)
    {
        cannonFire.push_back(pSceneMgr->createParticleSystem("CannonPXL" + to_string(i + 1), "Examples/FireLeft"
                                                             + to_string(i + 1)));
        shipNode->attachObject(cannonFire.at(i));
    }

    for(size_t i = 0; i < 18; ++i)
    {
        cannonFire.push_back(pSceneMgr->createParticleSystem("CannonPXR" + to_string(i + 1), "Examples/FireRight"
                                                             + to_string(i + 1)));
        shipNode->attachObject(cannonFire.at(i + 18));
    }
}

void DutchFrigate::setEmitting(bool flag)
{
    for(auto i : cannonFire)
    {
        i->setEmitting(flag);
    }
}

void DutchFrigate::setEmittingLeft(bool flag)
{
    for(int i = 0; i < 18; ++i)
    {
        cannonFire.at(i)->setEmitting(flag);
    }
}

void DutchFrigate::setEmittingRight(bool flag)
{
    for(int i = 19; i < 36; ++i)
    {
        cannonFire.at(i)->setEmitting(flag);
    }
}

void DutchFrigate::setCurrentPosition(Ogre::Vector3 currentPosition)
{
    shipNode->setPosition(currentPosition);
}

void DutchFrigate::setTurningAngle(int degree)
{
    shipNode->roll(Ogre::Degree(currentTurn - degree));
}

