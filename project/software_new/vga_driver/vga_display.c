/* * Device driver for the VGA video generator
 *
 * A Platform device implemented using the misc subsystem
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 *
 * Check code style with
 * checkpatch.pl --file --no-tree vga_ball.c
 */


/* Name/UNI:
 *    Daniel Mesko / dpm2153
 *    Cansu Cabuk / cc4455
 *    Alan Armero / aa3938 
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "vga_display.h"

#define DRIVER_NAME "vga_display"


// device registers
#define P1_X(x) (x)
#define P1_Y(x) (x+1)
#define P2_X(x) (x+2)
#define P2_Y(x) (x+3)
#define P1_HEALTH(x) ((x)+4)
#define P2_HEALTH(x) ((x)+4)


/*
 * Information about our device
 */
struct vga_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
	vga_display_arg_t arg;
} dev;

void place_players(vga_display_arg_t *arg)
{
	iowrite8(arg->p1_x, P1_X(dev.virtbase));
	iowrite8(arg->p1_y, P1_Y(dev.virtbase) );
	iowrite8(arg->p2_x, P2_X(dev.virtbase) );
	iowrite8(arg->p2_y, P2_Y(dev.virtbase) );
	iowrite8(arg->p1_health, P1_HEALTH(dev.virtbase) );
	iowrite8(arg->p2_health, P2_HEALTH(dev.virtbase) );
	dev.arg = *arg;

}



/*
 * Handle ioctl() calls from userspace:
 * Read or write the segments on single digits.
 * Note extensive error checking of arguments
 */
static long vga_display_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	vga_display_arg_t vla;

	switch (cmd) {
	case VGA_DISPLAY_PLACE_PLAYERS:
		if (copy_from_user(&vla, (vga_display_arg_t *) arg,
				   sizeof(vga_display_arg_t)))
			return -EACCES;
		place_players(&vla);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations vga_display_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = vga_display_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice vga_display_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &vga_display_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init vga_display_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/vga_ball */
	ret = misc_register(&vga_display_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
		ret = -EBUSY;
		goto out_deregister;
	}

	/* Arrange access to our registers */
	dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}

	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&vga_display_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int vga_display_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&vga_display_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id vga_display_of_match[] = {
	{ .compatible = "csee4840,vga_display-1.0" },
	{},
};
MODULE_DEVICE_TABLE(of, vga_display_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver vga_display_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(vga_display_of_match),
	},
	.remove	= __exit_p(vga_display_remove),
};

/* Called when the module is loaded: set things up */
static int __init vga_display_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&vga_display_driver, vga_display_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit vga_display_exit(void)
{
	platform_driver_unregister(&vga_display_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(vga_display_init);
module_exit(vga_display_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stephen A. Edwards, Columbia University");
MODULE_DESCRIPTION("VGA display driver");
