#ifndef INCLUDE_COMPILER_WORKER_H_
#define INCLUDE_COMPILER_WORKER_H_

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

#include "simulator/BasicBlock.h"
#include "utils/macros.h"

namespace RISCV::compiler {

class Compiler;

class CompilerTask {
public:
    CompilerTask() = default;
    NO_COPY_SEMANTIC(CompilerTask);
    DEFAULT_MOVE_SEMANTIC(CompilerTask);

    BasicBlock::Body instrs;
    BasicBlock::Entrypoint entrypoint;
};

class CompilerTaskQueue {
public:
    void addTask(CompilerTask &&task);
    std::optional<CompilerTask> getTask();

    void close();
    bool isClosed();

private:
    std::deque<CompilerTask> compiler_tasks_;
    std::condition_variable is_empty_or_closed_;
    std::mutex queue_lock_;
    std::atomic<bool> is_closed_{false};
};

class CompilerWorker {
public:
    CompilerWorker(Compiler *compiler) : compiler_(compiler) {}

    void Initialize();
    void Finalize();

    void processTask();

    ALWAYS_INLINE void addTask(CompilerTask &&task) {
        task_queue_.addTask(std::move(task));
    }

private:
    Compiler *compiler_;
    std::thread worker_thread_;
    CompilerTaskQueue task_queue_;
};

}  // namespace RISCV::compiler

#endif  // INCLUDE_COMPILER_WORKER_H_
