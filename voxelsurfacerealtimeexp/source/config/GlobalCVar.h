#pragma once 

namespace GeneralConfig
{
  // defined in RenderWindow.cpp
  extern ezCVarInt g_ResolutionWidth;
  extern ezCVarInt g_ResolutionHeight;
  ezSizeU32 GetScreenResolution();
  ezVec2 GetScreenResolutionF();
}

// defined in Scene.cpp
namespace SceneConfig
{
  extern ezCVarBool g_Wireframe;
  extern ezCVarBool g_UseReferenceVis;

  // this cvars are not saved, they only provide different informations about the general status
  namespace Status
  {
    extern ezCVarFloat g_VolumePrepareTime;
    extern ezCVarFloat g_VolumeDrawTime;
  };
}