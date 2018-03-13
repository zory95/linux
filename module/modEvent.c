#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include "modEvent.h"


#define DRV_NAME					 "modEvent"

struct cdev cdev;
struct class* class;
struct device* class_dev;
dev_t dev;
pid_t pid;
struct eventUser buffUserEvent[MAX_SIZE];
int buffUserEventPos;
char buffUserEventFull;

long ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_param){
	int err,aux;
	struct timespec time;
	struct task_struct* task;
	switch (ioctl_num) {
		case GET_SIZE:	//tamaño del buffer. Si pid == -1 devuelve el tamaño del buffer de usuario

			if(pid == -1){
				aux = buffUserEventFull ? MAX_SIZE : buffUserEventPos;
				err = copy_to_user((int*)ioctl_param,&aux,sizeof(int));
				if (err != 0) {
					 printk(KERN_ALERT DRV_NAME " : failed to copy the size of the user buffer.\n");
					 return err;
				}
			} else{
				task = pid_task(find_vpid(pid), PIDTYPE_PID);

				aux = task->fullEvent ? MAX_SIZE : task->posEvent;
				err = copy_to_user((int*)ioctl_param,&aux,sizeof(int));
				if (err != 0) {
					 printk(KERN_ALERT DRV_NAME " : failed to copy the size.\n");
					 return err;
				}
			}

		break;
		case SET_PID:	//copiar pid del usuario para manejar el buffer correspondiente. pid == -1 es el buffer de usuario

			err = copy_from_user(&pid,(pid_t*)ioctl_param,sizeof(pid_t));
			if (err != 0) {
				 printk(KERN_ALERT DRV_NAME " : failed to copy the pid.\n");
				 return -1;
			}

		break;
		case EXEC_EVENT:	//guardar evento de usuario

			buffUserEvent[buffUserEventPos].pid = current->pid;
			err = copy_from_user(buffUserEvent[buffUserEventPos].type,(char *)ioctl_param,19);
			if (err != 0) {
				 printk(KERN_ALERT DRV_NAME " : failed to copy the name of the event.\n");
				 return -1;
			}
			buffUserEvent[buffUserEventPos].type[19]='\0';

			getnstimeofday(&time);
			buffUserEvent[buffUserEventPos].sec = time.tv_sec;
			buffUserEvent[buffUserEventPos].nsec = time.tv_nsec;

			buffUserEventPos++;
			if(buffUserEventPos >= MAX_SIZE){
				buffUserEventPos = 0;
				buffUserEventFull = 1;
			}
		break;
		default:
			printk(KERN_ALERT DRV_NAME " : invalid parameter of ioctl %d\n",ioctl_num);
		break;
	}
	return 0;
}

ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp){
	size_t aux;
	unsigned long err;
	struct task_struct* task;
	if(pid == -1){	//si pid == -1 copiar buffer de usuario

		aux = buffUserEventFull ? MAX_SIZE : buffUserEventPos;
		aux = count > aux ? aux : count;

		err = copy_to_user(buff,buffUserEvent,aux * sizeof(struct eventUser));
		if (err != 0) {
			 printk(KERN_ALERT DRV_NAME " : failed to copy the result of the user buffer.\n");
			 return err;
		}
		return aux;
	} else{	//si no copiar el buffer del proceso del pid

		task = pid_task(find_vpid(pid), PIDTYPE_PID);

		aux = task->fullEvent ? MAX_SIZE : task->posEvent;
		aux = count > aux ? aux : count;

		err = copy_to_user(buff,task->buffEvent,aux * sizeof(struct event));
		if (err != 0) {
			 printk(KERN_ALERT DRV_NAME " : failed to copy the result.\n");
			 return err;
		}
		return aux;
	}
}

int open(struct inode *inode, struct file *filp){
	return 0;
}

int release(struct inode *inode, struct file *filp){
	return 0;
}

struct file_operations fops = {
 read:	read,
 open:	 open,
 unlocked_ioctl:	 ioctl,
 release: release
};

int __init
mod_event_init_module(void)
{
	 	int err;
		cdev_init(&cdev,&fops);
		err = alloc_chrdev_region(&dev , 0, 1,"modEvent");
		if (err < 0) {
			printk(KERN_INFO "Major number allocation is failed\n");
			return err;
		}

		err = cdev_add(&cdev,dev,1);
		if(err < 0 ){
			printk(KERN_INFO "Unable to allocate cdev");
			return err;
		}
		class = class_create(THIS_MODULE,"events");
		class_dev = device_create(class,NULL,dev,NULL,"modEvent");

		buffUserEventPos = 0;
		buffUserEventFull = 0;

	 	return 0;
}

void __exit
mod_event_exit_module(void)
{
	 device_destroy(class,dev);
	 class_destroy(class);
	 cdev_del(&cdev);
	 unregister_chrdev_region(dev, 1);
}

module_init(mod_event_init_module);
module_exit(mod_event_exit_module);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("kernel module to mesure process time in cpu.");
MODULE_VERSION("1.0");
