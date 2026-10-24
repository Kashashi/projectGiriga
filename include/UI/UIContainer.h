#ifndef GIRIGARA_UI_UI_CONTAINER_H
#define GIRIGARA_UI_UI_CONTAINER_H

#include <UI/UIElement.h>
#include <vector>
#include <memory>

class UIContainer : public UIElement {
public:
    UIContainer() = default;
    UIContainer(float x, float y, float width, float height)
    : UIElement(x, y, width, height) {}
    ~UIContainer() override = default;

    template <typename T, typename... Args>
    T* emplaceChild(Args&&... args) {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = child.get();
        m_children.push_back(std::move(child));
        return ptr;
    }

    void addChild(std::unique_ptr<UIElement> child);
    void clear();

    void setFocusedElement(UIElement* element);
    UIElement* getFocusedElement() const { return m_focusedElement; }

    // 核心補齊：遞迴更新子節點
    void update(float dt) override;

    void render(const Shader& shader) override;
    bool handleMouseMove(double x, double y) override;
    bool handleMouseButton(int button, int action, int mods, double x, double y) override;
    bool handleKey(int key, int scancode, int action, int mods) override;

private:
    std::vector<std::unique_ptr<UIElement>> m_children;
    UIElement* m_focusedElement = nullptr;
};

#endif // GIRIGARA_UI_UI_CONTAINER_H
