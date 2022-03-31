#include <stdio.h>
#include <string.h>
#include "utils.h"

int main(int argc, char *argv[])
{
   char* abStack = "hello world";

   printf("dump of abStack follows:\n");
   hexdump(abStack, strlen(abStack));
   printf("checksum=%d\n",Checksum(abStack,strlen(abStack)));
}
