#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <console.h>
#include "userprog/process.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <syscall-nr.h>
#include <user/syscall.h>
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/off_t.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
//	  printf ("system call!!\n");
	int *arg=(int *)(f->esp);
	switch(arg[0])
	{
		case SYS_HALT:
//			printf("sys_halt\n");
			shutdown_power_off();
			break;
		case SYS_EXIT:
	//		printf("sys_exit\n");
			if(is_user_vaddr(arg+1))
				exit(arg[1]);
			else
				exit(-1);
			break;
		case SYS_EXEC:
		//	printf("sys_exec\n");
			if(arg[1] != NULL && is_user_vaddr(arg[1]) && pagedir_get_page(thread_current()->pagedir,arg[1]) != NULL)
				f->eax=exec((char *)arg[1]);
			else
				exit(-1);
			break;
		case SYS_WAIT:
			//printf("sys_wait\n");
			f->eax=wait((pid_t)arg[1]);
			break;
		case SYS_CREATE:
			//printf("sys_create\n");
			if(arg[1] != NULL)
				f->eax = filesys_create(arg[1],arg[2]);
			else 
				exit(-1);
			break;
		case SYS_REMOVE:
			//printf("sys_remove\n");
			f->eax = filesys_remove(arg[1]);
			break;
		case SYS_OPEN:
			//printf("sys_open\n");
			if(arg[1] != NULL && is_user_vaddr(arg[1]) && pagedir_get_page(thread_current()->pagedir,arg[1]) != NULL)
				f->eax = open(arg[1]);
			else 
				exit(-1);
			break;
		case SYS_FILESIZE:
			//printf("sys_filesize\n");
			f->eax = filesize(arg[1]);
			break;
		case SYS_READ:
			//printf("sys_read\n");
			if(arg[2] != NULL && is_user_vaddr(arg[2]) && pagedir_get_page(thread_current()->pagedir,arg[2]) != NULL)
				f->eax = read(arg[1],(void *)arg[2],(unsigned)arg[3]);
			else 
				exit(-1);
			break;
		case SYS_WRITE:
			//printf("sys_write\n");
			if(arg[2] != NULL && is_user_vaddr(arg[2]) && pagedir_get_page(thread_current()->pagedir,arg[2]) != NULL)
				f->eax = write(arg[1],(void *)arg[2],(unsigned)arg[3]);
			else 
				exit(-1);
			break;
		case SYS_SEEK:
			//printf("sys_seek\n");
			seek(arg[1],arg[2]);
			break;
		case SYS_TELL:
			//printf("sys_tell\n");
			f->eax = tell(arg[1]);
			break;
		case SYS_CLOSE:
			//printf("sys_close\n");
			close(arg[1]);
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
	printf("%s: exit(%d)\n",strtok_r(thread_name()," ",&ptr),status );
	//pinfo_set_status(status);
	thread_current()->parent_thread->child_status = status;
	thread_exit();
}
pid_t exec(const char *cmd_line)
{
	/*
	tid_t tid;
	struct thread *t = NULL;
	struct process_info *p = NULL;
	tid = process_execute(cmd_line);
	t = find_thread(tid);
	p = find_pinfo(false,&tid);
	if(t != NULL) 
		sema_down(pinfo_sema(true,p));
	
	if(find_thread(tid) == NULL)
		return -1;
	else
		return pinfo_pid(p);
	*/
	struct thread *t = NULL;
	tid_t tid = process_execute(cmd_line);
	t=find_thread(tid);
	if(t == NULL)
		return -1;
	sema_down(&t->sema);
	if(find_thread(tid) == NULL)
		return -1;
	return (pid_t)tid;

}
int wait(pid_t pid)
{
	return process_wait((tid_t)pid);
	//tid_t tid = pinfo_tid(find_pinfo(true,pid)); 
	//return process_wait(tid);
}
int open(const char *cmd_line)
{
	struct file* f =filesys_open(cmd_line);
	if (f == NULL)
		return -1;
//	printf("file open fd = %d\n",file_get_fd(f));
	return file_get_fd(f);

}
int filesize(int fd)
{
	if(fd <= 1)
		return 0;
//	printf("file size fd = %d\n",fd);
	struct file *f = file_find_fd(fd);
	if(f==NULL)
		return 0;
	return file_length(f);
}
int read(int fd,void *buffer,unsigned size)
{
	if(fd == 0)
	{
		uint8_t *ptr = (uint8_t*)buffer;
		int i;
		for(i=0;i<size;++i)
			*(ptr++) = input_getc();
		return size;
	}
	else if(fd == 1)
		return 0;
	else
	{
		struct file *f = file_find_fd(fd);
		if(f == NULL)
			return 0;
		return file_read(f,buffer,size);
	}
}
int write(int fd, const void *buffer, unsigned size)
{
	if(fd == 1)
	{
		putbuf(buffer,size);
		return size;
	}
	else if(fd == 0)
		return 0;
	else
	{
		struct file *f = file_find_fd(fd);
		//printf("%s %s\n",thread_name(),file_get_name(f));
		if(f == NULL)
			return 0;
		if(find_thread_name(file_get_name(f)))
		{
		//	printf("bubug\n");
			file_deny_write(f);
			int retval = file_write(f,buffer,size);
			file_allow_write(f);
			return retval;
		}

		return file_write(f,buffer,size);
		
	}

}
void seek(int fd,unsigned position)
{
	if(fd <= 1)
		return ;
	struct file *f = file_find_fd(fd);
	if(f == NULL)
		return ;
	return file_seek(f,position);
}
unsigned tell(int fd)
{
	if(fd <= 1)
		return 0;
	struct file *f = file_find_fd(fd);
	if(f == NULL)
		return 0;
	return file_tell(f);
}
void close(int fd)
{
	struct file *f = file_find_fd(fd);
	if(f == NULL)
		exit(-1);
	else
		file_close(f);
}
