#ifndef DUTCHFRIGATE_H
#define DUTCHFRIGATE_H

#include "ship.h"

class DutchFrigate : public Ship
{
private:
    Ogre::Entity * dutchFrigate;
    std::vector<Ogre::ParticleSystem *> cannonFire;
public:
    DutchFrigate(Ogre::SceneManager * pSceneMgr, Ogre::Vector3 initialPosition, int leftCannons, int rightCannons);
    void setEmitting(bool);
    void setCurrentPosition(Ogre::Vector3);
};

#endif // DUTCHFRIGATE_H
