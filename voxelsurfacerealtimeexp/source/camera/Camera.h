#pragma once

class Camera
{
public:
  Camera(float fFov, float fAspectRatio);
  virtual ~Camera();

  virtual void Update(ezTime lastFrameDuration)=0;

  const ezMat4& GetProjectionMatrix() const;
  const ezMat4& GetViewMatrix() const;
  const ezMat4& GetViewProjectionMatrix() const;

  const ezVec3& GetPosition() const { return m_vPosition; }
  const ezVec3& GetUp() const { return m_vUp; }
  const ezVec3& GetViewDir() const { return m_ViewDir; }

protected:
  void UpdateMatrices();

  ezVec3 m_vPosition;
  ezVec3 m_vUp;
  ezVec3 m_ViewDir;

private:
  ezMat4 m_ViewProjectionMatrix;
  ezMat4 m_ProjectionMatrix;
  ezMat4 m_ViewMatrix;
};