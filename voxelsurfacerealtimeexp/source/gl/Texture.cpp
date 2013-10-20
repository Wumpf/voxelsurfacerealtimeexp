#include "PCH.h"
#include "Texture.h"

namespace gl
{
  Texture::Texture(ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, GLuint format, ezInt32 iNumMipLevels) : 
    m_uiWidth(uiWidth),
    m_uiHeight(uiHeight),
    m_uiDepth(uiDepth),

    m_Format(format),
    m_uiNumMipLevels(ConvertMipMapSettingToActualCount(iNumMipLevels, uiWidth, uiHeight, format))
  {
    glGenTextures(1, &m_TextureHandle);
  }

  Texture::~Texture()
  {
    glDeleteTextures(1, &m_TextureHandle);
  }

  ezUInt32 Texture::ConvertMipMapSettingToActualCount(ezInt32 iMipMapSetting, ezUInt32 width, ezUInt32 height, ezUInt32 depth)
  {
    if(iMipMapSetting == 0)
      return 1;

    else if(iMipMapSetting < 0)
    {
      ezUInt32 uiNumMipLevels = 0;
      if(depth != 0)
      {
        while(width > 1 && height > 1 && depth > 1)
        {
          width /= 2;
          height /= 2;
          depth /= 2;
          ++uiNumMipLevels;
        }
      }
      else
      {
        while(width > 1 && height > 1)
        {
          width /= 2;
          height /= 2;
          ++uiNumMipLevels;
        }
      }
      return uiNumMipLevels;
    }

    else
      return iMipMapSetting;
  }

}