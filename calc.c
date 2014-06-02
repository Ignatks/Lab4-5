#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/proc_fs.h>

#define WRITE_SIZE 100

#define ARG1 "arg1"
#define ARG2 "arg2"
#define OPERATION "operation"
#define RESULT "result"

#define PARENT_DIR "calc"

struct proc_dir_entry *calc_dir;
struct proc_dir_entry *arg1;
struct proc_dir_entry *arg2;
struct proc_dir_entry *operation;
struct proc_dir_entry *result;


static char arg1_input[WRITE_SIZE];
static char arg2_input[WRITE_SIZE];
static char operation_input[WRITE_SIZE];

/*
 * arg1 write handler
 */
int write_arg1(struct file *file, const char *buf, unsigned long count, void *data)
{
	if(count > WRITE_SIZE) {
    	count = WRITE_SIZE;
	}

	//memset(arg1_input, 0, WRITE_SIZE);
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

	//memset(arg2_input, 0, WRITE_SIZE);
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

	//memset(operation, 0, WRITE_SIZE);
	memcpy(operation_input, buf, count);
	return count;
}

/*
 * result read handler
 */
int read_result(char *buffer, char **buffer_location,
                  off_t offset, int buffer_length, int *eof, void *data)
{
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

	return sprintf(buffer, "%ld\n", res);
}

int init_module()
{
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

