#version 460 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 TexCoord;

// 明確綁定 Uniform 與 Texture Unit
layout (location = 2) uniform vec4 uiColor;
layout (location = 3) uniform bool isText;
layout (binding = 0) uniform sampler2D textTexture; // 直接綁定 Texture Unit 0

void main() {
    if (isText) {
        float alpha = texture(textTexture, TexCoord).r;
        FragColor = vec4(uiColor.rgb, alpha * uiColor.a);
    } else {
        FragColor = uiColor;
    }
}
