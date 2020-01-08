/* å‚è€ƒ/drivers/clk/qcom/clock_gpio.c */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/delay.h>
 #include <linux/device.h>

#include <linux/gpio.h>
#include <mach/gpio.h>

#include <linux/io.h> //add by andy

/* ÒÆÔ¶¿ª·¢°åsmart_evb v1.3ÓĞÈı¸öledµÆ¿É¹©¿ØÖÆ
 *  status 	  d808 GPIO_0
 *  NET_MODE	  d805 GPIO_99	
 *  NET_LIGHT  d807 GPIO_58
 */

/* ÔİÊ±ÏÈ²âÊÔd808 */
static u32 gpio_nr = 0>>4;
//gpio_nr = 99>>4;
//gpio_nr = 58>>4;

static void __iomem *base_gpio;


/*   ÅäÖÃGPIO
  *  uint32_t gpio     ÅäÖÃÄÄ¸öGPIO
  *  uint8_t func       ÅäÖÃÒı½Å¹¦ÄÜ¸´ÓÃ»òGPIO
  *  uint8_t pull        ÅäÖÃÉÏÀ­»òÏÂÀ­µç×è
  *  uint8_t drvstr     ÅäÖÃGPIOµÄÇı¶¯ÄÜÁ¦£¬ÒÔµçÁ÷Îªµ¥Î»
  *  uint32_t enable  Ê¹ÄÜGPOI
  */
static void gpio_config (uint32_t gpio, uint8_t func, uint8_t pull, uint8_t drvstr, uint32_t enable)
{
    uint32_t val = 0;

    val |= pull;
    val |= func << 2;
    val |= drvstr << 6;
    val |= enable << 9;

    writel_relaxed(val, base_gpio + (gpio) * 0x1000);
}

static int led_open(struct inode *inode,  struct file *file)
{
	printk(KERN_EMERG"led_open\n");
	
	/* ÅäÖÃIOÎªÆÕÍ¨IO¿Ú/Êä³ö/ÏÂÀ­/8MAÀ­µçÁ÷*/
	gpio_config(gpio_nr, 0, 1, 3, 1);

	/* ÉèÖÃÊä³öÎª0 */
   	writel_relaxed(0, base_gpio + 0x00000004 + gpio_nr * 0x1000);
	
	return 0;
}

/* app: ioctl(fd,cmd,arg) */

static long led_ioctl(struct file *filp, unsigned int cmd,unsigned long arg)
{
	/* ¸ù¾İ´«ÈëµÄ²ÎÊıÉèÖÃGPIOµÄµçÆ½ */
	/* cmd:0-off, 1-on    */
	/* arg:0-3,whitch led */

	printk(KERN_EMERG"led_ioctl\n");
	if((cmd != 0 ) && (cmd != 1))
	{
		printk(KERN_EMERG"cmd err %d\n",cmd);
		return -EINVAL;
	}
	if(arg > 4)
	{
		printk(KERN_EMERG"arg err %d\n",(int)arg);
		return -EINVAL;
	}

	printk(KERN_EMERG"cmd is %d, arg is %d\n",cmd,(int)arg);
	/* ÉèÖÃIO¿ÚµçÆ½ */
	writel_relaxed((cmd << 1), base_gpio + 0x00000004 + gpio_nr * 0x1000);


	return 0;
}

static struct file_operations led_ops = {
	.owner = THIS_MODULE, /* ÕâÊÇÒ»¸öºê£¬ÍÆÏò±àÒëÄ£¿éÊ±×Ô¶¯´´½¨*/
	.open = led_open,
	.unlocked_ioctl	= led_ioctl,
};


static int major;
static struct class *cls;

int led_init(void)
{
	printk("led_init\n");
	
	major = register_chrdev(0, "led1", &led_ops);

	if(!major)
	{
		printk("register_chrdev led err\n");
		return -2;
	}
	else
	{
		printk("leds major is %d\n",major);
	}
	
	/* ÎªÁËÈÃÏµÍ³udev.mdev¸øÎÒÃÇ´´½¨Éè±¸½Úµã */
	
	/* ´´½¨Àà£¬ÔÚÀàÏÂ´´½¨Éè±¸:/sys */
	cls = class_create(THIS_MODULE, "led1");
	
	device_create(cls, NULL, MKDEV(major,0), NULL, "led1"); /* /dev/led*/

	base_gpio = ioremap(0x1000000, 0x300000);
	if(!base_gpio)
	{
		printk("ioremap error!!!\n");
		return -1;
	}
	return 0;
}

void led_exit(void)
{
	device_destroy(cls, MKDEV(major,0));
	class_destroy(cls);

	unregister_chrdev(major, "led1");

	iounmap(base_gpio);
}

module_init(led_init);
//late_initcall(leds_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO output led Driver");


