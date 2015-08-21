#include "shadergeneratortechniqueresolverlistener.h"

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

/** Player's vessel node */
DutchFrigate * playersShip;
Ogre::SceneNode * wakeNode;


/** Particle system for cannon shot */
//ParticleSystem * blowParticle;

#define CAMERA_Y_POSITION 80
#define VESSEL_Y_POSITION -25
#define VESSEL_Z_DISTATION 210

#include <android/log.h>


void * fireLeft(void *)
{
    playersShip->setEmittingLeft(true);
    __android_log_print(ANDROID_LOG_VERBOSE, "BLOW_THEM", "SLEEPED", 1);
    usleep(300000);
    __android_log_print(ANDROID_LOG_VERBOSE, "BLOW_THEM", "WOKE UP", 1);
    playersShip->setEmittingLeft(false);
}

void * fireRight(void *)
{
    playersShip->setEmittingRight(true);
    __android_log_print(ANDROID_LOG_VERBOSE, "BLOW_THEM", "SLEEPED", 1);
    usleep(300000);
    __android_log_print(ANDROID_LOG_VERBOSE, "BLOW_THEM", "WOKE UP", 1);
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

                        playersShip = new DutchFrigate(pSceneMgr,
                                                       Ogre::Vector3(initXposition, VESSEL_Y_POSITION, initZposition - VESSEL_Z_DISTATION),
                                                       21, 18);


                        wakeNode = waterNode->createChildSceneNode("WakeParticle");
                        ParticleSystem * wakeParticle = pSceneMgr->createParticleSystem("Wake", "Water/Wake");
                        wakeNode->attachObject(wakeParticle);

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

                playersShip->setCurrentPosition(Ogre::Vector3(initXposition, VESSEL_Y_POSITION, initZposition - VESSEL_Z_DISTATION));
                wakeNode->setPosition(Ogre::Vector3(initXposition + 19, -19, initZposition - VESSEL_Z_DISTATION + 20));

                //gVM->DetachCurrentThread();
            }
            catch(Ogre::RenderingAPIException ex) {}
        }
    }
}
