#ifndef GIRIGARA_EVENT_EVENT_BUS_H
#define GIRIGARA_EVENT_EVENT_BUS_H

#include <Event/Event.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <typeindex>
#include <utility>

class EventBus {
public:
    template <typename T>
    using EventCallback = std::function<void(T&)>;

    // 一般訂閱 (排在佇列尾端)
    template <typename T>
    void subscribe(EventCallback<T> callback) {
        auto type = std::type_index(typeid(T));
        m_subscribers[type].push_back([callback](Event& event) {
            callback(static_cast<T&>(event));
        });
    }

    // ★ 優先訂閱 (排在佇列最前端，供 Console / 全域攔截器使用)
    template <typename T>
    void subscribePriority(EventCallback<T> callback) {
        auto type = std::type_index(typeid(T));
        m_subscribers[type].insert(m_subscribers[type].begin(), [callback](Event& event) {
            callback(static_cast<T&>(event));
        });
    }

    // 發布左值事件 (實體變數)
    template <typename T>
    void publish(T& event) {
        auto type = std::type_index(typeid(T));
        auto it = m_subscribers.find(type);
        if (it != m_subscribers.end()) {
            for (auto& handler : it->second) {
                handler(event);
                if (event.isHandled) {
                    break; // 被處理則中止後續廣播
                }
            }
        }
    }

    // ★ 發布右值暫時物件 (支援 publish(CharInputEvent{cp}))
    template <typename T>
    void publish(T&& event) {
        T ev = std::move(event);
        publish(ev);
    }

private:
    using RawHandler = std::function<void(Event&)>;
    std::unordered_map<std::type_index, std::vector<RawHandler>> m_subscribers;
};

#endif // GIRIGARA_EVENT_EVENT_BUS_H
