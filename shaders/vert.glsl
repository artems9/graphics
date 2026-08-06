#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 colors;

uniform mat4 u_modelMatrix;
uniform mat4 u_perspectiveProjection;

out vec3 v_vertexColors;

void main()
{
  v_vertexColors = colors;
  vec4 newPosition = u_perspectiveProjection * u_modelMatrix * vec4(position, 1.0f);
  gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
}

