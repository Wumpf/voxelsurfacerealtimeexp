#include "PCH.h"
#include "Camera.h"

Camera::Camera(float fov, float aspectRatio) :
  m_vPosition(0.0f, 200.0f, 0.0f),
  m_vUp(0.0f, 1.0f, 0.0f),
  m_ViewDir(1.0f, 0.0f, 0.0f)
{
  const float nearPlane = 0.1f;
  const float farPlane = 1000.0f;
  m_ProjectionMatrix.SetPerspectiveProjectionMatrixFromFovY(fov, aspectRatio, nearPlane, farPlane, ezProjectionDepthRange::MinusOneToOne);

  UpdateMatrices();
}

Camera::~Camera(void)
{
}

void Camera::UpdateMatrices()
{
  m_ViewMatrix.SetLookAtMatrix(m_vPosition, m_vPosition + m_ViewDir, m_vUp);

  ezVec3 vPos = m_vPosition;
  ezVec3 vLockAt = m_vPosition+ m_ViewDir;

  m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix ;
}