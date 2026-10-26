/* Member 4: CGI executable, environment, stdin/stdout pipes, header parsing */
#define _POSIX_C_SOURCE 200809L
#include "sws.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
int serve_cgi(int fd,const struct request *r,const char *path){struct stat st;if(stat(path,&st)||!S_ISREG(st.st_mode)||access(path,X_OK)){
 const char *s="CGI is not executable\n";response(fd,403,"text/plain",s,strlen(s),0,!strcmp(r->method,"HEAD"));return 403;}
 int pipefd[2];if(pipe(pipefd))return 500;pid_t pid=fork();if(pid<0){close(pipefd[0]);close(pipefd[1]);return 500;}
 if(pid==0){close(pipefd[0]);dup2(pipefd[1],STDOUT_FILENO);close(pipefd[1]);const char *query=strchr(r->uri,'?');setenv("GATEWAY_INTERFACE","CGI/1.1",1);setenv("SERVER_PROTOCOL","HTTP/1.0",1);setenv("REQUEST_METHOD",r->method,1);setenv("QUERY_STRING",query?query+1:"",1);setenv("SCRIPT_NAME",r->uri,1);setenv("SERVER_SOFTWARE","sws-group4/1.0",1);char port[16];snprintf(port,sizeof port,"%d",conf.port);setenv("SERVER_PORT",port,1);execl(path,path,(char*)NULL);_exit(127);}
 close(pipefd[1]);char output[131072];size_t used=0;ssize_t n;while(used<sizeof output&&(n=read(pipefd[0],output+used,sizeof output-used))>0)used+=(size_t)n;close(pipefd[0]);int status=0;waitpid(pid,&status,0);if(!WIFEXITED(status)||WEXITSTATUS(status)!=0){const char *msg="CGI failed\n";response(fd,500,"text/plain",msg,strlen(msg),0,!strcmp(r->method,"HEAD"));return 500;}
 size_t hdr=0;for(size_t i=0;i+1<used;i++)if(output[i]=='\n'&&output[i+1]=='\n'){hdr=i+2;break;}for(size_t i=0;i+3<used;i++)if(!memcmp(output+i,"\r\n\r\n",4)){hdr=i+4;break;}
 const char *type="text/html; charset=utf-8";char t[128];if(hdr){char *p=strstr(output,"Content-Type:");if(p&&(size_t)(p-output)<hdr){p+=13;while(*p==' ')p++;size_t j=0;while(j<sizeof t-1&&p[j]!='\r'&&p[j]!='\n'&&(size_t)(p+j-output)<hdr){t[j]=p[j];j++;}t[j]=0;type=t;}}
 response(fd,200,type,output+hdr,used-hdr,0,!strcmp(r->method,"HEAD"));return 200;}
