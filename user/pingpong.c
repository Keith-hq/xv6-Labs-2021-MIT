#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
  int p[2];
  pipe(p);
  char buf[10];

  int pid = fork();
  if(pid == 0){
    // 子进程：读管道
    read(p[0], buf, 1);
    printf("%d: received ping\n", getpid());
    write(p[1], "a", 1);
    close(p[0]);
    close(p[1]);
    exit(0);
  } else {
    // 父进程：写ping，读pong
    write(p[1], "a", 1);
    wait(0);
    read(p[0], buf, 1);
    printf("%d: received pong\n", getpid());
    close(p[0]);
    close(p[1]);
    exit(0);
  }
}