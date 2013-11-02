#pragma once 

namespace GeneralConfig
{
  // defined in RenderWindow.cpp
  extern ezCVarInt g_ResolutionWidth;
  extern ezCVarInt g_ResolutionHeight;
  ezSizeU32 GetScreenResolution();
  ezVec2 GetScreenResolutionF();
}

namespace SceneConfig
{
  // defined in Scene.cpp
  extern ezCVarBool g_Wireframe;
  extern ezCVarBool g_UseReferenceVis;

  // defined in VoxelTerrain.cpp
  extern ezCVarFloat g_GradientDescendStepMultiplier;
  extern ezCVarInt g_GradientDescendStepCount;
  extern ezCVarBool g_UseAnisotropicFilter;

  // this cvars are not saved, they only provide different informations about the general status
  namespace Status
  {
    extern ezCVarFloat g_VolumePrepareTime;
    extern ezCVarFloat g_VolumeDrawTime;
  };
}