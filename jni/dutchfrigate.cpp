#include "dutchfrigate.h"

#include <android/log.h>

DutchFrigate::DutchFrigate(Ogre::SceneManager * pSceneMgr, Ogre::Vector3 initialPosition,
                           int leftCannons, int rightCannons) : Ship(pSceneMgr, initialPosition.x,
                                                                     initialPosition.y, initialPosition.z,
                                                                     leftCannons, rightCannons)
{
    dutchFrigate = pSceneMgr->createEntity("DutchFrigate", "Ship.mesh");
    shipNode->attachObject(dutchFrigate);


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
        __android_log_print(ANDROID_LOG_VERBOSE, "BLOW_THEM", "RIGHT FIRE ADDED", 1);

    }
}

void DutchFrigate::setEmitting(bool flag)
{
    for(auto i : cannonFire)
    {
        i->setEmitting(flag);
    }
}

void DutchFrigate::setCurrentPosition(Ogre::Vector3 currentPosition)
{
    shipNode->setPosition(currentPosition);
}

