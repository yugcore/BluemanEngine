#ifndef COMMAND_STACK_H
#define COMMAND_STACK_H

#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include "EditorState.h"

namespace EngineEditor {

static constexpr size_t kMaxUndoDepth = 200;

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual const char* GetName() const = 0;
};

class TransformChangeCommand : public ICommand {
public:
    TransformChangeCommand(const std::string& nodeName, uint64_t nodeId, const TransformData& oldTransform, const TransformData& newTransform);
    TransformChangeCommand(const std::string& nodeName, const TransformData& oldTransform, const TransformData& newTransform);

    void Execute() override;
    void Undo() override;
    const char* GetName() const override { return "Transform Change"; }

private:
    std::string m_NodeName;
    uint64_t m_NodeId;
    TransformData m_OldTransform;
    TransformData m_NewTransform;

    void ApplyTransformToNode(const TransformData& transform);
};

class CommandStack {
public:
    static CommandStack& Get();

    void PushAndExecute(std::shared_ptr<ICommand> command);
    bool Undo();
    bool Redo();

    bool CanUndo() const { return !m_UndoStack.empty(); }
    bool CanRedo() const { return !m_RedoStack.empty(); }
    void Clear();

private:
    std::vector<std::shared_ptr<ICommand>> m_UndoStack;
    std::vector<std::shared_ptr<ICommand>> m_RedoStack;
};

} // namespace EngineEditor

#endif // COMMAND_STACK_H
