#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
// 프로젝트 2번 추가 헤더
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"
  
struct semaphore IO;

static void syscall_handler (struct intr_frame *);

void user_valid_addr(const void* vaddr);


int fibonacci(int n){
  if(n < 1){ // invalid input
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_current()->exit_code = -1;
    thread_exit();
    return -1;
  }
  int x = 1, y = 1, cur = 2;
  if(n == 1 || n == 2){
    return 1;
  }
  while(cur < n){
    int tmp = x + y;
    x = y;
    y = tmp;
    cur++;
  }
  return y;
}

int max_of_four_int(int a, int b, int c, int d){
  int Max = -2147483648; // int 자료형의 가장 작은 값
  if(a > Max){
    Max = a;
  }
  if(b > Max){
    Max = b;
  }
  if(c > Max){
    Max = c;
  }
  if(d > Max){
    Max = d;
  }
  return Max;
}

void 
user_valid_addr(const void* vaddr)
{
  if(is_kernel_vaddr(vaddr)){
    sys_exit(-1);
    return;
  }
  if(pagedir_get_page(thread_current()->pagedir, vaddr) == NULL){ // given user virtual address is unmapped
    sys_exit(-1);
    return; 
  }
}

void
syscall_init (void) 
{
  sema_init(&IO,1);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  // printf("syscall : %d\n",*((uint32_t*)f->esp));
  void * ptr, *ptr1,*ptr2, *ptr3, *ptr4;
  
  user_valid_addr(f->esp);
  switch(*((uint32_t*)f->esp)){
    case SYS_HALT :
      shutdown_power_off();
      return;

    case SYS_EXIT :
      ptr = (void*)((uint32_t*)f->esp + 1);
      user_valid_addr(ptr); // 접근하려는 주소가 올바른 유저 영역의 주소인지
      sys_exit(*((uint32_t*)ptr));
      // printf ("%s: exit(%d)\n", thread_name(), *((uint32_t*)ptr)); // process termination message
      // thread_current()->exit_code = *((uint32_t*)ptr);
      // thread_exit();
      return;

    case SYS_EXEC :
      ptr = (void*)((uint32_t*)f->esp + 1);
      user_valid_addr(ptr); // 접근하려는 주소가 올바른 유저 영역의 주소인지
      f->eax = process_execute((char *)*((uint32_t*)ptr));
      return;

    case SYS_WAIT :
      ptr = (void*)((uint32_t*)f->esp + 1);
      user_valid_addr(ptr);
      f->eax = process_wait((tid_t)*((uint32_t*)ptr));
      return;

    case SYS_READ :
      ptr1 = (void*)((uint32_t*)f->esp + 1);
      ptr2 = (void*)((uint32_t*)f->esp + 2);
      ptr3 = (void*)((uint32_t*)f->esp + 3);
      user_valid_addr(ptr1);
      user_valid_addr(ptr2);
      user_valid_addr(ptr3);
      user_valid_addr((char *)*((uint32_t*)ptr2));
      sema_down(&IO);
      if(*((uint32_t*)ptr1) == 0){ // fd is 0
        int len = *((uint32_t*)ptr3);
        for(int i = 0; i < len; i++){
          uint8_t c = input_getc();
          memset((char *)*((uint32_t*)ptr2), c, 1);
          if(c == '\0'){
            f->eax = i;
            break;
          }
        }
      }
      else if(*((uint32_t*)ptr1) <= 2){
        f->eax = -1;
      }
      else{
        if(thread_current()->fd[*((uint32_t*)ptr1)] == NULL){
          sema_up(&IO);
          sys_exit(-1);
        }
        f->eax = file_read(thread_current()->fd[*((uint32_t*)ptr1)], (char *)*((uint32_t*)ptr2), *((uint32_t*)ptr3));
        // file_deny_write(thread_current()->fd[*((uint32_t*)ptr1)]); // set deny_write to 1
      }
      sema_up(&IO);
      return;

    case SYS_WRITE :
      ptr1 = (void*)((uint32_t*)f->esp + 1);
      ptr2 = (void*)((uint32_t*)f->esp + 2);
      ptr3 = (void*)((uint32_t*)f->esp + 3);
      user_valid_addr(ptr1);
      user_valid_addr(ptr2);
      user_valid_addr(ptr3);
      user_valid_addr((char *)*((uint32_t*)ptr2));
      sema_down(&IO);
      if(*((uint32_t*)ptr1) == 1){
        putbuf((char *)*((uint32_t*)ptr2), (unsigned)*((uint32_t*)ptr3));
        f->eax = (uint32_t)((size_t)*((uint32_t*)ptr3));
      }
      else if(*((uint32_t*)ptr1) <= 2){
        f->eax = 0;
      }
      else{
        if(thread_current()->fd[*((uint32_t*)ptr1)] == NULL){
          sema_up(&IO);
          sys_exit(-1);
        }
        // if(thread_current()->fd[*((uint32_t*)ptr1)]->deny_write){
        //   file_deny_write(thread_current()->fd[*((uint32_t*)ptr1)]);
        // }
        f->eax = file_write(thread_current()->fd[*((uint32_t*)ptr1)], (char *)*((uint32_t*)ptr2), *((uint32_t*)ptr3));
      }
      sema_up(&IO);
      return;

    case SYS_FIBONACCI :
      ptr1 = (void*)((uint32_t*)f->esp + 1);
      user_valid_addr(ptr1);
      f->eax = fibonacci(*((uint32_t*)ptr1));
      return;

    case SYS_MAX_FOUR :
      ptr1 = (void*)((uint32_t*)f->esp + 1);
      ptr2 = (void*)((uint32_t*)f->esp + 2);
      ptr3 = (void*)((uint32_t*)f->esp + 3);
      ptr4 = (void*)((uint32_t*)f->esp + 4);
      user_valid_addr(ptr1);
      user_valid_addr(ptr2);
      user_valid_addr(ptr3);
      user_valid_addr(ptr4);
      f->eax = max_of_four_int(*((uint32_t*)ptr1),*((uint32_t*)ptr2), *((uint32_t*)ptr3), *((uint32_t*)ptr4));
      return;

      case SYS_CREATE :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        ptr2 = (void*)((uint32_t*)f->esp + 2);
        user_valid_addr(ptr1);
        user_valid_addr(ptr2);
        if((char *)*((uint32_t*)ptr1) == NULL){
          sys_exit(-1);
          return;
        }
        f->eax = filesys_create((char *)*((uint32_t*)ptr1), *((unsigned*)ptr2));
        return;
      
      case SYS_REMOVE :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        user_valid_addr(ptr1);
        if((char *)*((uint32_t*)ptr1) == NULL){
          sys_exit(-1);
          return;
        }
        f->eax = filesys_remove((char *)*((uint32_t*)ptr1));
        return;

      case SYS_OPEN :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        user_valid_addr(ptr1);
        if((char *)*((uint32_t*)ptr1) == NULL){
          sys_exit(-1);
          return;
        }
        user_valid_addr((char *)*((uint32_t*)ptr1));
        sema_down(&IO);
        struct file * fptr = filesys_open((char *)*((uint32_t*)ptr1));
        if(fptr == NULL){
          f->eax = -1;
        }
        else{
          if(strcmp(thread_name(), (const char *)*((uint32_t*)ptr1)) == 0){
            file_deny_write(fptr);
          }
          for(int i = 3; i < 128; i++){
            if(thread_current()->fd[i] == NULL){
              thread_current()->fd[i] = fptr;
              f->eax = i;
              break;
            }
          }
        }
        sema_up(&IO);
        return;
      
      case SYS_CLOSE :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        user_valid_addr(ptr1);
        sys_close(*((int*)ptr1));
        return;
      
      case SYS_FILESIZE :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        user_valid_addr(ptr1);
        if(thread_current()->fd[*((int*)ptr1)] == NULL){
          sys_exit(-1);
        }
        f->eax = file_length(thread_current()->fd[*((int*)ptr1)]);
        return;

      case SYS_SEEK :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        ptr2 = (void*)((uint32_t*)f->esp + 2);
        user_valid_addr(ptr1);
        user_valid_addr(ptr2);
        if(thread_current()->fd[*((int*)ptr1)] == NULL){
          sys_exit(-1);
        }
        file_seek(thread_current()->fd[*((int*)ptr1)], *((unsigned *)ptr2));
        return;

      case SYS_TELL :
        ptr1 = (void*)((uint32_t*)f->esp + 1);
        user_valid_addr(ptr1);
        if(thread_current()->fd[*((int*)ptr1)] == NULL){
          sys_exit(-1);
        }
        f->eax = file_tell(thread_current()->fd[*((int*)ptr1)]);
        return;


  }
  // thread_exit();
}

void sys_exit(int code){
  printf ("%s: exit(%d)\n", thread_name(), code); // process termination message
  thread_current()->exit_code = code;
  for(int i = 3; i < 128; i++){
    if(thread_current()->fd[i] != NULL){ // 파일 디스크립터 닫아주기 
      sys_close(i);
    }
  }
  struct list_elem *e;
  struct thread * cur = thread_current();
  int ret;
  /* 자식 프로세스를 process_wait() 를 통해 종료 상태를 전부 회수한다. */
  for (e = list_begin (&cur->child_threads); e != list_end (&cur->child_threads);
       e = list_next (e))
    {
      struct thread * c = list_entry (e, struct thread, threads_elem);
      ret = process_wait(c->tid);
    }
  thread_exit();
}

void sys_close(int idx){
  if(thread_current()->fd[idx] == NULL){
    sys_exit(-1);
    return;
  }
  struct file * fptr = thread_current()->fd[idx];
  thread_current()->fd[idx] = NULL;
  file_close(fptr);
}