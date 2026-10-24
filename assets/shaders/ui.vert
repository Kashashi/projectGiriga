#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

// 明確指定 Uniform Location，C++ 側可直接寫入固定 slot
layout (location = 0) uniform mat4 projection;
layout (location = 1) uniform mat4 model;

layout (location = 0) out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
