#ifndef SWS_H
#define SWS_H
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <time.h>
#define BUF 8192
#define PATHSZ 4096
struct config { char root[PATHSZ],cgi[PATHSZ],ip[128],log[PATHSZ]; int port,debug,has_cgi,has_ip,has_log; };
struct request {char method[12],uri[2048],version[24],line[2300];time_t ims;int has_ims;};
extern struct config conf;
int process_client(int fd,const char *remote);
int parse_request(int fd,struct request *r);
int response(int fd,int code,const char *type,const void *body,size_t size,time_t mtime,int head);
int serve_resource(int fd,const struct request *r);
int serve_cgi(int fd,const struct request *r,const char *path);
void log_request(const char *remote,const struct request *r,int code,size_t bytes);
int send_all(int fd,const void *buf,size_t len);
void http_date(time_t t,char *out,size_t len);
#endif
