#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Spark {

    class Command {
    public:
        virtual ~Command() = default;
        virtual void Execute() = 0;
        virtual void Undo() = 0;
        virtual std::string GetName() const = 0;
    };

    class CommandHistory {
    public:
        static void ExecuteCommand(std::unique_ptr<Command> command) {
            command->Execute();
            s_UndoStack.push_back(std::move(command));
            s_RedoStack.clear();
        }

        static void Undo() {
            if (s_UndoStack.empty()) return;

            auto command = std::move(s_UndoStack.back());
            s_UndoStack.pop_back();
            command->Undo();
            s_RedoStack.push_back(std::move(command));
        }

        static void Redo() {
            if (s_RedoStack.empty()) return;

            auto command = std::move(s_RedoStack.back());
            s_RedoStack.pop_back();
            command->Execute();
            s_UndoStack.push_back(std::move(command));
        }

        static void Clear() {
            s_UndoStack.clear();
            s_RedoStack.clear();
        }

        static bool CanUndo() { return !s_UndoStack.empty(); }
        static bool CanRedo() { return !s_RedoStack.empty(); }

    private:
        inline static std::vector<std::unique_ptr<Command>> s_UndoStack;
        inline static std::vector<std::unique_ptr<Command>> s_RedoStack;
    };

}
