#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  if(argc != 5){
    printf("Invalid Usage.\n");
    printf("there should be four arguments\n");
    return EXIT_FAILURE;
  }
  else{
    int a = atoi(argv[1]), b = atoi(argv[2]), c = atoi(argv[3]), d = atoi(argv[4]);
    printf("%d %d\n", fibonacci(a), max_of_four_int(a,b,c,d));
  }
  return EXIT_SUCCESS;
}
