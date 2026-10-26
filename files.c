/* Member 3: docroot, index.html, alphabetical directory listing, user homes, MIME */
#define _POSIX_C_SOURCE 200809L
#include "sws.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <limits.h>
static int prefix(const char *p,const char *root){size_t n=strlen(root);return !strncmp(p,root,n)&&(p[n]=='/'||p[n]=='\0');}
static int bad(int fd,int code,const struct request *r){char body[100];int n=snprintf(body,sizeof body,"%d - request could not be served\n",code);response(fd,code,"text/plain; charset=utf-8",body,(size_t)n,0,!strcmp(r->method,"HEAD"));return code;}
static const char *mime(const char *p){const char *s=strrchr(p,'.');if(!s)return "application/octet-stream";if(!strcmp(s,".html")||!strcmp(s,".htm"))return "text/html; charset=utf-8";if(!strcmp(s,".css"))return "text/css";if(!strcmp(s,".js"))return "text/javascript";if(!strcmp(s,".txt"))return "text/plain; charset=utf-8";if(!strcmp(s,".json"))return "application/json";if(!strcmp(s,".png"))return "image/png";if(!strcmp(s,".jpg")||!strcmp(s,".jpeg"))return "image/jpeg";if(!strcmp(s,".svg"))return "image/svg+xml";return "application/octet-stream";}
static int cmp(const void *a,const void *b){return strcmp(*(const char *const*)a,*(const char *const*)b);}
static int listdir(int fd,const char *path,const struct request *r){DIR *d=opendir(path);if(!d)return bad(fd,403,r);char *names[1024];size_t count=0;struct dirent *e;while((e=readdir(d))&&count<1024){if(e->d_name[0]=='.')continue;names[count++]=strdup(e->d_name);}closedir(d);qsort(names,count,sizeof *names,cmp);char *body=calloc(1,131072);if(!body)return bad(fd,500,r);size_t pos=(size_t)snprintf(body,131072,"<!doctype html><meta charset='utf-8'><title>Directory</title><h1>Directory listing</h1><ul>");for(size_t i=0;i<count;i++){if(pos+strlen(names[i])*6+100<131072)pos+=(size_t)snprintf(body+pos,131072-pos,"<li><a href=\"%s\">%s</a></li>",names[i],names[i]);free(names[i]);}pos+=(size_t)snprintf(body+pos,131072-pos,"</ul>");response(fd,200,"text/html; charset=utf-8",body,pos,0,!strcmp(r->method,"HEAD"));free(body);return 200;}
int serve_resource(int fd,const struct request *r){int ishead=!strcmp(r->method,"HEAD");char uri[2048];snprintf(uri,sizeof uri,"%s",r->uri);char *q=strchr(uri,'?');if(q)*q=0;
 if(strchr(uri,'%')||strchr(uri,'\\')||strstr(uri,"/../")||strstr(uri,"/./")||strstr(uri,"//")||strstr(uri,"/..")||strstr(uri,"/."))return bad(fd,403,r);
 if(conf.has_cgi&&!strncmp(uri,"/cgi-bin/",9)){char path[PATHSZ],real[PATHSZ];if(snprintf(path,sizeof path,"%s/%s",conf.cgi,uri+9)>=(int)sizeof path||!realpath(path,real)||!prefix(real,conf.cgi))return bad(fd,404,r);return serve_cgi(fd,r,real);}
 char root[PATHSZ],path[PATHSZ],real[PATHSZ];snprintf(root,sizeof root,"%s",conf.root);const char *suffix=uri;
 if(uri[0]=='/'&&uri[1]=='~'){char *slash=strchr(uri+2,'/');char user[128];size_t n=slash?(size_t)(slash-(uri+2)):strlen(uri+2);if(!n||n>=sizeof user)return bad(fd,404,r);memcpy(user,uri+2,n);user[n]=0;struct passwd *pw=getpwnam(user);if(!pw)return bad(fd,404,r);if(snprintf(root,sizeof root,"%s/sws",pw->pw_dir)>=(int)sizeof root)return bad(fd,404,r);suffix=slash?slash:"/";}
 char canonicalroot[PATHSZ];if(!realpath(root,canonicalroot))return bad(fd,404,r);
 if(snprintf(path,sizeof path,"%s%s",canonicalroot,suffix)>=(int)sizeof path)return bad(fd,414,r);
 if(!realpath(path,real))return bad(fd,404,r);
 if(!prefix(real,canonicalroot))return bad(fd,403,r);
 struct stat st;if(stat(real,&st)<0)return bad(fd,404,r);
 if(S_ISDIR(st.st_mode)){char index[PATHSZ];if(snprintf(index,sizeof index,"%s/index.html",real)>=(int)sizeof index)return bad(fd,500,r);struct stat ix;if(stat(index,&ix)==0&&S_ISREG(ix.st_mode)){strcpy(real,index);st=ix;}else return listdir(fd,real,r);}
 if(!S_ISREG(st.st_mode))return bad(fd,403,r);
 if(r->has_ims&&st.st_mtime<=r->ims){response(fd,304,mime(real),NULL,0,st.st_mtime,ishead);return 304;}
 FILE *f=fopen(real,"rb");if(!f)return bad(fd,403,r);size_t sz=(size_t)st.st_size;char *buf=malloc(sz?sz:1);if(!buf){fclose(f);return bad(fd,500,r);}size_t nr=fread(buf,1,sz,f);fclose(f);if(nr!=sz){free(buf);return bad(fd,500,r);}response(fd,200,mime(real),buf,sz,st.st_mtime,ishead);free(buf);return 200;
}
