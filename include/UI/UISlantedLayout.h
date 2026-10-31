#ifndef GIRIGARA_UI_UI_SLANTED_LAYOUT_H
#define GIRIGARA_UI_UI_SLANTED_LAYOUT_H

#include <UI/UIContainer.h>

class UISlantedLayout : public UIContainer {
public:
    using UIContainer::UIContainer;
    ~UISlantedLayout() override = default;

    void setPadding(float left, float top) {
        m_paddingLeft = left;
        m_paddingTop = top;
    }

    void setSpacing(float spacing) { m_spacing = spacing; }

    // ★ 關鍵：設定每一個元件相對前一個元件的水平偏移量 (X 軸傾斜量)
    // 例如設為 25.0f 代表每往下一格，就向右歪 25 像素；設為負數則往左歪
    void setSlantOffsetX(float offset) { m_slantOffsetX = offset; }

    void update(float dt) override;

private:
    float m_paddingLeft   = 10.0f;
    float m_paddingTop    = 10.0f;
    float m_spacing       = 8.0f;
    float m_slantOffsetX  = 20.0f; // 階梯式斜率偏移

    void performLayout();
};

#endif // GIRIGARA_UI_UI_SLANTED_LAYOUT_H
