#pragma once
#include <arpa/inet.h>

#ifndef __WRAP_H_
#define __WRAP_H_
void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
int Read(int fd, void *ptr, unsigned int nbytes);
int Write(int fd, const void *ptr, unsigned int nbytes);
int Close(int fd);
int Readn(int fd, void *vptr, unsigned int n);
int Writen(int fd, const void *vptr, unsigned int n);
//ssize_t my_read(int fd, char *ptr);
int Readline(int fd, void *vptr, unsigned int maxlen);
#endif