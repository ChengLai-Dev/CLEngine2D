#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

class Node;

class UndoCommand {
public:
    virtual ~UndoCommand() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual std::string GetName() const { return "Command"; }
};

class PropertyChangeCommand : public UndoCommand {
public:
    using ApplyFunc = std::function<void()>;

    PropertyChangeCommand(const std::string& desc, ApplyFunc execute, ApplyFunc undo)
        : m_description(desc), m_execute(std::move(execute)), m_undo(std::move(undo)) {}

    void Execute() override { m_execute(); }
    void Undo() override { m_undo(); }
    std::string GetName() const override { return m_description; }

private:
    std::string m_description;
    ApplyFunc m_execute;
    ApplyFunc m_undo;
};

class UndoRedoStack {
public:
    static UndoRedoStack& GetInstance();

    void ExecuteCommand(std::unique_ptr<UndoCommand> command);
    bool CanUndo() const;
    bool CanRedo() const;
    void Undo();
    void Redo();
    void Clear();

    std::string GetUndoName() const;
    std::string GetRedoName() const;

private:
    UndoRedoStack() = default;
    UndoRedoStack(const UndoRedoStack&) = delete;
    UndoRedoStack& operator=(const UndoRedoStack&) = delete;

    std::vector<std::unique_ptr<UndoCommand>> m_undoStack;
    std::vector<std::unique_ptr<UndoCommand>> m_redoStack;
    size_t m_maxCommands = 50;
};
