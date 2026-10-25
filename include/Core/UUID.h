#ifndef GIRIGARA_CORE_UUID_H
#define GIRIGARA_CORE_UUID_H

#include <cstdint>
#include <random>
#include <functional>

namespace Core {

    class UUID {
    public:
        // 預設建構子：自動生成唯一 ID
        UUID() : m_uuid(generate()) {}

        // 指定已知 ID 建構
        explicit UUID(uint64_t uuid) : m_uuid(uuid) {}

        UUID(const UUID&) = default;
        UUID& operator=(const UUID&) = default;

        // 支援隱含轉換為 uint64_t，無縫相容舊程式碼
        operator uint64_t() const { return m_uuid; }
        uint64_t get() const { return m_uuid; }

        bool isValid() const { return m_uuid != 0; }

        bool operator==(const UUID& other) const { return m_uuid == other.m_uuid; }
        bool operator!=(const UUID& other) const { return m_uuid != other.m_uuid; }
        bool operator<(const UUID& other) const { return m_uuid < other.m_uuid; }

    private:
        static uint64_t generate() {
            // 使用 thread_local 保持亂數引擎高效且執行緒安全
            static thread_local std::mt19937_64 engine(std::random_device{}());
            static thread_local std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);
            return dist(engine);
        }

        uint64_t m_uuid{0};
    };

} // namespace Core

// 特化 std::hash，使其支援 std::unordered_map 與 std::unordered_set
namespace std {
    template <>
    struct hash<Core::UUID> {
        std::size_t operator()(const Core::UUID& uuid) const noexcept {
            return std::hash<uint64_t>()(uuid.get());
        }
    };
}

#endif // GIRIGARA_CORE_UUID_H
