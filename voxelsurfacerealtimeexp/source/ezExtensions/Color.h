#pragma once

#include <Foundation/Math/Vec4.h>

class ezColor : public ezVec4Template<float>
{
public:

  ezColor() : ezVec4Template<float>() {}
  ezColor(float r, float g, float b, float a = 1.0f) : ezVec4Template<float>(r, g, b, a) {}
  ezColor(float rgb) : ezVec4Template<float>(rgb, rgb, rgb, 1.0f) {}

  EZ_FORCE_INLINE ezColor BGRA() { return ezColor(B(), G(), R(), A()); }

  EZ_FORCE_INLINE float R() const { return x; }
  EZ_FORCE_INLINE float G() const { return y; }
  EZ_FORCE_INLINE float B() const { return z; }
  EZ_FORCE_INLINE float A() const { return w; }
  EZ_FORCE_INLINE float& R() { return x; }
  EZ_FORCE_INLINE float& G() { return y; }
  EZ_FORCE_INLINE float& B() { return z; }
  EZ_FORCE_INLINE float& A() { return w; }

  static const ezColor White;
  static const ezColor Black;
  static const ezColor Grey;
  static const ezColor Red;
  static const ezColor Green;
  static const ezColor Blue;
};