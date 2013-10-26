  layout(binding = 0, shared) uniform Camera
  {
	  mat4 ViewMatrix;
	  mat4 ViewProjection;
	  mat4 InverseViewProjection;
	  vec3 CameraPosition;
  };

  layout(binding = 1, shared) uniform Time
  {
    float CurrentTime;
    float LastFrameDuration;
  };

  layout(binding = 2, shared) uniform VolumeDataInfo
  {
    vec3 VolumeWorldSize;
  };

  const float IsoValue = 0.5;