#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <console.h>
#include "userprog/process.h"
//#include <syscall.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
//  printf ("system call!\n");
	int *arg=(int *)(f->esp);
	switch(arg[0])
	{
		case SYS_HALT:
			printf("sys_halt\n");
			break;
		case SYS_EXIT:
			exit(arg[1]);
			break;
		case SYS_EXEC:
			//printf("sys_exec\n");
			f->eax=exec((char *)arg[1]);
			break;
		case SYS_WAIT:
			printf("sys_wait\n");
			break;
		case SYS_CREATE:
			printf("sys_create\n");
			break;
		case SYS_REMOVE:
			printf("sys_remove\n");
			break;
		case SYS_OPEN:
			printf("sys_open\n");
			break;
		case SYS_FILESIZE:
			printf("sys_filesize\n");
			break;
		case SYS_READ:
			printf("sys_read\n");
			break;
		case SYS_WRITE:
	//		printf("sys_write\n");
			f->eax = write(arg[1],(void *)arg[2],(unsigned)arg[3]);
			break;
		case SYS_SEEK:
			printf("sys_seek\n");
			break;
		case SYS_TELL:
			printf("sys_tell\n");
			break;
		case SYS_CLOSE:
			printf("sys_close\n");
			break;
		default:
			printf("not yet system call!\n\n");
			break;
		
	}
 // thread_exit ();
}
void exit(int status)
{
	char *ptr;
	printf("%s: exit(%d)\n",strtok_r(thread_name()," ",&ptr),status);
	process_exit();
	thread_exit();
}
pid_t exec(const char *cmd_line)
{
	process_execute(cmd_line);
	return 0;
}
int write(int fd, const void *buffer, unsigned size)
{
	if(fd == 1)
		putbuf(buffer,size);
	return size;
}
