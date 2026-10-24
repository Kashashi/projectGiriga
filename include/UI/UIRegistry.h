#ifndef GIRIGARA_UI_UI_REGISTRY_H
#define GIRIGARA_UI_UI_REGISTRY_H

#include <string>
#include <unordered_map>
#include <cstdint>
#include <atomic>

class UIElement;

class UIRegistry {
public:
    static UIRegistry& getInstance() {
        static UIRegistry instance;
        return instance;
    }

    // 生成全域唯一 UUID (從 1 開始，0 代表無效)
    uint64_t generateUUID() {
        static std::atomic<uint64_t> s_counter{1};
        return s_counter.fetch_add(1, std::memory_order_relaxed);
    }

    // 註冊物件
    void registerElement(uint64_t uuid, const std::string& name, UIElement* element) {
        if (!element || uuid == 0) return;
        m_byUUID[uuid] = element;
        if (!name.empty()) {
            m_byName[name] = element;
        }
    }

    // 物件解構時自動註銷
    void unregisterElement(uint64_t uuid, const std::string& name) {
        m_byUUID.erase(uuid);
        if (!name.empty()) {
            auto it = m_byName.find(name);
            if (it != m_byName.end() && it->second == m_byUUID[uuid]) {
                m_byName.erase(it);
            }
        }
    }

    // 依名稱查詢
    UIElement* findByName(const std::string& name) const {
        auto it = m_byName.find(name);
        return (it != m_byName.end()) ? it->second : nullptr;
    }

    // 依 UUID 查詢
    UIElement* findByUUID(uint64_t uuid) const {
        auto it = m_byUUID.find(uuid);
        return (it != m_byUUID.end()) ? it->second : nullptr;
    }

private:
    UIRegistry() = default;
    ~UIRegistry() = default;

    std::unordered_map<uint64_t, UIElement*> m_byUUID;
    std::unordered_map<std::string, UIElement*> m_byName;
};

#endif // GIRIGARA_UI_UI_REGISTRY_H
