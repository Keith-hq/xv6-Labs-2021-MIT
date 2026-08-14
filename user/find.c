#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 提取文件名
char* getname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;
  for(p = path + strlen(path); p >= path && *p != '/'; p--);
  p++;
  memmove(buf, p, strlen(p)+1);
  return buf;
}

void find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 文件类型
  if(st.type != T_DIR){
    if(strcmp(getname(path), target) == 0){
      printf("%s\n", path);
    }
    close(fd);
    return;
  }

  // 目录，递归遍历
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0) continue;
    // 跳过 . 和 ..
    if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    find(buf, target);
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "usage: find dir filename\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}