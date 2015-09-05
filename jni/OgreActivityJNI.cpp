#include "shadergeneratortechniqueresolverlistener.h"

static Ogre::RTShader::ShaderGenerator * mShaderGenerator = nullptr;         // The Shader generator instance.
static ShaderGeneratorTechniqueResolverListener * gMatListener = nullptr;      // Shader generator material manager listener.

using namespace Ogre;

static bool gInit = false;
static Ogre::Root * gRoot = nullptr;
static Ogre::RenderWindow * gRenderWnd = nullptr;

#ifdef OGRE_BUILD_PLUGIN_OCTREE
static Ogre::OctreePlugin* gOctreePlugin = nullptr;
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
static Ogre::ParticleFXPlugin* gParticleFXPlugin = nullptr;
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
static Ogre::OverlaySystem* gOverlaySystem = nullptr;
#endif

static Ogre::GLESRS * gGLESPlugin = nullptr;

static Ogre::SceneManager * pSceneMgr = nullptr;
static Ogre::Camera * pCamera = nullptr;
static AAssetManager * gAssetMgr = nullptr;
static JavaVM * gVM = nullptr;

static float initZposition = 200.0f;
static float initXposition = 0.0f;

static float camYposition = 0.0f;
static float camXposition = 0.0f;
static float camZposition = 0.0f;

static float camAngleHor = -1.483f;
static float camAngleVer = 1.919f; //60 degrees as initial value

static int vesselAngle = 0;

/** Player's vessel node */
DutchFrigate * playersShip;

/** Particle system for cannon shot */
//ParticleSystem * blowParticle;

#define CAMERA_Y_POSITION 80
#define VESSEL_Y_POSITION -25
#define VESSEL_Z_DISTATION 210

#include <android/log.h>

static const float ELLIPSE_AB = 2.5f;
static const float angle_coefficient = 1.8f;
#define PI 3.14159265

/** Physics elementary provided by bullet library */

static Ogre::Entity * cannonEnt;
btDiscreteDynamicsWorld * world;
std::vector<btRigidBody *> objects;
int shipWidth = 0;
std::vector<Ogre::SceneNode *> leftCannonBallNodes;
std::vector<Ogre::SceneNode *> rightCannonBallNodes;

void createCannon(Ogre::SceneNode * tmpNode, const btVector3 & position, btScalar Mass, const btVector3 & initFire,
                  const btVector3 & endFire,
                  float factorForce)
{
    Ogre::Vector3 size = Ogre::Vector3::ZERO;
    Ogre::Vector3 pos = Ogre::Vector3::ZERO;

    pos.x = position.getX();
    pos.y = position.getY();
    pos.z = position.getZ();

    Ogre::Entity * cannonEnt = static_cast<Ogre::Entity *>(tmpNode->getAttachedObject(0));
    Ogre::AxisAlignedBox boundingBox = cannonEnt->getBoundingBox();
    size = boundingBox.getSize() * 0.95f;
    tmpNode->setPosition(pos);

    /** Physics */
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);
    btDefaultMotionState * motionState = new btDefaultMotionState(transform);
    btVector3 halfExtents(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f);
    btCollisionShape * shape = new btBoxShape(halfExtents);
    btVector3 localInertia;
    shape->calculateLocalInertia(Mass, localInertia);
    btRigidBody * rigitBody = new btRigidBody(Mass, motionState, shape, localInertia);
    rigitBody->applyForce(initFire * factorForce, endFire);
    rigitBody->setUserPointer((void *) tmpNode);
    world->addRigidBody(rigitBody);
    objects.push_back(rigitBody);
}

void clearObjects()
{
    objects.clear();
}

void updatePhysics(unsigned int deltaTime)
{
    world->stepSimulation(deltaTime * 0.1f, 60);
    btRigidBody *tObject;
    for(std::vector<btRigidBody *>::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        if((*it)->getUserPointer() != nullptr)
        {
            Ogre::SceneNode * tmpNode = static_cast<Ogre::SceneNode *>((*it)->getUserPointer());
            tObject = *it;

            btVector3 point = tObject->getCenterOfMassPosition();
            tmpNode->setPosition(Ogre::Vector3((float)point[0], (float)point[1], (float)point[2]));
            btQuaternion btq = tObject->getOrientation();
            Ogre::Quaternion quart = Ogre::Quaternion(btq.w(), btq.x(), btq.y(), btq.z());
            tmpNode->setOrientation(quart);
        }
    }
}
//Physics_END

void * fireLeft(void *)
{
    std::vector<Ogre::Entity *> c_balls = playersShip->getCannonBalls();
    Ogre::Vector3 orientation = playersShip->getOrientation();
    orientation.normalise();
    int degree = -55;
    Ogre::Vector2 tmp(orientation.z * cos(degree * PI / 180) - orientation.x * sin(degree * PI / 180),
                      orientation.z * sin(degree * PI / 180) + orientation.x * cos(degree * PI / 180));

    /*std::string msg = "!!!APP \t x : " + to_string(orientation.x);
    __android_log_print(ANDROID_LOG_VERBOSE, msg.c_str(), "");
    msg = "!!!APP \t y : " + to_string(orientation.y);
    __android_log_print(ANDROID_LOG_VERBOSE, msg.c_str(), "");
    msg = "!!!APP \t z : " + to_string(orientation.z);
    __android_log_print(ANDROID_LOG_VERBOSE, msg.c_str(), "");*/
    for(int i = 0; i < 18; ++i)
    {
        Ogre::Vector3 center = c_balls.at(i)->getWorldBoundingBox().getCenter();
        createCannon(leftCannonBallNodes.at(i), btVector3(center.x, center.y, center.z),
                     10, btVector3(tmp.x, -orientation.y, -tmp.y),
                     btVector3(tmp.x, 0, -tmp.y), 10000.0f);
    }

    playersShip->setEmittingLeft(true);
    usleep(300000);
    playersShip->setEmittingLeft(false);
}

void * fireRight(void *)
{
    std::vector<Ogre::Entity *> c_balls = playersShip->getCannonBalls();
    Ogre::Vector3 orientation = playersShip->getOrientation();
    orientation.normalise();
    int degree = -55;
    Ogre::Vector2 tmp(orientation.z * cos(degree * PI / 180) - orientation.x * sin(degree * PI / 180),
                      orientation.z * sin(degree * PI / 180) + orientation.x * cos(degree * PI / 180));
    for(int i = 0; i < 18; ++i)
    {
        Ogre::Vector3 center = c_balls.at(i + 21)->getWorldBoundingBox().getCenter();
        createCannon(rightCannonBallNodes.at(i), btVector3(center.x, center.y, center.z),
                     10, btVector3(-tmp.x, -orientation.y, tmp.y),
                     btVector3(-tmp.x, 0, tmp.y), 10000.0f);
    }

    playersShip->setEmittingRight(true);
    usleep(300000);
    playersShip->setEmittingRight(false);
}

static Ogre::DataStreamPtr openAPKFile(const Ogre::String& fileName)
{
    Ogre::DataStreamPtr stream;
    AAsset* asset = AAssetManager_open(gAssetMgr, fileName.c_str(), AASSET_MODE_BUFFER);
    if(asset)
    {
        off_t length = AAsset_getLength(asset);
        void* membuf = OGRE_MALLOC(length, Ogre::MEMCATEGORY_GENERAL);
        memcpy(membuf, AAsset_getBuffer(asset), length);
        AAsset_close(asset);

        stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(membuf, length, true, true));
    }
    return stream;
}

extern "C" 
{
    JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        gVM = vm;
        return JNI_VERSION_1_4;
    }

    JNIEXPORT void JNICALL  Java_org_ogre3d_android_OgreActivityJNI_create(JNIEnv * env, jobject obj, jobject assetManager)
    {
        if(gInit)
            return;

        gRoot = new Ogre::Root();

        gGLESPlugin = OGRE_NEW GLESRS();
        gRoot->installPlugin(gGLESPlugin);

#ifdef OGRE_BUILD_PLUGIN_OCTREE
        gOctreePlugin = OGRE_NEW OctreePlugin();
        gRoot->installPlugin(gOctreePlugin);
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
        gParticleFXPlugin = OGRE_NEW ParticleFXPlugin();
        gRoot->installPlugin(gParticleFXPlugin);
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
        gOverlaySystem = OGRE_NEW OverlaySystem();
#endif

        gRoot->setRenderSystem(gRoot->getAvailableRenderers().at(0));
        gRoot->initialise(false);
        gInit = true;

        gAssetMgr = AAssetManager_fromJava(env, assetManager);
        if (gAssetMgr)
        {
            ArchiveManager::getSingleton().addArchiveFactory( new APKFileSystemArchiveFactory(gAssetMgr) );
            ArchiveManager::getSingleton().addArchiveFactory( new APKZipArchiveFactory(gAssetMgr) );
        }
    }
	
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_destroy(JNIEnv * env, jobject obj)
    {
        if(!gInit)
            return;

        gInit = false;

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
        OGRE_DELETE gOverlaySystem;
        gOverlaySystem = nullptr;
#endif

        OGRE_DELETE gRoot;
        gRoot = nullptr;
        gRenderWnd = nullptr;

#ifdef OGRE_BUILD_PLUGIN_PFX
        OGRE_DELETE gParticleFXPlugin;
        gParticleFXPlugin = nullptr;
#endif

#ifdef OGRE_BUILD_PLUGIN_OCTREE
        OGRE_DELETE gOctreePlugin;
        gOctreePlugin = nullptr;
#endif

        OGRE_DELETE gGLESPlugin;
        gGLESPlugin = nullptr;
        clearObjects();

        //OGRE_DELETE playersShip;
        //playersShip = nullptr;
    }
	

    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_initWindow(JNIEnv * env, jobject obj,  jobject surface)
    {
        if(surface)
        {
            ANativeWindow * nativeWnd = ANativeWindow_fromSurface(env, surface);
            if (nativeWnd && gRoot)
            {
                if (!gRenderWnd)
                {
                    Ogre::NameValuePairList opt;
                    opt["externalWindowHandle"] = Ogre::StringConverter::toString((int)nativeWnd);
                    gRenderWnd = Ogre::Root::getSingleton().createRenderWindow("OgreWindow", 0, 0, false, &opt);


                    if(pSceneMgr == nullptr)
                    {
                        Ogre::ConfigFile cf;
                        cf.load(openAPKFile("resources.cfg"));

                        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
                        Ogre::String sec, type, arch;
                        while (seci.hasMoreElements())
                        {
                            sec = seci.peekNextKey();
                            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
                            Ogre::ConfigFile::SettingsMultiMap::iterator i;

                            // go through all resource paths
                            for (i = settings->begin(); i != settings->end(); i++)
                            {
                                type = i->first;
                                arch = i->second;
                                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
                            }
                        }

                        camXposition = sin(camAngleVer) * cos(camAngleHor) + initXposition;
                        camZposition = sin(camAngleVer) * sin(camAngleHor) + initZposition;
                        camYposition = cos(camAngleVer) + CAMERA_Y_POSITION;

                        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

                        Ogre::RTShader::ShaderGenerator::initialize();
                        Ogre::RTShader::ShaderGenerator::getSingletonPtr()->setTargetLanguage("glsles");
                        gMatListener = new ShaderGeneratorTechniqueResolverListener();
                        Ogre::MaterialManager::getSingleton().addListener(gMatListener);

                        pSceneMgr = gRoot->createSceneManager(Ogre::ST_GENERIC);
                        Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(pSceneMgr);

                        pCamera = pSceneMgr->createCamera("MyCam");
                        pCamera->setNearClipDistance(1.0f);
                        pCamera->setPosition(initXposition, CAMERA_Y_POSITION, initZposition);
                        pCamera->lookAt(camXposition, camYposition, camZposition);
                        pCamera->setAutoAspectRatio(true);

                        Ogre::Viewport * vp = gRenderWnd->addViewport(pCamera);
                        vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
                        vp->setMaterialScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

                        pSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.5));
                        pSceneMgr->setSkyBox(true, "SkyBox", 1000);


                        Ogre::Plane oceanSurface;
                        oceanSurface.normal = Ogre::Vector3::UNIT_Y;
                        oceanSurface.d = 20;
                        Ogre::MeshManager::getSingleton().createPlane("OceanSurface",
                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                            oceanSurface,
                            10000, 10000, 50, 50, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
                        Ogre::Entity * mOgreSurfaceEntity = pSceneMgr->createEntity("OceanSurface", "OceanSurface");
                        Ogre::SceneNode * waterNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();
                        waterNode->attachObject(mOgreSurfaceEntity);
                        mOgreSurfaceEntity->setMaterialName("Ocean2_HLSL_GLSL");

                        /** init physics */
                        btBroadphaseInterface * broadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000),
                                                                              btVector3(1000, 1000, 1000));
                        btDefaultCollisionConfiguration * collisionConfiguration = new btDefaultCollisionConfiguration;
                        btCollisionDispatcher * dispatcher = new btCollisionDispatcher(collisionConfiguration);
                        btSequentialImpulseConstraintSolver * solver = new btSequentialImpulseConstraintSolver;
                        world = new btDiscreteDynamicsWorld(dispatcher, broadPhase, solver, collisionConfiguration);

                        /** physics of the plane */
                        btTransform transform;
                        transform.setIdentity();
                        transform.setOrigin(btVector3(0, 0, 0));
                        btDefaultMotionState *motionState = new btDefaultMotionState(transform);
                        btCollisionShape * shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
                        btVector3 localInertia;
                        shape->calculateLocalInertia(0, localInertia);
                        btRigidBody * rigidBody = new btRigidBody(0, motionState, shape, localInertia);
                        rigidBody->setUserPointer((void *) waterNode);
                        world->addRigidBody(rigidBody);
                        objects.push_back(rigidBody);
                        //end of physics

                        for(int i = 0; i < 18; ++i)
                        {
                            leftCannonBallNodes.push_back(pSceneMgr->getRootSceneNode()->createChildSceneNode());
                            Ogre::Entity * cannonEnt = pSceneMgr->createEntity("LeftBall_" + to_string(i), "Sphere.mesh");
                            leftCannonBallNodes.at(i)->attachObject(cannonEnt);

                            rightCannonBallNodes.push_back(pSceneMgr->getRootSceneNode()->createChildSceneNode());
                            cannonEnt = pSceneMgr->createEntity("RightBall_" + to_string(i), "Sphere.mesh");
                            rightCannonBallNodes.at(i)->attachObject(cannonEnt);
                        }

                        playersShip = new DutchFrigate(pSceneMgr,
                                                       Ogre::Vector3(initXposition, VESSEL_Y_POSITION, initZposition - VESSEL_Z_DISTATION),
                                                       21, 18);

                        playersShip->setEmitting(false);


                        Ogre::RTShader::ShaderGenerator::getSingletonPtr()->invalidateScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
                    }
                }
                else
                {
                    static_cast<Ogre::AndroidEGLWindow*>(gRenderWnd)->_createInternalResources(nativeWnd, nullptr);
                }
            }
        }
    }
	
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_termWindow(JNIEnv * env, jobject obj)
    {
        if(gRoot && gRenderWnd)
        {
            static_cast<Ogre::AndroidEGLWindow*>(gRenderWnd)->_destroyInternalResources();
        }
    }

    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_shootLeftDeck(JNIEnv * env, jobject obj)
    {
        pthread_t fireThread;
        pthread_create(&fireThread, nullptr, &fireLeft, nullptr);
        pthread_detach(fireThread);
    }

    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_shootRightDeck(JNIEnv * env, jobject obj)
    {
        pthread_t fireThread;
        pthread_create(&fireThread, nullptr, &fireRight, nullptr);
        pthread_detach(fireThread);
    }
	
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj,
                                                                                  jint angle, jfloat angleHor, jfloat angleVer)
    {
        if(gRenderWnd != nullptr && gRenderWnd->isActive())
        {
            try
            {
                if(gVM->AttachCurrentThread(&env, nullptr) < 0)
                    return;

                gRenderWnd->windowMovedOrResized();
                gRoot->renderOneFrame();

                float angle_ = (float) angle;

                int turningAngle = 0;
                if((angle_ <= 90 && angle_ > 30) || (angle_ >= -90 && angle_ < -30))
                {
                    float ellipse_coefficientA = 0.0f;
                    ellipse_coefficientA = angle_ / 90;

                    float ellipse_coefficientB = 1 - abs(ellipse_coefficientA);
                    ellipse_coefficientA *= 2;

                    float A = ELLIPSE_AB * ellipse_coefficientA;
                    float B = ELLIPSE_AB * ellipse_coefficientB;

                    int tmp = vesselAngle - angle_;
                    if(tmp < 0)
                    {
                        vesselAngle += 1;
                        turningAngle = -1;
                    }
                    else if(tmp > 0)
                    {
                        vesselAngle -= 1;
                        turningAngle = 1;
                    }

                    if(angle_ > 0)
                    {
                        initZposition -= A * cos(angle_ * PI / 180) / 2;
                        initXposition += B * sin(angle_ * PI / 180) / 2;
                    }
                    else
                    {
                        initZposition += A * cos(angle_ * PI / 180) / 2;
                        initXposition += B * sin(angle_ * PI / 180) / 2;
                    }
                }
                else if((angle_ <= 30 && angle_ > 0) || (angle_ >= -30 && angle_ < 0))
                {
                    float ellipse_coefficientB = 0.0f;
                    ellipse_coefficientB = angle_ / 30;

                    float ellipse_coefficientA = 1 - abs(ellipse_coefficientB);
                    ellipse_coefficientB *= 2;

                    float A = ELLIPSE_AB * ellipse_coefficientA;
                    float B = ELLIPSE_AB * ellipse_coefficientB;

                    int tmp = vesselAngle - angle_;
                    if(tmp < 0)
                    {
                        vesselAngle += 1;
                        turningAngle = -1;
                    }
                    else if(tmp > 0)
                    {
                        vesselAngle -= 1;
                        turningAngle = 1;
                    }

                    if(angle_ >= 0)
                    {
                        initZposition -= A * cos(angle_ * PI / 180) / 2;
                        initXposition += B * sin(angle_ * PI / 180) / 2;
                    }
                    else
                    {
                        initZposition -= A * cos(angle_ * PI / 180) / 2;
                        initXposition -= B * sin(angle_ * PI / 180) / 2;
                    }
                }

                camAngleHor += (float)angleHor;
                camAngleVer += (float)angleVer;

                camXposition = sin(camAngleVer) * cos(camAngleHor) + initXposition;
                camZposition = sin(camAngleVer) * sin(camAngleHor) + initZposition;
                camYposition = cos(camAngleVer) + CAMERA_Y_POSITION;

                pCamera->setPosition(initXposition, CAMERA_Y_POSITION, initZposition);
                pCamera->lookAt(camXposition, camYposition, camZposition);

                if(angle_ != 0)
                {
                    playersShip->setTurningAngle(turningAngle);
                }
                updatePhysics(1);
                playersShip->setCurrentPosition(Ogre::Vector3(initXposition, VESSEL_Y_POSITION, initZposition - VESSEL_Z_DISTATION));
                //gVM->DetachCurrentThread();
            }
            catch(Ogre::RenderingAPIException ex) {}
        }
    }
}
