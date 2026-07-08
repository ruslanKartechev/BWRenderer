#version 330 core

layout (location = 0) in vec3 position;


uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;
out vec4 vertColor;

void main(){
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * MATRIX_MODEL) * vec4(position, 1.0);
    vertColor = vec4(0.95, 0.95, 0.5, 1.0);
}
