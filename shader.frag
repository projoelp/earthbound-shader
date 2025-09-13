#version 330 core

out vec4 FragColor;

uniform vec2 u_resolution;
uniform float u_time;

void main()
{
    // Normalize coordinates (from 0->width to 0->1)
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;

    // Create a color that cycles with time
    float r = 0.5 + 0.5 * sin(u_time);
    float g = 0.5 + 0.5 * cos(u_time);
    float b = uv.x; // Blue channel varies horizontally

    FragColor = vec4(r, g, b, 1.0);
}

