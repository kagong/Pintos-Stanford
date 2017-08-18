#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
typedef int pid_t;
void syscall_init (void);
void exit(int status);
pid_t exec(const char *cmd_line);
int write(int fd, const void *buffer, unsigned size);

#endif /* userprog/syscall.h */
