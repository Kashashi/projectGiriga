#ifndef GIRIGARA_EVENT_EVENT_BUS_H
#define GIRIGARA_EVENT_EVENT_BUS_H

#include <Event/Event.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <typeindex>

class EventBus {
public:
    template <typename T>
    using EventCallback = std::function<void(T&)>;

    template <typename T>
    void subscribe(EventCallback<T> callback) {
        auto type = std::type_index(typeid(T));
        m_subscribers[type].push_back([callback](Event& event) {
            callback(static_cast<T&>(event));
        });
    }

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

private:
    using RawHandler = std::function<void(Event&)>;
    std::unordered_map<std::type_index, std::vector<RawHandler>> m_subscribers;
};

#endif // GIRIGARA_EVENT_EVENT_BUS_H
