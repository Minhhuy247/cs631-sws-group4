/* Member 1: startup, option parsing, IPv4/IPv6 socket, daemon, concurrency, logs */
#define _POSIX_C_SOURCE 200809L
#include "sws.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
struct config conf={.port=8080};
static void usage(const char *p){fprintf(stderr,"Usage: %s [-dh] [-c dir] [-i address] [-l file] [-p port] dir\n",p);}
static void reap(int sig){(void)sig;while(waitpid(-1,NULL,WNOHANG)>0){} }
void log_request(const char *remote,const struct request *r,int code,size_t bytes){
 if(!conf.debug&&!conf.has_log)return;
 FILE *f=conf.debug?stdout:fopen(conf.log,"a");if(!f)return;
 char ts[40];time_t now=time(NULL);struct tm tm;gmtime_r(&now,&tm);strftime(ts,sizeof ts,"%Y-%m-%dT%H:%M:%SZ",&tm);
 fprintf(f,"%s %s \"%s\" %d %zu\n",remote,ts,r->line[0]?r->line:"-",code,bytes);fflush(f);if(!conf.debug)fclose(f);
}
static int listen_one(struct addrinfo *a){int s=socket(a->ai_family,a->ai_socktype,a->ai_protocol);if(s<0)return -1;int yes=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);
#ifdef IPV6_V6ONLY
 if(a->ai_family==AF_INET6){yes=1;setsockopt(s,IPPROTO_IPV6,IPV6_V6ONLY,&yes,sizeof yes);}
#endif
 if(bind(s,a->ai_addr,a->ai_addrlen)||listen(s,64)){close(s);return -1;}return s;}
static int daemonize_self(void){pid_t pid=fork();if(pid<0)return -1;if(pid>0)_exit(0);if(setsid()<0)return -1;pid=fork();if(pid<0)return -1;if(pid>0)_exit(0);int dev=open("/dev/null",O_RDWR);if(dev<0)return -1;dup2(dev,0);dup2(dev,1);dup2(dev,2);if(dev>2)close(dev);return 0;}
int main(int argc,char **argv){int opt;while((opt=getopt(argc,argv,"dhc:i:l:p:"))!=-1){switch(opt){case 'd':conf.debug=1;break;case 'h':usage(argv[0]);return 0;case 'c':conf.has_cgi=1;snprintf(conf.cgi,sizeof conf.cgi,"%s",optarg);break;case 'i':conf.has_ip=1;snprintf(conf.ip,sizeof conf.ip,"%s",optarg);break;case 'l':conf.has_log=1;snprintf(conf.log,sizeof conf.log,"%s",optarg);break;case 'p':{char *end;long x=strtol(optarg,&end,10);if(*end||x<1||x>65535){usage(argv[0]);return 1;}conf.port=(int)x;break;}default:usage(argv[0]);return 1;}}
 if(optind+1!=argc){usage(argv[0]);return 1;}if(!realpath(argv[optind],conf.root)){perror("document root");return 1;}
 if(conf.has_cgi){char tmp[PATHSZ];if(!realpath(conf.cgi,tmp)){perror("cgi dir");return 1;}strcpy(conf.cgi,tmp);}
 struct addrinfo hints,*ai,*p;memset(&hints,0,sizeof hints);hints.ai_family=AF_UNSPEC;hints.ai_socktype=SOCK_STREAM;hints.ai_flags=AI_PASSIVE;char port[16];snprintf(port,sizeof port,"%d",conf.port);
 int e=getaddrinfo(conf.has_ip?conf.ip:NULL,port,&hints,&ai);if(e){fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(e));return 1;}
 int sockets[8],n=0;for(p=ai;p&&n<8;p=p->ai_next){int s=listen_one(p);if(s>=0)sockets[n++]=s;}freeaddrinfo(ai);if(!n){perror("bind/listen");return 1;}
 signal(SIGPIPE,SIG_IGN);if(!conf.debug){if(daemonize_self()<0){perror("daemonize");return 1;}}else fprintf(stderr,"[sws] debug HTTP/1.0 port=%d root=%s\n",conf.port,conf.root);
 struct sigaction sa;memset(&sa,0,sizeof sa);sa.sa_handler=reap;sigemptyset(&sa.sa_mask);sa.sa_flags=SA_RESTART;sigaction(SIGCHLD,&sa,NULL);
 while(1){fd_set rfds;FD_ZERO(&rfds);int max=-1;for(int i=0;i<n;i++){FD_SET(sockets[i],&rfds);if(sockets[i]>max)max=sockets[i];}int r=select(max+1,&rfds,NULL,NULL,NULL);if(r<0){if(errno==EINTR)continue;break;}
 for(int i=0;i<n;i++)if(FD_ISSET(sockets[i],&rfds)){struct sockaddr_storage addr;socklen_t len=sizeof addr;int c=accept(sockets[i],(struct sockaddr*)&addr,&len);if(c<0)continue;char host[NI_MAXHOST]="unknown";getnameinfo((struct sockaddr*)&addr,len,host,sizeof host,NULL,0,NI_NUMERICHOST);
 if(conf.debug){process_client(c,host);close(c);}else{pid_t pid=fork();if(pid==0){for(int j=0;j<n;j++)close(sockets[j]);process_client(c,host);close(c);_exit(0);}close(c);}}
 }
 for(int i=0;i<n;i++)close(sockets[i]);
 return 1;
}
