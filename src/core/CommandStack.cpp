#include "CommandStack.h"
#include "Logger.h"
#include <iostream>

namespace EngineEditor {

// --- TransformChangeCommand Implementation ---
TransformChangeCommand::TransformChangeCommand(const std::string& nodeName, const TransformData& oldTransform, const TransformData& newTransform)
    : m_NodeName(nodeName), m_OldTransform(oldTransform), m_NewTransform(newTransform) {
}

void TransformChangeCommand::Execute() {
    EditorState::Get().activeTransform = m_NewTransform;
    Logger::Get().Info("[CommandStack] Executed " + std::string(GetName()) + " on " + m_NodeName);
}

void TransformChangeCommand::Undo() {
    EditorState::Get().activeTransform = m_OldTransform;
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
