/* Member 2: HTTP parsing, validation, status, standard headers, conditional GET */
#define _POSIX_C_SOURCE 200809L
#include "sws.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
int send_all(int fd,const void *buf,size_t len){const char *p=buf;while(len){ssize_t n=send(fd,p,len,0);if(n<0&&errno==EINTR)continue;if(n<=0)return -1;p+=n;len-=n;}return 0;}
void http_date(time_t t,char *out,size_t len){struct tm tm;gmtime_r(&t,&tm);strftime(out,len,"%a, %d %b %Y %H:%M:%S GMT",&tm);}
static int read_line(int fd,char *out,size_t cap){size_t i=0;char c;while(i+1<cap){ssize_t n=recv(fd,&c,1,0);if(n<=0)return -1;if(c=='\n'){if(i&&out[i-1]=='\r')i--;out[i]=0;return 0;}out[i++]=c;}return -1;}
static time_t parse_date(const char *s){struct tm tm;memset(&tm,0,sizeof tm);char *end=strptime(s,"%a, %d %b %Y %H:%M:%S GMT",&tm);if(!end||*end)return (time_t)-1;return timegm(&tm);}
int parse_request(int fd,struct request *r){memset(r,0,sizeof *r);char line[BUF];if(read_line(fd,line,sizeof line)<0)return 400;snprintf(r->line,sizeof r->line,"%.*s",(int)sizeof(r->line)-1,line);
 char extra;if(sscanf(line,"%11s %2047s %23s %c",r->method,r->uri,r->version,&extra)!=3)return 400;
 if(strcmp(r->method,"GET")&&strcmp(r->method,"HEAD"))return 501;
 if(strcmp(r->version,"HTTP/1.0")&&strcmp(r->version,"HTTP/1.1"))return 505;
 if(r->uri[0]!='/'||strstr(r->uri,"://"))return 400;
 for(int i=0;i<100;i++){if(read_line(fd,line,sizeof line)<0)return 400;if(!line[0])return 0;if(!strncasecmp(line,"If-Modified-Since:",18)){const char *v=line+18;while(*v==' '||*v=='\t')v++;time_t d=parse_date(v);if(d!=(time_t)-1){r->ims=d;r->has_ims=1;}}}
 return 400;
}
static const char *reason(int code){switch(code){case 200:return "OK";case 301:return "Moved Permanently";case 304:return "Not Modified";case 400:return "Bad Request";case 403:return "Forbidden";case 404:return "Not Found";case 500:return "Internal Server Error";case 501:return "Not Implemented";case 505:return "HTTP Version Not Supported";default:return "Error";}}
int response(int fd,int code,const char *type,const void *body,size_t size,time_t mtime,int head){char h[2048],date[80],mod[80];http_date(time(NULL),date,sizeof date);if(mtime)http_date(mtime,mod,sizeof mod);
 int n=snprintf(h,sizeof h,"HTTP/1.0 %d %s\r\nDate: %s\r\nServer: sws-group4/1.0\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n",code,reason(code),date,type?type:"text/plain; charset=utf-8",size);
 if(mtime)n+=snprintf(h+n,sizeof h-(size_t)n,"Last-Modified: %s\r\n",mod);
 n+=snprintf(h+n,sizeof h-(size_t)n,"\r\n");if(send_all(fd,h,(size_t)n)<0)return -1;if(!head&&body&&size)return send_all(fd,body,size);return 0;}
int process_client(int fd,const char *remote){struct timeval tv={.tv_sec=8,.tv_usec=0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof tv);struct request r;int code=parse_request(fd,&r);int result=code;size_t bytes=0;
 if(!code){result=serve_resource(fd,&r);}
 else{char msg[128];int n=snprintf(msg,sizeof msg,"%d HTTP request error\n",code);response(fd,code,"text/plain; charset=utf-8",msg,(size_t)n,0,!strcmp(r.method,"HEAD"));bytes=(size_t)n;}
 if(result==304)bytes=0;
 log_request(remote,&r,result,bytes);return result;}
