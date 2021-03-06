#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <sys/ptrace.h>
#include <fcntl.h>

#include <seccomp.h> /* libseccomp */

void say_hello()
{
   printf("Can you drive with a blindfold?\n");
}

char* get_flag()
{
   char* flag = (char*)malloc(0x100);
   int i = 0;
   int fd = open("/flag", O_RDONLY);
   while (1)
   {
	  int result = read(fd, flag+i, 1);
	  if (result != 1)
	  {
		 break;
	  }
	  i += 1;
   }
   flag[i] = '\0';
   close(fd);
   return flag;
}

void shellcode_it(char* buf, unsigned int size)
{
   int rc = 0;
   scmp_filter_ctx ctx;
   char* flag = get_flag();

   void *ptr = mmap(0, size, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
   void (*shell)(char*);

   memcpy(ptr, buf, size);

   // Close all possible communication channels
   close(0);
   close(1);
   close(2);


   ctx = seccomp_init(SCMP_ACT_KILL); // default action: kill

   // setup strict whitelist
   rc += seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
   rc += seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
   rc += seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
   rc += seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);

   if (rc != 0) {
        perror("seccomp_rule_add failed");
        exit(-2);
   }

   // load the filter
   seccomp_load(ctx);
   if (rc != 0) {
        perror("seccomp_load failed");
        exit(-1);
   }

   if (getenv("TEST_SECCOMP") != NULL) {
      int filedesc = open("/dev/random", O_RDONLY );

   }

/*
   if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) != 0)
   {
	  exit(-1);
   }
*/


   shell = (void (*)(char*)) ptr;
   shell(flag);

   return;
}

void get_that_shellcode()
{
   char buf[513];
   int num;
   int i;
   printf("Send me your vehicle\n");
   num = read(0, buf, 512);
   buf[512] = '\0';
   for (i = 0; i < num; i++)
   {
	  if (buf[i] == '\0')
	  {
		 printf("Failed smog inspection.\n");
		 return;
	  }
   }

   shellcode_it(buf, num);
}


int main(int argc, char** argv)
{
   setvbuf(stdout, NULL, _IONBF, 0);

   if (getenv("DEBUG") == NULL)
   {
	  alarm(5);
   }

   say_hello();
   get_that_shellcode();
   exit(0);
}
