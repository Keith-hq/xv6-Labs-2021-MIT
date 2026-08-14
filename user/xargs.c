#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXARG 16
#define MAXBUF 128

int main(int argc, char *argv[])
{
  char buf[MAXBUF];
  char *args[MAXARG];
  int arg_cnt = 0;

  // 先复制命令行参数
  for(int i = 1; i < argc; i++){
    args[arg_cnt++] = argv[i];
  }

  // 循环读取stdin每行
  while(1){
    int pos = 0;
    // 读取一行
    while(pos < MAXBUF-1 && read(0, buf+pos, 1) == 1 && buf[pos] != '\n'){
      pos++;
    }
    if(pos == 0) break; // 读到文件末尾

    buf[pos] = 0;
    args[arg_cnt] = buf;
    args[arg_cnt+1] = 0;

    int pid = fork();
    if(pid == 0){
      exec(args[0], args);
      exit(1);
    } else {
      wait(0);
    }
  }
  exit(0);
}