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

class SelectionChangeCommand : public ICommand {
public:
    SelectionChangeCommand(const std::vector<std::string>& oldSel, const std::vector<std::string>& newSel)
        : m_OldSelection(oldSel), m_NewSelection(newSel) {}

    void Execute() override {
        EditorState::Get().selectedNodeNames = m_NewSelection;
        EditorState::Get().selectedNodeName = m_NewSelection.empty() ? "" : m_NewSelection.back();
        EditorState::Get().RefreshActiveTransform(EditorState::Get().selectedNodeName);
    }

    void Undo() override {
        EditorState::Get().selectedNodeNames = m_OldSelection;
        EditorState::Get().selectedNodeName = m_OldSelection.empty() ? "" : m_OldSelection.back();
        EditorState::Get().RefreshActiveTransform(EditorState::Get().selectedNodeName);
    }

    const char* GetName() const override { return "Selection Change"; }

private:
    std::vector<std::string> m_OldSelection;
    std::vector<std::string> m_NewSelection;
};

class CameraMoveCommand : public ICommand {
public:
    CameraMoveCommand(const Vec3f& oldPos, float oldYaw, float oldPitch, const Vec3f& newPos, float newYaw, float newPitch)
        : m_OldPos(oldPos), m_OldYaw(oldYaw), m_OldPitch(oldPitch),
          m_NewPos(newPos), m_NewYaw(newYaw), m_NewPitch(newPitch) {}

    void Execute() override {
        auto& cam = EditorState::Get().camera;
        cam.SetPositionAndOrientation(m_NewPos, m_NewYaw, m_NewPitch);
    }

    void Undo() override {
        auto& cam = EditorState::Get().camera;
        cam.SetPositionAndOrientation(m_OldPos, m_OldYaw, m_OldPitch);
    }

    const char* GetName() const override { return "Camera Move"; }

private:
    Vec3f m_OldPos;
    float m_OldYaw, m_OldPitch;
    Vec3f m_NewPos;
    float m_NewYaw, m_NewPitch;
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
