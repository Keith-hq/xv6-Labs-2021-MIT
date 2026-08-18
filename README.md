# xv6-Labs-2021-MIT

MIT 6.S081（Operating System Engineering）2021 课程实验仓库。基于 xv6（RISC-V）教学操作系统完成全部 10 个实验，内容覆盖用户程序、系统调用、页表、中断、写时复制、多线程、网络驱动、锁、文件系统与 mmap 等操作系统核心机制。每个实验一个独立分支，均可独立编译运行。

## 实验列表

| 实验 | 分支 | 内容 |
|------|------|------|
| Lab 1 | [lab1-util](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab1-util) | Unix 实用工具：sleep、pingpong、primes、find、xargs |
| Lab 2 | [lab2-syscall](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab2-syscall) | 系统调用：trace、sysinfo、进程系统调用追踪 |
| Lab 3 | [lab3-pgtbl](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab3-pgtbl) | 页表：打印页表、内核页表重构、加速系统调用 |
| Lab 4 | [lab4-traps](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab4-traps) | 中断与陷阱：RISC-V 汇编、backtrace、alarm |
| Lab 5 | [lab5-cow](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab5-cow) | 写时复制（Copy-on-Write）fork 实现 |
| Lab 6 | [lab6-thread](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab6-thread) | 多线程：用户态线程切换、pthread 实现、barrier |
| Lab 7 | [lab7-net](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab7-net) | 网络驱动：E1000 网卡驱动与 UDP/IP 协议栈 |
| Lab 8 | [lab8-lock](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab8-lock) | 锁：内存分配器并行化、缓冲缓存并发优化 |
| Lab 9 | [lab9-fs](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab9-fs) | 文件系统：大文件、符号链接、inode 与日志 |
| Lab 10 | [lab10-mmap](https://github.com/Keith-hq/xv6-Labs-2021-MIT/tree/lab10-mmap) | mmap：内存映射文件与 munmap 实现 |

## 仓库结构

- `master`：本仓库默认分支，仅含 README 与仓库说明
- `lab1-util` ~ `lab10-mmap`：每个实验一个分支，各含完整 xv6 源码与对应实验的实现

## 构建与运行

xv6 基于 RISC-V 架构，需要 RISC-V 交叉编译工具链与 QEMU：

```bash
git checkout lab1-util      # 切换到目标实验分支
make qemu                   # 编译并启动 xv6
```

各分支自带 `grade-lab-*` 评分脚本，可运行 `make grade` 检查实验通过情况。

## 参考

- [MIT 6.S081: Operating System Engineering](https://pdos.csail.mit.edu/6.828/2021/schedule.html)
- [xv6: a simple, Unix-like teaching operating system](https://pdos.csail.mit.edu/6.828/2021/xv6/book-riscv-rev2.pdf)
