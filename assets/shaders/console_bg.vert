#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

// 與 ui.vert 一致的 Uniform Location
layout (location = 0) uniform mat4 projection;
layout (location = 1) uniform mat4 model;

layout (location = 0) out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
