#define SYSFS


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

#ifdef SYSFS
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#else
#include <linux/proc_fs.h>
#endif

// names
#define ARG1 "arg1"
#define ARG2 "arg2"
#define OPERATION "operation"
#define RESULT "result"

#define PARENT_DIR "calc"

#define WRITE_SIZE 100

static char arg1_input[WRITE_SIZE];
static char arg2_input[WRITE_SIZE];
static char operation_input[WRITE_SIZE];

long calculate(void) {
	long a1 = 0;
	long a2 = 0;
	long res = 0;

	if (arg1_input[strlen(arg1_input) - 2] == '\n') {
		arg1_input[strlen(arg1_input) - 2] = (char)0;
	}

	kstrtol(arg1_input, 10, &a1);
	kstrtol(arg2_input, 10, &a2);

	if (operation_input[0] == '+') {
		res = a1 + a2;
	} else if (operation_input[0] == '-') {
		res = a1 - a2;
	} else if (operation_input[0] == '.') {
		res = a1 * a2;
	} else if (operation_input[0] == '/') {
		res = a1 / a2;
	}
	return res;
}

#ifdef SYSFS
/*
The attributes describe the ordinary files in the sysfs tree.
*/
// 0666 - rw for owner, group and the world
static struct attribute arg1 = {
	.name = ARG1,
	.mode = 0666,
};

static struct attribute arg2 = {
	.name = ARG2,
	.mode = 0666,
};

static struct attribute operation = {
	.name = OPERATION,
	.mode = 0666,
};

static struct attribute result = {
	.name = RESULT,
	.mode = 0666,
};

static struct attribute * calc_attributes[] = {
	&arg1,
	&arg2,
	&operation,
	&result,
	NULL
};


static ssize_t default_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	if (!strcmp(attr->name, RESULT)) {
		long res = calculate();

		return sprintf(buf, "%ld\n", res);
	} else {
		return 0;
	}
}

static ssize_t default_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
	if(len > WRITE_SIZE) {
		len = WRITE_SIZE;
	}

	if (!strcmp(attr->name, ARG1)) {
		memcpy(arg1_input, buf, len);
	} else if (!strcmp(attr->name, ARG2)) {
		memcpy(arg2_input, buf, len);
	} else if (!strcmp(attr->name, OPERATION)) {
		memcpy(operation_input, buf, len);
	}
	return len;
}

/*
The contents of these files is generated by show() and can possibly be modified by the store() function
*/
static struct sysfs_ops calc_ops = {
	.show = default_show,
	.store = default_store,
};

/*
 Such a struct represents a type of objects, and will hold the methods used to operate on them.
*/
static struct kobj_type calc_type = {
	.sysfs_ops = &calc_ops,
	.default_attrs = calc_attributes,
};

/*
A struct kobject represents a kernel object, maybe a device or so, such as the things that show up as directory in the sysfs filesystem.

The __init macro causes the init function to be discarded and its memory freed once the init function finishes for built-in drivers, but not loadable modules.
*/
//kzalloc — allocate memory. The memory is set to zero.
/*
GFP_KERNEL will try a little harder to find memory.  There's a
possibility that the call to kmalloc() will sleep while the kernel is
trying to find memory (thus making it unsuitable for interrupt

handlers).  It's much more rare for an allocation with GFP_KERNEL to
fail than with GFP_ATOMIC. GFP_ATOMIC means roughly "make the allocation operation atomic".
*/
struct kobject *calc_obj;
static int __init sysfsexample_module_init(void)
{
	int err = -1;

	calc_obj = kzalloc(sizeof(*calc_obj), GFP_KERNEL);
	if (calc_obj) {
		//kobject_init — initialize a kobject structure
		kobject_init(calc_obj, &calc_type);
		//The kobject name is set and added to the kobject hierarchy in this function.
		if (kobject_add(calc_obj, NULL, "%s", PARENT_DIR)) {
			 err = -1;
			 printk("Sysfs creation failed\n");
			 kobject_put(calc_obj);
			 calc_obj = NULL;
		}
		err = 0;
	}
	return err;
}

/*
The __exit macro causes the omission of the function when the module is built into the kernel, and like __exit, has no effect for loadable modules.
*/
static void __exit sysfsexample_module_exit(void)
{
	// kobject_put — decrement refcount for object.
	if (calc_obj) {
		kobject_put(calc_obj);
		kfree(calc_obj);
	}
}

module_init(sysfsexample_module_init);
module_exit(sysfsexample_module_exit);
/*
In kernel 2.4 and later, a mechanism was devised to identify code licensed under the GPL (and friends) so people can be warned that the code is non open-source. This is accomplished by the MODULE_LICENSE() macro which is demonstrated in the next piece of code. By setting the license to GPL, you can keep the warning from being printed. This license mechanism is defined and documented in linux/module.h:
"GPL"				[GNU Public License v2 or later]
*/
MODULE_LICENSE("GPL");


#else

struct proc_dir_entry *calc_dir;
struct proc_dir_entry *arg1;
struct proc_dir_entry *arg2;
struct proc_dir_entry *operation;
struct proc_dir_entry *result;

/*
 * arg1 write handler
 */
int write_arg1(struct file *file, const char *buf, unsigned long count, void *data)
{
	if(count > WRITE_SIZE) {
		count = WRITE_SIZE;
	}

	memcpy(arg1_input, buf, count);
	return count;
}

/*
 * arg2 write handler
 */
int write_arg2(struct file *file, const char *buf, unsigned long count, void *data)
{
	if(count > WRITE_SIZE) {
		count = WRITE_SIZE;
	}

	memcpy(arg2_input, buf, count);
	return count;
}

/*
 * operation write handler
 */
int write_operation(struct file *file, const char *buf, unsigned long count, void *data)
{
	if(count > WRITE_SIZE) {
		count = WRITE_SIZE;
	}

	memcpy(operation_input, buf, count);
	return count;
}

/*
 * result read handler
 */
int read_result(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
	long res = calculate();

	return sprintf(buffer, "%ld\n", res);
}

//KERN_INFO - info log level
int init_module()
{
	// 666 - rw for owner, group, world
	// parent dir
	calc_dir = proc_mkdir(PARENT_DIR, NULL);
	if(!calc_dir) {
		printk(KERN_INFO "Error creating proc entry");
		return -ENOMEM;
	}

	// arg1
	arg1 = create_proc_entry(ARG1, 0666, calc_dir);
	if(!arg1) {
		printk(KERN_INFO "Error creating proc entry");
		return -ENOMEM;
	}
	arg1->write_proc = write_arg1;

	// arg2
	arg2 = create_proc_entry(ARG2, 0666, calc_dir);
	if(!arg2) {
		printk(KERN_INFO "Error creating proc entry");
		return -ENOMEM;
	}
	arg2->write_proc = write_arg2;

	// operation
	operation = create_proc_entry(OPERATION, 0666, calc_dir);
	if(!operation) {
		printk(KERN_INFO "Error creating proc entry");
		return -ENOMEM;
	}
	operation->write_proc = write_operation;

	// result
	result = create_proc_entry(RESULT, 0666, calc_dir);
	if(!result) {
		printk(KERN_INFO "Error creating proc entry");
		return -ENOMEM;
	}
	result->read_proc = read_result;

	printk(KERN_INFO "/proc/%s created\n", PARENT_DIR);
	return 0;
}

void cleanup_module()
{
	remove_proc_entry(ARG1, NULL);
	remove_proc_entry(ARG2, NULL);
	remove_proc_entry(OPERATION, NULL);
	remove_proc_entry(RESULT, NULL);
	printk(KERN_INFO "/proc/%s removed\n", PARENT_DIR);
}

#endif
