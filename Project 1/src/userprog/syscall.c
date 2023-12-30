#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

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
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_current()->exit_code = -1;
    thread_exit();
    return;
  }
  if(pagedir_get_page(thread_current()->pagedir, vaddr) == NULL){ // given user virtual address is unmapped
    printf("%s: exit(%d)\n", thread_name(), -1);
    thread_current()->exit_code = -1;
    thread_exit();
    return;
  }
}

void
syscall_init (void) 
{
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
      printf ("%s: exit(%d)\n", thread_name(), *((uint32_t*)ptr)); // process termination message
      thread_current()->exit_code = *((uint32_t*)ptr);
      thread_exit();
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
      if(*((uint32_t*)ptr1)== 0){ // fd is 0
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
      else{
        f->eax = -1;
      }
      return;

    case SYS_WRITE :
      ptr1 = (void*)((uint32_t*)f->esp + 1);
      ptr2 = (void*)((uint32_t*)f->esp + 2);
      ptr3 = (void*)((uint32_t*)f->esp + 3);
      user_valid_addr(ptr1);
      user_valid_addr(ptr2);
      user_valid_addr(ptr3);
      if(*((uint32_t*)ptr1) == 1){
        putbuf((char *)*((uint32_t*)ptr2), (unsigned)*((uint32_t*)ptr3));
        f->eax = (uint32_t)((size_t)*((uint32_t*)ptr3));
      }
      else{
        f->eax = 0;
      }
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
  }
  // thread_exit();
}