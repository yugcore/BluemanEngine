#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <mutex>
#include <algorithm>
#include <utility>
#include "zegfx.h"

namespace EngineEditor {

// ============================================================================
// Core ECS Component Definitions
// ============================================================================

struct TransformComponent {
    float location[3] = { 0.0f, 0.0f, 0.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3]    = { 1.0f, 1.0f, 1.0f };
};

struct MeshComponent {
    std::string meshPath = "";
    zegfx::RenderMeshHandle renderMeshHandle = {};
    float lodBias = 0.0f;
    bool castShadows = true;
    bool receiveShadows = true;
    bool showBoundingBox = false;
};

struct MaterialComponent {
    std::string materialPath = "";
    zegfx::RenderMaterialHandle renderMaterialHandle = {};
    float baseColor[4] = { 0.80f, 0.80f, 0.80f, 1.00f };
    float roughness = 0.40f;
    float metallic = 0.00f;
    float specular = 0.50f;
    float emissiveColor[3] = { 1.00f, 1.00f, 1.00f };
    float emissiveIntensity = 0.00f;
};

struct LightComponent {
    int lightType = 0; // 0: Directional, 1: Point, 2: Spot
    float color[3] = { 1.00f, 0.95f, 0.85f };
    float intensity = 100000.00f;
    float range = 10.00f;
    float innerCone = 15.00f;
    float outerCone = 45.00f;
    bool castShadows = true;
};

struct CameraComponent {
    float fov = 60.00f;
    float nearPlane = 0.10f;
    float farPlane = 1000.00f;
    int projectionMode = 0; // 0: Perspective, 1: Orthographic
    int priority = 0;
};

struct RigidBodyComponent {
    float mass = 1.00f;
    float linearDamping = 0.01f;
    float angularDamping = 0.05f;
    bool isKinematic = false;
    bool useGravity = true;
};

struct ColliderComponent {
    int shapeType = 0; // 0: Box, 1: Sphere, 2: Capsule, 3: Mesh
    float size[3] = { 1.00f, 1.00f, 1.00f };
    float radius = 0.50f;
    float height = 1.00f;
    bool isTrigger = false;
};

struct ScriptComponent {
    std::string scriptPath = "";
    std::string className = "";
    bool enabled = true;
};

struct AudioSourceComponent {
    std::string clipPath = "";
    float volume = 1.00f;
    float pitch = 1.00f;
    float spatialBlend = 1.00f; // 0: 2D, 1: 3D
    bool loop = false;
    bool playOnAwake = true;
};

// ============================================================================
// Type-Erased Sparse-Set Storage Interface & Template Implementation
// ============================================================================

class ISparseSet {
public:
    virtual ~ISparseSet() = default;
    virtual bool RemoveEntity(uint64_t entityId) = 0;
    virtual bool HasEntity(uint64_t entityId) const = 0;
    virtual const char* GetComponentTypeName() const = 0;
    virtual void Clear() = 0;
};

template<typename T>
class ComponentSparseSet : public ISparseSet {
public:
    ComponentSparseSet(const char* typeName) : m_TypeName(typeName) {}

    bool Add(uint64_t entityId, const T& component) {
        auto it = m_Sparse.find(entityId);
        if (it != m_Sparse.end()) {
            m_Dense[it->second] = component;
            return false; // Replaced existing
        }
        size_t index = m_Dense.size();
        m_Dense.push_back(component);
        m_Entities.push_back(entityId);
        m_Sparse[entityId] = index;
        return true;
    }

    T* Get(uint64_t entityId) {
        auto it = m_Sparse.find(entityId);
        if (it == m_Sparse.end()) return nullptr;
        return &m_Dense[it->second];
    }

    const T* Get(uint64_t entityId) const {
        auto it = m_Sparse.find(entityId);
        if (it == m_Sparse.end()) return nullptr;
        return &m_Dense[it->second];
    }

    bool RemoveEntity(uint64_t entityId) override {
        auto it = m_Sparse.find(entityId);
        if (it == m_Sparse.end()) return false;

        size_t indexToRemove = it->second;
        size_t lastIndex = m_Dense.size() - 1;

        if (indexToRemove != lastIndex) {
            uint64_t lastEntity = m_Entities[lastIndex];
            m_Dense[indexToRemove] = std::move(m_Dense[lastIndex]);
            m_Entities[indexToRemove] = lastEntity;
            m_Sparse[lastEntity] = indexToRemove;
        }

        m_Dense.pop_back();
        m_Entities.pop_back();
        m_Sparse.erase(it);
        return true;
    }

    bool HasEntity(uint64_t entityId) const override {
        return m_Sparse.find(entityId) != m_Sparse.end();
    }

    const char* GetComponentTypeName() const override {
        return m_TypeName;
    }

    void Clear() override {
        m_Dense.clear();
        m_Entities.clear();
        m_Sparse.clear();
    }

    const std::vector<uint64_t>& GetEntities() const { return m_Entities; }
    std::vector<T>& GetComponents() { return m_Dense; }
    const std::vector<T>& GetComponents() const { return m_Dense; }

private:
    const char* m_TypeName;
    std::unordered_map<uint64_t, size_t> m_Sparse;
    std::vector<uint64_t> m_Entities;
    std::vector<T> m_Dense;
};

// ============================================================================
// Component Registry Singleton
// ============================================================================

class ComponentRegistry {
public:
    static ComponentRegistry& Get();

    ComponentRegistry() = default;
    ~ComponentRegistry() = default;

    // Component Registration & Access Methods
    template<typename T>
    T* AddComponent(uint64_t entityId, const T& component = T()) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        ComponentSparseSet<T>* storage = GetOrCreateStorage<T>();
        storage->Add(entityId, component);
        return storage->Get(entityId);
    }

    template<typename T>
    T* GetComponent(uint64_t entityId) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        ComponentSparseSet<T>* storage = GetStorage<T>();
        if (!storage) return nullptr;
        return storage->Get(entityId);
    }

    template<typename T>
    const T* GetComponent(uint64_t entityId) const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        const ComponentSparseSet<T>* storage = GetStorage<T>();
        if (!storage) return nullptr;
        return storage->Get(entityId);
    }

    template<typename T>
    bool RemoveComponent(uint64_t entityId) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        ComponentSparseSet<T>* storage = GetStorage<T>();
        if (!storage) return false;
        return storage->RemoveEntity(entityId);
    }

    template<typename T>
    bool HasComponent(uint64_t entityId) const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        const ComponentSparseSet<T>* storage = GetStorage<T>();
        if (!storage) return false;
        return storage->HasEntity(entityId);
    }

    template<typename T>
    std::vector<uint64_t> GetEntitiesWith() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        const ComponentSparseSet<T>* storage = GetStorage<T>();
        if (!storage) return {};
        return storage->GetEntities();
    }

    template<typename T>
    std::vector<std::pair<uint64_t, T*>> GetComponentsWithEntity() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        ComponentSparseSet<T>* storage = GetStorage<T>();
        if (!storage) return {};

        std::vector<std::pair<uint64_t, T*>> result;
        const auto& entities = storage->GetEntities();
        auto& components = storage->GetComponents();
        result.reserve(entities.size());
        for (size_t i = 0; i < entities.size(); ++i) {
            result.emplace_back(entities[i], &components[i]);
        }
        return result;
    }

    // Dynamic Reflection & Cleanup Helpers
    std::vector<const char*> GetComponentTypeNames(uint64_t entityId) const;
    void RemoveAllComponents(uint64_t entityId);
    void Clear();

private:
    mutable std::mutex m_Mutex;
    std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> m_StorageMap;

    template<typename T>
    ComponentSparseSet<T>* GetStorage() {
        auto it = m_StorageMap.find(std::type_index(typeid(T)));
        if (it == m_StorageMap.end()) return nullptr;
        return static_cast<ComponentSparseSet<T>*>(it->second.get());
    }

    template<typename T>
    const ComponentSparseSet<T>* GetStorage() const {
        auto it = m_StorageMap.find(std::type_index(typeid(T)));
        if (it == m_StorageMap.end()) return nullptr;
        return static_cast<const ComponentSparseSet<T>*>(it->second.get());
    }

    template<typename T>
    ComponentSparseSet<T>* GetOrCreateStorage() {
        std::type_index typeIdx(typeid(T));
        auto it = m_StorageMap.find(typeIdx);
        if (it == m_StorageMap.end()) {
            auto storage = std::make_unique<ComponentSparseSet<T>>(typeid(T).name());
            ComponentSparseSet<T>* rawPtr = storage.get();
            m_StorageMap[typeIdx] = std::move(storage);
            return rawPtr;
        }
        return static_cast<ComponentSparseSet<T>*>(it->second.get());
    }
};

} // namespace EngineEditor

#endif // COMPONENT_REGISTRY_H
