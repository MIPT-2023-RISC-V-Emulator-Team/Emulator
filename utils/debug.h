#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

namespace RISCV::debug {
[[noreturn]] void AssertionFail(const char *expr, const char *file, unsigned line, const char *function);
}  // namespace RISCV::debug

#endif  // INCLUDE_DEBUG_H
