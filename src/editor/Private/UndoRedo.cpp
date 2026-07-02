#include "UndoRedo.h"

UndoRedoStack& UndoRedoStack::GetInstance() {
    static UndoRedoStack instance;
    return instance;
}

void UndoRedoStack::ExecuteCommand(std::unique_ptr<UndoCommand> command) {
    command->Execute();
    m_undoStack.push_back(std::move(command));
    m_redoStack.clear();

    if (m_undoStack.size() > m_maxCommands) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

bool UndoRedoStack::CanUndo() const {
    return !m_undoStack.empty();
}

bool UndoRedoStack::CanRedo() const {
    return !m_redoStack.empty();
}

void UndoRedoStack::Undo() {
    if (m_undoStack.empty()) return;
    auto command = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    command->Undo();
    m_redoStack.push_back(std::move(command));
}

void UndoRedoStack::Redo() {
    if (m_redoStack.empty()) return;
    auto command = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    command->Execute();
    m_undoStack.push_back(std::move(command));
}

void UndoRedoStack::Clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}

std::string UndoRedoStack::GetUndoName() const {
    if (m_undoStack.empty()) return "";
    return m_undoStack.back()->GetName();
}

std::string UndoRedoStack::GetRedoName() const {
    if (m_redoStack.empty()) return "";
    return m_redoStack.back()->GetName();
}
