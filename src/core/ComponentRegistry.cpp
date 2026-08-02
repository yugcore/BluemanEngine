#include "ComponentRegistry.h"

namespace EngineEditor {

ComponentRegistry& ComponentRegistry::Get() {
    static ComponentRegistry instance;
    return instance;
}

std::vector<const char*> ComponentRegistry::GetComponentTypeNames(uint64_t entityId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<const char*> typeNames;
    for (const auto& pair : m_StorageMap) {
        if (pair.second && pair.second->HasEntity(entityId)) {
            typeNames.push_back(pair.second->GetComponentTypeName());
        }
    }
    return typeNames;
}

void ComponentRegistry::RemoveAllComponents(uint64_t entityId) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& pair : m_StorageMap) {
        if (pair.second) {
            pair.second->RemoveEntity(entityId);
        }
    }
}

void ComponentRegistry::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& pair : m_StorageMap) {
        if (pair.second) {
            pair.second->Clear();
        }
    }
}

} // namespace EngineEditor
