LOCAL_PATH := $(call my-dir)
    include $(CLEAR_VARS)
    LOCAL_MODULE    := OgreJNI

    LOCAL_LDLIBS	:= -landroid -lc -lm -ldl -llog -lEGL -lGLESv2
    LOCAL_LDLIBS	+= -L/home/walter/Android/OgreSource/ogre/build/lib -L/home/walter/Android/OgreSource/ogre/AndroidDependencies/lib/armeabi-v7a
	
    LOCAL_LDLIBS	+= -lPlugin_ParticleFXStatic -lPlugin_OctreeSceneManagerStatic -lRenderSystem_GLES2Static -lOgreRTShaderSystemStatic -lOgreOverlayStatic -lOgreTerrainStatic -lOgrePagingStatic -lOgreVolumeStatic -lOgreMainStatic
    LOCAL_LDLIBS	+= -lzzip -lz -lFreeImage -lfreetype -lOIS  /home/walter/Android/OgreSource/ogre/build/systemlibs/armeabi-v7a/libsupc++.a /home/walter/Android/OgreSource/ogre/build/systemlibs/armeabi-v7a/libstdc++.a /home/walter/Android/OgreSource/ogre/build/OgreJNI/obj/local/armeabi-v7a/libcpufeatures.a
    LOCAL_STATIC_LIBRARIES := android_native_app_glue cpufeatures
    
    LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES=1 -I/home/walter/Android/OgreSource/ogre/build/include -I/home/walter/Android/OgreSource/ogre/OgreMain/include -I/home/walter/Android/OgreSource/ogre/RenderSystems/GLES2/include -I/home/walter/Android/OgreSource/ogre/RenderSystems/GLES2/include/EGL
    LOCAL_CFLAGS += -I/home/walter/Android/android-ndk-r8c/sources/cpufeatures -I/home/walter/Android/OgreSource/ogre/Components/RTShaderSystem/include -I/home/walter/Android/OgreSource/ogre/Components/Overlay/include -I/home/walter/Android/OgreSource/ogre/Components/Volume/include -I/home/walter/Android/OgreSource/ogre/Components/Terrain/include -I/home/walter/Android/OgreSource/ogre/Components/Paging/include
    LOCAL_CFLAGS += -I/home/walter/Android/OgreSource/ogre/PlugIns/ParticleFX/include -I/home/walter/Android/OgreSource/ogre/PlugIns/OctreeSceneManager/include 
    LOCAL_CFLAGS += -I/home/walter/Android/OgreSource/ogre/AndroidDependencies/include -I/home/walter/Android/OgreSource/ogre/AndroidDependencies/include/OIS -I/home/walter/Android/OgreSource/ogre/build/Samples/include
    LOCAL_CFLAGS += -fexceptions -frtti -x c++ -D___ANDROID___ -DANDROID -DZZIP_OMIT_CONFIG_H -DINCLUDE_RTSHADER_SYSTEM=1 -std=c++11
    
    #hydrax
    #LOCAL_CFLAGS += -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Modules -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Modules/ProjectedGrid -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Modules/RadialGrid -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Modules/SimpleGrid -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Noise -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Noise/FFT -I/home/walter/Android/OgreSource/HydraX/Hydrax/src/Hydrax/Noise/Perlin
    
    
    #HYDRAX_PATH := ../../../../HydraX/Hydrax/src/Hydrax
    #PROJECT_FILES1 := $(wildcard $(LOCAL_PATH)/$(HYDRAX_PATH)/*.cpp)
    #LOCAL_SRC_FILES += $(PROJECT_FILES1:$(LOCAL_PATH)/%=%)
    
    #HYDRAX_NOISE_MODULES_PATH := ../../../../HydraX/Hydrax/src/Hydrax/*
    #PROJECT_FILES2 := $(wildcard $(LOCAL_PATH)/$(HYDRAX_NOISE_MODULES_PATH)/*.cpp)
    #LOCAL_SRC_FILES += $(PROJECT_FILES2:$(LOCAL_PATH)/%=%)
    
    #HYDRAX_NOISE_SUBPATH := ../../../../HydraX/Hydrax/src/Hydrax/Noise/*
    #PROJECT_FILES3 := $(wildcard $(LOCAL_PATH)/$(HYDRAX_NOISE_SUBPATH)/*.cpp)
    #LOCAL_SRC_FILES += $(PROJECT_FILES3:$(LOCAL_PATH)/%=%)
    
    #HYDRAX_MODULES_SUBPATH := ../../../../HydraX/Hydrax/src/Hydrax/Modules/*
    #PROJECT_FILES4 := $(wildcard $(LOCAL_PATH)/$(HYDRAX_MODULES_SUBPATH)/*.cpp)
    #LOCAL_SRC_FILES += $(PROJECT_FILES4:$(LOCAL_PATH)/%=%)
    
	LOCAL_PATH := /home/walter/Android/OgreSource/ogre/build/OgreJNI/jni
	LOCAL_SRC_FILES += WaterMesh.cpp dutchfrigate.cpp ship.cpp OgreActivityJNI.cpp
	        
    include $(BUILD_SHARED_LIBRARY) 
      
    $(call import-module,android/cpufeatures) 
    $(call import-module,android/native_app_glue) 
