#include "compiler/CompilerWorker.h"

#include "compiler/Compiler.h"

namespace RISCV::compiler {

void CompilerTaskQueue::addTask(CompilerTask&& task) {
    bool is_empty = false;
    {
        std::unique_lock holder(queue_lock_);
        is_empty = compiler_tasks_.empty();
        compiler_tasks_.emplace_back(std::move(task));
    }
    if (is_empty) {
        is_empty_or_closed_.notify_one();
    }
}

std::optional<CompilerTask> CompilerTaskQueue::getTask() {
    std::unique_lock holder(queue_lock_);
    while (compiler_tasks_.empty() && !isClosed()) {
        is_empty_or_closed_.wait(holder);
    }

    if (isClosed()) {
        return std::nullopt;
    }

    auto task = std::move(compiler_tasks_.front());
    compiler_tasks_.pop_front();
    return task;
}

void CompilerTaskQueue::close() {
    is_closed_.store(true, std::memory_order_release);
    is_empty_or_closed_.notify_one();
}

bool CompilerTaskQueue::isClosed() {
    return is_closed_.load(std::memory_order_acquire);
}

void CompilerWorker::Initialize() {
    worker_thread_ = std::thread([this] {
        while (!task_queue_.isClosed()) {
            processTask();
        }
    });
}

void CompilerWorker::Finalize() {
    task_queue_.close();
    worker_thread_.join();
}

void CompilerWorker::processTask() {
    auto task = task_queue_.getTask();
    if (task == std::nullopt) {
        return;
    }
    compiler_->compileBasicBlock(std::move(*task));
}

}  // namespace RISCV::compiler
