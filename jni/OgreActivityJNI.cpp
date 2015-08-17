#include "shadergeneratortechniqueresolverlistener.h"
#include <string>
#include <sstream>

static Ogre::RTShader::ShaderGenerator * mShaderGenerator = nullptr;         // The Shader generator instance.
static ShaderGeneratorTechniqueResolverListener * gMatListener = nullptr;      // Shader generator material manager listener.

using namespace Ogre;

static bool gInit = false;
static Ogre::Root* gRoot = nullptr;
static Ogre::RenderWindow* gRenderWnd = nullptr;

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

static Ogre::SceneManager* pSceneMgr = nullptr;
static Ogre::Camera* pCamera = nullptr;
static AAssetManager* gAssetMgr = nullptr;
static JavaVM* gVM = nullptr;

static float initZposition = 500.0f;
static float initXposition = 0.0f;

static float camYposition = 0.0f;
static float camXposition = 0.0f;
static float camZposition = 0.0f;

static float camAngleHor = -1.483f;
static float camAngleVer = 1.919f; //60 degrees as initial value

/** Player's vessel node */
Ogre::SceneNode * shipNode;
Ogre::SceneNode * wakeNode;

/** Particle system for cannon shot */
ParticleSystem * blowParticle;

#define CAMERA_Y_POSITION 80
#define VESSEL_Y_POSITION -25
#define VESSEL_Z_DISTATION 210

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
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

                        wakeNode = waterNode->createChildSceneNode("WakeParticle");

                        Ogre::Entity * mSoleiRoyal = pSceneMgr->createEntity("SoleiRoyal", "Ship.mesh");
                        shipNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();
                        shipNode->setScale(0.05, 0.05, 0.05);
                        shipNode->setPosition(camXposition, VESSEL_Y_POSITION, camYposition - VESSEL_Z_DISTATION);
                        shipNode->pitch(Ogre::Degree(90));
                        shipNode->roll(Ogre::Degree(165));
                        shipNode->attachObject(mSoleiRoyal);



                        ParticleSystem * wakeParticle = pSceneMgr->createParticleSystem("Wake", "Water/Wake");
                        wakeNode->attachObject(wakeParticle);


                        /** Checking out fire system */
                        /*Ogre::SceneNode * fire1Node = pSceneMgr->getRootSceneNode()->createChildSceneNode();
                        fire1Node->setPosition(Ogre::Vector3(100, 100, 100));
                        Ogre::SceneNode * fire2Node = pSceneMgr->getRootSceneNode()->createChildSceneNode();
                        fire2Node->setPosition(Ogre::Vector3(100, 100, 100));
                        Ogre::SceneNode * fire3Node = pSceneMgr->getRootSceneNode()->createChildSceneNode();
                        fire3Node->setPosition(Ogre::Vector3(100, 100, 100));


                        Ogre::Entity * cannon1Entity = pSceneMgr->createEntity("FirstCannon", "Cannon1.mesh");
                        Ogre::Entity * cannon2Entity = pSceneMgr->createEntity("SecondCannon", "Cannon2.mesh");
                        Ogre::Entity * cannon3Entity = pSceneMgr->createEntity("ThirdCannon", "Cannon3.mesh");

                        fire1Node->attachObject(cannon1Entity);
                        fire1Node->setScale(0.05, 0.05, 0.05);
                        fire2Node->attachObject(cannon2Entity);
                        fire2Node->setScale(0.05, 0.05, 0.05);
                        fire3Node->attachObject(cannon3Entity);
                        fire3Node->setScale(0.05, 0.05, 0.05);

                        Ogre::ParticleSystem * blowParticle1 = pSceneMgr->createParticleSystem("Cannon1", "Examples/Fire");
                        Ogre::ParticleSystem * blowParticle2 = pSceneMgr->createParticleSystem("Cannon2", "Examples/Fire");
                        Ogre::ParticleSystem * blowParticle3 = pSceneMgr->createParticleSystem("Cannon3", "Examples/Fire");
                        //fire1Node->attachObject(blowParticle1);
                        //fire2Node->attachObject(blowParticle2);
                        //fire3Node->attachObject(blowParticle3);
                        //blowParticle->setEmitting(true);*/

                        Ogre::SceneNode **cannonNodes = new Ogre::SceneNode*[21];
                        Ogre::Entity **cannonEntities = new Ogre::Entity*[21];
                        for(int i = 0; i < 21; ++i)
                        {
                            cannonNodes[i] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
                            cannonNodes[i]->setPosition(Ogre::Vector3(100, 100, 100));
                            cannonNodes[i]->pitch(Ogre::Degree(90));
                            cannonNodes[i]->roll(Ogre::Degree(165));

                            cannonEntities[i] = pSceneMgr->createEntity("Cannon" + to_string((i + 1)),
                                                                        "Cannon" + to_string((i + 1))
                                                                        + ".mesh");

                            cannonNodes[i]->attachObject(cannonEntities[i]);
                            cannonNodes[i]->setScale(0.05, 0.05, 0.05);
                        }
                        blowParticle = pSceneMgr->createParticleSystem("Cannon1", "Examples/FireLeft1");
                        cannonNodes[1]->attachObject(blowParticle);
                        Ogre::ParticleSystem * blowParticle1 = pSceneMgr->createParticleSystem("Cannon2", "Examples/FireLeft2");
                        cannonNodes[2]->attachObject(blowParticle1);
                        Ogre::ParticleSystem * blowParticle2 = pSceneMgr->createParticleSystem("Cannon3", "Examples/FireLeft3");
                        cannonNodes[3]->attachObject(blowParticle2);
                        Ogre::ParticleSystem * blowParticle3 = pSceneMgr->createParticleSystem("Cannon4", "Examples/FireLeft4");
                        cannonNodes[4]->attachObject(blowParticle3);
                        Ogre::ParticleSystem * blowParticle4 = pSceneMgr->createParticleSystem("Cannon5", "Examples/FireLeft5");
                        cannonNodes[5]->attachObject(blowParticle4);
                        Ogre::ParticleSystem * blowParticle5 = pSceneMgr->createParticleSystem("Cannon6", "Examples/FireLeft6");
                        cannonNodes[6]->attachObject(blowParticle5);
                        Ogre::ParticleSystem * blowParticle6 = pSceneMgr->createParticleSystem("Cannon7", "Examples/FireLeft7");
                        cannonNodes[7]->attachObject(blowParticle6);
                        Ogre::ParticleSystem * blowParticle7 = pSceneMgr->createParticleSystem("Cannon8", "Examples/FireLeft8");
                        cannonNodes[8]->attachObject(blowParticle7);
                        Ogre::ParticleSystem * blowParticle8 = pSceneMgr->createParticleSystem("Cannon9", "Examples/FireLeft9");
                        cannonNodes[9]->attachObject(blowParticle8);
                        Ogre::ParticleSystem * blowParticle9 = pSceneMgr->createParticleSystem("Cannon10", "Examples/FireLeft10");
                        cannonNodes[10]->attachObject(blowParticle9);
                        Ogre::ParticleSystem * blowParticle10 = pSceneMgr->createParticleSystem("Cannon11", "Examples/FireLeft11");
                        cannonNodes[11]->attachObject(blowParticle10);
                        Ogre::ParticleSystem * blowParticle11 = pSceneMgr->createParticleSystem("Cannon12", "Examples/FireLeft12");
                        cannonNodes[12]->attachObject(blowParticle11);
                        Ogre::ParticleSystem * blowParticle12 = pSceneMgr->createParticleSystem("Cannon13", "Examples/FireLeft13");
                        cannonNodes[13]->attachObject(blowParticle12);
                        Ogre::ParticleSystem * blowParticle13 = pSceneMgr->createParticleSystem("Cannon14", "Examples/FireLeft14");
                        cannonNodes[14]->attachObject(blowParticle13);
                        Ogre::ParticleSystem * blowParticle14 = pSceneMgr->createParticleSystem("Cannon15", "Examples/FireLeft15");
                        cannonNodes[15]->attachObject(blowParticle14);
                        Ogre::ParticleSystem * blowParticle15 = pSceneMgr->createParticleSystem("Cannon16", "Examples/FireLeft16");
                        cannonNodes[16]->attachObject(blowParticle15);
                        Ogre::ParticleSystem * blowParticle16 = pSceneMgr->createParticleSystem("Cannon17", "Examples/FireLeft17");
                        cannonNodes[17]->attachObject(blowParticle16);
                        Ogre::ParticleSystem * blowParticle17 = pSceneMgr->createParticleSystem("Cannon18", "Examples/FireLeft18");
                        cannonNodes[18]->attachObject(blowParticle17);

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
	
    JNIEXPORT void JNICALL Java_org_ogre3d_android_OgreActivityJNI_renderOneFrame(JNIEnv * env, jobject obj,
                                                                                  jint direction, jfloat angleHor, jfloat angleVer)
    {
        if(gRenderWnd != nullptr && gRenderWnd->isActive())
        {
            try
            {
                if(gVM->AttachCurrentThread(&env, nullptr) < 0)
                    return;

                gRenderWnd->windowMovedOrResized();
                gRoot->renderOneFrame();

                int dir_ = (int) direction;
                if(dir_ != 0)
                {

                    switch(dir_)
                    {
                        case 1:
                            initXposition += 1; break;
                        case 2:
                            initXposition += 1;
                            initZposition -= 1; break;
                        case 3:
                            initZposition -= 1; break;
                        case 4:
                            initZposition -= 1;
                            initXposition -= 1; break;
                        case 5:
                            initXposition -= 1; break;
                        case 6:
                            initXposition -= 1;
                            initZposition += 1; break;
                        case 7:
                            initZposition += 1; break;
                        case 8:
                            initZposition += 1;
                            initXposition += 1; break;
                    }

                }

                camAngleHor += (float)angleHor;
                camAngleVer += (float)angleVer;

                camXposition = sin(camAngleVer) * cos(camAngleHor) + initXposition;
                camZposition = sin(camAngleVer) * sin(camAngleHor) + initZposition;
                camYposition = cos(camAngleVer) + CAMERA_Y_POSITION;

                pCamera->setPosition(initXposition, CAMERA_Y_POSITION, initZposition);
                pCamera->lookAt(camXposition, camYposition, camZposition);

                shipNode->setPosition(Ogre::Vector3(camXposition, VESSEL_Y_POSITION, camZposition - VESSEL_Z_DISTATION));
                wakeNode->setPosition(Ogre::Vector3(camXposition + 19, -19, camZposition - VESSEL_Z_DISTATION + 20));

                //gVM->DetachCurrentThread();
            }
            catch(Ogre::RenderingAPIException ex) {}
        }
    }
}
