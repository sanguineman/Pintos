#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

int fibonacci(int n);

int max_of_four_int(int a, int b, int c, int d);

void sys_exit(int code);

void sys_close(int idx);
#endif /* userprog/syscall.h */
