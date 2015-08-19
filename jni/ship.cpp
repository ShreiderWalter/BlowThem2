#include "ship.h"

Ship::Ship(Ogre::SceneManager * pSceneMgr, float x, float y, float z,
           int leftCannons, int rightCannons) : currentPosition(x, y, z)
{
    shipNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();
    shipNode->setScale(0.05, 0.05, 0.05);
    shipNode->setPosition(x, y, z);
    shipNode->pitch(Ogre::Degree(90));
    shipNode->roll(Ogre::Degree(165));

    int res = leftCannons + rightCannons;
    for(size_t i = 0; i < res; ++i)
    {
        cannonNodes.push_back(shipNode->createChildSceneNode());
        //cannonNodes.at(i)->setPosition(100, 100, 100);
        cannonNodes.at(i)->pitch(Ogre::Degree(90));
        cannonNodes.at(i)->roll(Ogre::Degree(165));

        cannonEntities.push_back(pSceneMgr->createEntity("Cannon" + to_string(i + 1), "Cannon" + to_string(i + 1)
                                                         + ".mesh"));
        cannonNodes.at(i)->attachObject(cannonEntities.at(i));
        cannonNodes.at(i)->setScale(0.05, 0.05, 0.05);
    }
}

