#include "CommandStack.h"
#include "engine/scene/SceneGraph.h"
#include "engine/core/Logger.h"
#include <iostream>

namespace EngineEditor {

// --- TransformChangeCommand Implementation ---
TransformChangeCommand::TransformChangeCommand(const std::string& nodeName, uint64_t nodeId, const TransformData& oldTransform, const TransformData& newTransform)
    : m_NodeName(nodeName), m_NodeId(nodeId), m_OldTransform(oldTransform), m_NewTransform(newTransform) {
}

TransformChangeCommand::TransformChangeCommand(const std::string& nodeName, const TransformData& oldTransform, const TransformData& newTransform)
    : TransformChangeCommand(nodeName, 0, oldTransform, newTransform) {
}

void TransformChangeCommand::ApplyTransformToNode(const TransformData& transform) {
    // 1. Update the EditorState active transform (for Details panel display)
    EditorState::Get().activeTransform = transform;

    // 2. Update the actual SceneGraph node's transform data
    SceneNode* node = nullptr;
    if (m_NodeId != 0) {
        node = SceneGraph::Get().FindNodeByIdMutable(m_NodeId);
    }
    if (!node) {
        node = SceneGraph::Get().FindNodeMutable(m_NodeName);
    }
    if (node) {
        node->location[0] = transform.location[0];
        node->location[1] = transform.location[1];
        node->location[2] = transform.location[2];
        node->rotation[0] = transform.rotation[0];
        node->rotation[1] = transform.rotation[1];
        node->rotation[2] = transform.rotation[2];
        node->scale[0]    = transform.scale[0];
        node->scale[1]    = transform.scale[1];
        node->scale[2]    = transform.scale[2];
    }
}

void TransformChangeCommand::Execute() {
    ApplyTransformToNode(m_NewTransform);
    Logger::Get().Info("[CommandStack] Executed " + std::string(GetName()) + " on " + m_NodeName);
}

void TransformChangeCommand::Undo() {
    ApplyTransformToNode(m_OldTransform);
    Logger::Get().Info("[CommandStack] Undid " + std::string(GetName()) + " on " + m_NodeName);
}

// --- CommandStack Implementation ---
CommandStack& CommandStack::Get() {
    static CommandStack instance;
    return instance;
}

void CommandStack::PushAndExecute(std::shared_ptr<ICommand> command) {
    if (!command) return;
    command->Execute();
    m_UndoStack.push_back(command);
    m_RedoStack.clear();

    // Enforce max undo depth to bound memory usage
    if (m_UndoStack.size() > kMaxUndoDepth) {
        m_UndoStack.erase(m_UndoStack.begin());
    }
}

bool CommandStack::Undo() {
    if (m_UndoStack.empty()) {
        Logger::Get().Warning("[CommandStack] Nothing to undo.");
        return false;
    }
    auto command = m_UndoStack.back();
    m_UndoStack.pop_back();
    command->Undo();
    m_RedoStack.push_back(command);
    return true;
}

bool CommandStack::Redo() {
    if (m_RedoStack.empty()) {
        Logger::Get().Warning("[CommandStack] Nothing to redo.");
        return false;
    }
    auto command = m_RedoStack.back();
    m_RedoStack.pop_back();
    command->Execute();
    m_UndoStack.push_back(command);
    return true;
}

void CommandStack::Clear() {
    m_UndoStack.clear();
    m_RedoStack.clear();
}

} // namespace EngineEditor
