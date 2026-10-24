#ifndef GIRIGARA_SYSTEM_KEYBOARD_SYSTEM_H
#define GIRIGARA_SYSTEM_KEYBOARD_SYSTEM_H

#include <Event/EventBus.h>
#include <unordered_map>
#include <functional>

struct ShortcutKey {
    int key;
    int mods;

    bool operator==(const ShortcutKey& other) const {
        return key == other.key && mods == other.mods;
    }
};

struct ShortcutKeyHash {
    std::size_t operator()(const ShortcutKey& k) const {
        return std::hash<int>()(k.key) ^ (std::hash<int>()(k.mods) << 1);
    }
};

class KeyboardSystem {
public:
    KeyboardSystem(EventBus& bus);

    // 註冊快捷鍵 (例如: GLFW_KEY_U, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)
    void registerShortcut(int key, int mods, std::function<void()> action);

private:
    void onKeyPressed(KeyPressedEvent& e);

    std::unordered_map<ShortcutKey, std::function<void()>, ShortcutKeyHash> m_shortcuts;
};

#endif // GIRIGARA_SYSTEM_KEYBOARD_SYSTEM_H
