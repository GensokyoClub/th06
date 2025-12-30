precision highp float;

attribute vec3 position;
attribute vec2 texCoords;
attribute vec4 diffuse;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;

uniform float fogNear;
uniform float fogFar;

varying mediump vec2 interpTexCoords;
varying lowp vec4 interpDiffuse;
varying mediump float fogCoeff;

void main()
{
    interpTexCoords = (textureMatrix * vec4(texCoords, 1.0, 1.0)).xy;
    interpDiffuse = diffuse;

    vec4 viewPos = modelviewMatrix * vec4(position, 1.0);

    fogCoeff = (fogFar - viewPos.z) / (fogFar - fogNear);
    fogCoeff = clamp(fogCoeff, 0.0, 1.0);

    gl_Position = projectionMatrix * viewPos;
}
