#include <jni.h>
#include <EGL/egl.h>
#include <android/api-level.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "OgrePlatform.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreArchiveManager.h"
#include "Android/OgreAndroidEGLWindow.h"
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"
#include <iostream>
#include "InputContext.h"
#include "OgreFileSystemLayer.h"
#include "SdkTrays.h"
#include <android/log.h>
#include "SdkCameraMan.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

//#include "Caelum.h"
//#include "CaelumSystem.h"
//#include "Hydrax.h"

#ifdef OGRE_BUILD_PLUGIN_OCTREE
#	include "OgreOctreePlugin.h"
#endif

#ifdef OGRE_BUILD_PLUGIN_PFX
#	include "OgreParticleFXPlugin.h"
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
#	include "OgreOverlaySystem.h"
#endif

#include "OgreConfigFile.h"

#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#	include "OgreGLES2Plugin.h"
#	define GLESRS GLES2Plugin
#else
#	include "OgreGLESPlugin.h"
#	define GLESRS GLESPlugin
#endif

#define ENABLE_SHADERS_CACHE_SAVE 1
#define ENABLE_SHADERS_CACHE_LOAD 1
#ifdef OGRE_STATIC_LIB
#   ifdef INCLUDE_RTSHADER_SYSTEM
#       include "OgreRTShaderSystem.h"
#       define _RTSS_WRITE_SHADERS_TO_DISK
#   endif // INCLUDE_RTSHADER_SYSTEM
#endif // OGRE_STATIC_LIB

/** Shader System RTTS bridge */
//#ifdef INCLUDE_RTSHADER_SYSTEM

class ShaderGeneratorTechniqueResolverListener : public Ogre::MaterialManager::Listener
{
public:
    ShaderGeneratorTechniqueResolverListener(/*Ogre::RTShader::ShaderGenerator * pShaderGenerator*/):mShaderGenerator(nullptr)
    {
        //mShaderGenerator = pShaderGenerator;
        mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    }

    virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex,
    const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex,
    const Ogre::Renderable* rend)
    {
        Ogre::Technique* generatedTech = nullptr;

        // Case this is the default shader generator scheme.
        if (schemeName == Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
        {
            bool techniqueCreated;

            // Create shader generated technique for this material.
            techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
            originalMaterial->getName(),
            Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
            schemeName);

            // Case technique registration succeeded.
            if (techniqueCreated)
            {
                // Force creating the shaders for the generated technique.
                mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName());

                // Grab the generated technique.
                Ogre::Material::TechniqueIterator itTech = originalMaterial->getTechniqueIterator();

                while (itTech.hasMoreElements())
                {
                    Ogre::Technique* curTech = itTech.getNext();

                    if (curTech->getSchemeName() == schemeName)
                    {
                        generatedTech = curTech;
                        break;
                    }
                }
            }
        }

        return generatedTech;
    }


protected:
        Ogre::RTShader::ShaderGenerator * mShaderGenerator;

};
//#endif // INCLUDE_RTSHADER_SYSTEM

