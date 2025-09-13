#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    // This shader just passes the vertex positions through.
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}

