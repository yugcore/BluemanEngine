#ifndef COMMAND_STACK_H
#define COMMAND_STACK_H

#include <memory>
#include <vector>
#include <string>
#include "EditorState.h"

namespace EngineEditor {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual const char* GetName() const = 0;
};

class TransformChangeCommand : public ICommand {
public:
    TransformChangeCommand(const std::string& nodeName, const TransformData& oldTransform, const TransformData& newTransform);

    void Execute() override;
    void Undo() override;
    const char* GetName() const override { return "Transform Change"; }

private:
    std::string m_NodeName;
    TransformData m_OldTransform;
    TransformData m_NewTransform;
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
