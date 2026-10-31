#version 460 core

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;

layout (location = 2) uniform float uTime;
layout (location = 3) uniform vec2  uResolution;

// 460 Core 快速整數位元 Hash
uint pcg(uint v) {
    uint state = v * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

vec2 hash2(vec2 p) {
    uvec2 u = uvec2(ivec2(p));
    uint n1 = pcg(u.x + pcg(u.y));
    uint n2 = pcg(n1 + 198491317u);
    return vec2(n1, n2) * (1.0 / 4294967295.0);
}

void main() {
    // -------------------------------------------------------------
    // 1. 淡紫色線性漸層：
    // TexCoord.y = 1.0 為畫面最底下 (30% 不透明度 = 0.30)
    // TexCoord.y = 0.0 為畫面最頂上 (0% 不透明度 = 0.00)
    // -------------------------------------------------------------
    float bottomFactor = clamp(TexCoord.y, 0.0, 1.0);
    float baseAlpha = mix(0.0, 0.30, bottomFactor);
    vec3 basePurple = vec3(0.55, 0.38, 0.88); // 柔和淡紫
    vec4 finalColor = vec4(basePurple, baseAlpha);

    // -------------------------------------------------------------
    // 2. 泡泡生成 (修正裁切問題版)
    // -------------------------------------------------------------
    vec2 aspectUV = TexCoord * vec2(uResolution.x / uResolution.y, 1.0);
    float bubbleIntensity = 0.0;

    for (int layer = 1; layer <= 2; ++layer) {
        // 適度縮放網格密度，讓泡泡分佈更自然
        float scale = 5.0 * float(layer);
        vec2 st = aspectUV * scale;

        float colIdx = floor(st.x);

        // ★ 關鍵修正 1：將搜尋範圍擴大到前後各 2 格 (-2 到 +2)
        // 確保泡泡即便有大半徑加上大幅晃動，也不會因為跨格子被硬生生切掉
        for (float neighbor = -2.0; neighbor <= 2.0; neighbor += 1.0) {
            float cIdx = colIdx + neighbor;
            vec2 rnd = hash2(vec2(cIdx, float(layer) * 37.17));

            float speed = 0.08 + rnd.y * 0.16;       // 浮起速度

            // ★ 關鍵修正 2：微調半徑上限，避免泡泡膨脹到超出網格包容度
            float radius = 0.035 + rnd.x * 0.065;

            // Y 軸飄浮循環區間
            float totalSpan = 1.0 + radius * 2.0;
            float bubbleY = (1.0 + radius) - fract((uTime * speed) + rnd.y) * totalSpan;

            // ★ 關鍵修正 3：晃動幅度限制在安全範圍內 (0.15 格以內)
            float bubbleX = cIdx + 0.5 + sin(uTime * 1.5 + rnd.x * 6.28318) * 0.15;

            vec2 bubblePos = vec2(bubbleX / scale, bubbleY);
            vec2 currentPos = vec2(aspectUV.x, TexCoord.y);

            float dist = length(currentPos - bubblePos);

            // 邊緣光環 (Rim) 與高光
            float rim = smoothstep(radius, radius - 0.008, dist) -
            smoothstep(radius - 0.010, radius - 0.030, dist);

            vec2 hlOffset = vec2(-radius * 0.32, -radius * 0.32);
            float hl = smoothstep(radius * 0.20, 0.0, length(currentPos - (bubblePos + hlOffset)));
            float body = smoothstep(radius, radius - 0.015, dist) * 0.10;

            float currentBubble = (rim * 0.85 + hl * 0.95 + body);

            // ★ 關鍵修正 4：螢幕左右邊界 (TexCoord.x = 0.0 與 1.0) 邊緣軟淡出
            // 避免飄在螢幕最左/最右兩側的泡泡撞到 Console 邊緣硬切
            float screenEdgeFade = smoothstep(0.0, 0.04, TexCoord.x) *
            smoothstep(1.0, 0.96, TexCoord.x);
            currentBubble *= screenEdgeFade;

            bubbleIntensity += currentBubble * (0.45 / float(layer));
        }
    }

    vec4 bubbleColor = vec4(0.85, 0.78, 1.0, clamp(bubbleIntensity, 0.0, 1.0) * 0.8);

    // Alpha 混合
    finalColor.rgb = mix(finalColor.rgb, bubbleColor.rgb, bubbleColor.a);
    finalColor.a = max(finalColor.a, bubbleColor.a * (bottomFactor * 0.6 + 0.3));

    FragColor = finalColor;
}
