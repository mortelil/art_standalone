/* SPDX-License-Identifier: Apache-2.0 */
#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int main(int argc,char **argv) {
 assert(argc==2);
 int saved=dup(STDERR_FILENO);assert(saved>=0);
 char path[]="/tmp/atl-log-XXXXXX";int target=mkstemp(path);assert(target>=0);unlink(path);
 assert(dup2(target,STDERR_FILENO)==STDERR_FILENO);
 void *lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);assert(lib);
 int (*logprint)(int,const char*,const char*,...)=dlsym(lib,"__android_log_print");assert(logprint);
 struct stat expected;assert(!fstat(target,&expected));
 int output=-1;
 for(int fd=3;fd<256;fd++){struct stat actual;
  if(fd!=target&&!fstat(fd,&actual)&&actual.st_dev==expected.st_dev&&actual.st_ino==expected.st_ino){assert(output==-1);output=fd;}
 }
 assert(output>=0 && (fcntl(output,F_GETFD)&FD_CLOEXEC));
 int redirected[2];assert(!pipe2(redirected,O_NONBLOCK|O_CLOEXEC));
 assert(dup2(redirected[1],STDERR_FILENO)==STDERR_FILENO);
 logprint(4,"ATLLogTest","preserved-output");
 char buffer[1024];assert(read(redirected[0],buffer,sizeof(buffer))==-1&&errno==EAGAIN);
 assert(lseek(target,0,SEEK_SET)==0);ssize_t n=read(target,buffer,sizeof(buffer));assert(n>0);
 assert(memmem(buffer,(size_t)n,"preserved-output",16));
 assert(dup2(saved,STDERR_FILENO)==STDERR_FILENO);close(saved);
 dlclose(lib);
 /* musl retains dlopened objects until process exit; dlclose is not an
  * unload/descriptor-cleanup boundary on the target platform. */
 close(target);close(redirected[0]);close(redirected[1]);
 puts("PASS: log output survives stderr redirection, descriptor is close-on-exec");
}
