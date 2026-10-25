#ifndef GIRIGARA_CORE_IDENTIFIABLE_H
#define GIRIGARA_CORE_IDENTIFIABLE_H

#include <Core/UUID.h>
#include <string>

namespace Core {

    class Identifiable {
    public:
        Identifiable() : m_uuid() {}
        explicit Identifiable(std::string name) : m_uuid(), m_name(std::move(name)) {}
        virtual ~Identifiable() = default;

        UUID getUUID() const { return m_uuid; }
        const std::string& getName() const { return m_name; }
        virtual void setName(const std::string& name) { m_name = name; }

    protected:
        UUID m_uuid;
        std::string m_name;
    };

} // namespace Core

#endif // GIRIGARA_CORE_IDENTIFIABLE_H
