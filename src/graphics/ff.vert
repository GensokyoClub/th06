precision mediump float;

attribute vec3 position;
attribute vec2 texCoords;
attribute vec4 diffuse;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;

varying vec2 interpTexCoords;
varying vec4 interpDiffuse;
varying float viewZ;

void main() {
    interpTexCoords = (textureMatrix * vec4(texCoords, 1.0, 1.0)).xy;
    interpDiffuse = diffuse;

    vec4 viewCoordinates = modelviewMatrix * vec4(position, 1.0);
    viewZ = viewCoordinates.z;

    gl_Position = projectionMatrix * viewCoordinates;
}
