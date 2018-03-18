#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout (location = 0) in vec3 vertCoordinates_in;
layout (location = 1) in vec3 vertNormals_in;
layout (location = 2) in vec2 texCoords_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelViewTransform;
uniform mat4 projectionTransform;
uniform vec3 lightPosition;
uniform mat3 normalTransform;

// Specify the output of the vertex stage
out vec3 vertNormal;
out vec3 vertPosition;
out vec3 relativeLightPosition;
out vec2 uvCoords;
out vec3 pos;

// Specify wave constants
uniform float amp[3];
uniform float freq[3];
uniform float phase[3];
uniform float timestep = 100;
uniform float time;

void main()
{
    pos = vertCoordinates_in;
    pos.z = 0;
    float dU = 0;
    for (int i = 0; i < 2; i++) {
        pos.z += amp[i] * sin(2*M_PI*(time/timestep + phase[i] + pos.y*freq[i]));
        dU += 2*M_PI * freq[i] * amp[i] * cos(2*M_PI*(time/timestep + phase[i] + pos.y*freq[i]));
    }

    // derivative of pos.z:
    // float dU = 2*M_PI * freq * amp * cos(2*M_PI*(time/timestep + pos.y*freq));
    vertNormal = normalTransform * vec3(-dU, 0, 1.0);

    gl_Position  = projectionTransform * modelViewTransform * vec4(pos, 1.0);

    // Pass the required information to the fragment stage.
    relativeLightPosition = vec3(modelViewTransform * vec4(lightPosition, 1));
    vertPosition = vec3(modelViewTransform * vec4(pos, 1));
    uvCoords     = texCoords_in;
}
