#include "PCH.h"
#include "Camera.h"

Camera::Camera(float fov, float aspectRatio) :
  m_vPosition(0.0f, 200.0f, 0.0f),
  m_vUp(0.0f, 1.0f, 0.0f),
  m_ViewDir(1.0f, 0.0f, 0.0f)
{
  m_ViewProjectionMatrix.SetPerspectiveProjectionMatrixFromFovY(fov, aspectRatio, 0.1f, 5000.0f, ezProjectionDepthRange::MinusOneToOne);
  UpdateMatrices();
}

Camera::~Camera(void)
{
}

void Camera::UpdateMatrices()
{
  m_ViewMatrix.SetLookAtMatrix(m_vPosition, m_vPosition + m_ViewDir , m_vUp);
  m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
}