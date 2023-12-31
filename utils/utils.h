#ifndef UTILS_H
#define UTILS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace RISCV::utils {

bool startHostCount(int *fd, long long instr_flag) {
    perf_event_attr pe = {};

    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(perf_event_attr);
    pe.config = instr_flag;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    *fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (*fd == -1) {
        return false;
    }
    ioctl(*fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(*fd, PERF_EVENT_IOC_ENABLE, 0);
    return true;
}

bool endHostCount(int *fd, size_t *config_val) {
    ioctl(*fd, PERF_EVENT_IOC_DISABLE, 0);
    int rd = read(*fd, config_val, sizeof(size_t));
    bool result = true;
    if (rd == -1) {
        result = false;
    }
    close(*fd);
    return result;
}

}  // namespace RISCV::utils

#endif  // UTILS_H
