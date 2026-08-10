#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 colors;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_perspectiveProjection;

out vec3 v_vertexColors;

void main()
{
  v_vertexColors = colors;
  // applied in right to left order
  vec4 newPosition = u_perspectiveProjection  // camera space --> clip space
                   * u_viewMatrix             // world space --> camera space
                   * u_modelMatrix            // local space --> world space
                   * vec4(position, 1.0f);
  gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
}

