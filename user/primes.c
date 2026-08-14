#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void filter(int left_fd)
{
  int num;
  int r = read(left_fd, &num, sizeof(int));
  if(r <= 0){
    close(left_fd);
    return;
  }
  // 第一个数是素数
  printf("prime %d\n", num);

  int p[2];
  pipe(p);
  int pid = fork();
  if(pid == 0){
    // 子进程继续过滤
    close(left_fd);
    close(p[1]);
    filter(p[0]);
  } else {
    // 父进程筛选倍数，写入下一级管道
    close(p[0]);
    int n;
    while(read(left_fd, &n, sizeof(int)) > 0){
      if(n % num != 0){
        write(p[1], &n, sizeof(int));
      }
    }
    close(left_fd);
    close(p[1]);
    wait(0);
  }
}

int main(void)
{
  int p[2];
  pipe(p);
  int pid = fork();
  if(pid == 0){
    close(p[1]);
    filter(p[0]);
  } else {
    close(p[0]);
    // 向管道写入2~35
    for(int i = 2; i <= 35; i++){
      write(p[1], &i, sizeof(int));
    }
    close(p[1]);
    wait(0);
  }
  exit(0);
}