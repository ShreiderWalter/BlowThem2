#include "dutchfrigate.h"

#include <android/log.h>

DutchFrigate::DutchFrigate(Ogre::SceneManager * pSceneMgr, Ogre::Vector3 initialPosition,
                           int leftCannons, int rightCannons) : Ship(pSceneMgr, initialPosition.x,
                                                                     initialPosition.y, initialPosition.z,
                                                                     leftCannons, rightCannons)
{
    dutchFrigate = pSceneMgr->createEntity("DutchFrigate", "Ship.mesh");
    shipNode->attachObject(dutchFrigate);

    std::string tmp = "!!!!!!!!!!!!" + to_string(initialPosition.x) +
            " " + to_string(initialPosition.y);
    __android_log_print(ANDROID_LOG_VERBOSE, "BLOW_THEM", tmp.c_str(), 1);

    for(size_t i = 0; i < 18; ++i)
    {
        cannonFire.push_back(pSceneMgr->createParticleSystem("CannonPX" + to_string(i + 1), "Examples/FireLeft"
                                                             + to_string(i + 1)));
        //cannonNodes.at(i)->attachObject(cannonFire.at(i));
        shipNode->attachObject(cannonFire.at(i));
    }

    /*int totalCannons = 36;
    for(size_t i = 19; i < totalCannons - 1; ++i)
    {
        cannonFire.push_back(pSceneMgr->createParticleSystem("Cannon" + to_string(i + 1), "Examples/FireRight"
                                                             + to_string(i - 18)));
        cannonNodes.at(i)->attachObject(cannonFire.at(i));
    }*/
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

