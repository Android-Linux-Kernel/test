/*
	luminjie@wind-mobi.com 20160119 begin
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_typedef.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>
//luminjie@wind-mobi.com 20160322 begin
#include <mach/gpio_const.h>
#include <mt-plat/mt_gpio.h>
//luminjie@wind-mobi.com 20160322 end
/******************************************************************************
 * Debug configuration
******************************************************************************/
/* availible parameter */
/* ANDROID_LOG_ASSERT */
/* ANDROID_LOG_ERROR */
/* ANDROID_LOG_WARNING */
/* ANDROID_LOG_INFO */
/* ANDROID_LOG_DEBUG */
/* ANDROID_LOG_VERBOSE */

#define TAG_NAME "[leds_strobe.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

/*#define DEBUG_LEDS_STROBE*/
#ifdef DEBUG_LEDS_STROBE
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif
//luminjie@wind-mobi.com 20170331 begin
#ifdef wind_flashlight_mode
static int hduration=-1;
static int lduration=-1;
extern int flashlight_set_pwm(u32 hduration, u32 lduration, u32 level);
extern int flashlight_set_pwm_old(u32 hduration, u32 lduration, u32 level);
#endif
//luminjie@wind-mobi.com 20170331 end
/******************************************************************************
 * local variables
******************************************************************************/

static DEFINE_SPINLOCK(g_strobeSMPLock);	/* cotta-- SMP proection */


static u32 strobe_Res;
static u32 strobe_Timeus;
static BOOL g_strobe_On;

static int g_duty = -1;
static int g_timeOutTimeMs;

static DEFINE_MUTEX(g_strobeSem);


#define STROBE_DEVICE_ID 0xC6
//luminjie@wind-mobi.com 20170207 begin
//extern int mtkcam_gpio_set(int PinIdx, int PwrType, int Val);
static struct work_struct workTimeOut;

/* #define FLASH_GPIO_ENF GPIO12 */
/* #define FLASH_GPIO_ENT GPIO13 */
//luminjie@wind-mobi.com modify the flashlight mode 20161206 begin
static int FL_Enable(void)
{
//luminje@wind-mobi.com 20170331 begin
#ifdef wind_flashlight_mode
	int level =0;
#endif
//luminje@wind-mobi.com 20170331 end
	printk("FL_enable g_duty=%d \n",g_duty);
//luminjie@wind-mobi.com 20160418 begin
	if(g_duty < 1){
//luminjie@wind-mobi.com 20160418 end
		g_duty=0;
	}
	if(g_duty==0){
	//luminjie@wind-mobi.com 20160506 begin
	//flash_gpio_set(FLASHLIGHT_BACK, 1);
	flash_gpio_back_high();
	//luminjie@wind-mobi.com 20160506 end
	}
	//luminjie@wind-mobi.com 20170331 begin
	#ifdef wind_flashlight_mode
	else if (g_duty >= 1 && g_duty <= 9){
	
	printk("enable flash mode \n");
	//flash_gpio_set(FLASHLIGHT_backpwm, 0);
	flash_gpio_backpwm_low();
	
	//mtkcam_gpio_set(0, 5, 1);
	mdelay(10);
	//luminjie@wind-mobi.com 20170109 begin
	mt_set_gpio_mode(GPIO1 | 0x80000000, GPIO_MODE_01);
	//luminjie@wind-mobi.com 20170109 end
	mt_set_gpio_dir(GPIO1 | 0x80000000,1);

	switch(g_duty){
		case 0:
			hduration = 9;
			lduration = 89;
			level = 10;
			break;
		case 1:
			hduration = 9;
			lduration = 89;
			level = 10;
			break;
		case 2:
			hduration = 19;
			lduration = 79;
			level = 20;
			break;
		case 3:
			hduration = 29;
			lduration = 69;
			level = 30;
			break;
		case 4:
			hduration = 39;
			lduration = 59;
			level = 40;
			break;
		case 5:
			hduration = 49;
			lduration = 49;
			level = 50;
			break;
		case 6:
			hduration = 59;
			lduration = 39;
			level = 60;
			break;
		case 7:
			hduration = 69;
			lduration = 29;
			level = 70;
			break;
		case 8:
			hduration = 79;
			lduration = 19;
			level = 80;
			break;
		case 9:
			hduration = 89;
			lduration = 9;
			level = 90;
			break;
		case 10:
			hduration = 97;
			lduration = 1;
			level = 99;
			break;
		default:
			hduration = 97;
			lduration = 1;
			level = 98;
			break;
		}
	flashlight_set_pwm_old(hduration,lduration,level);
	//luminjie@wind-mobi.com 20170105 begin
	mdelay(6);
	//luminjie@wind-mobi.com 20170105 end
	//flash_gpio_set(FLASHLIGHT_backpwm, 1);
	flash_gpio_backpwm_high();
	}
	#endif
	//luminje@wind-mobi.com 201770331 end
	else{
		//flash_gpio_set(FLASHLIGHT_backpwm, 0);
		flash_gpio_backpwm_low();
		//flash_gpio_set(FLASHLIGHT_BACK, 1);
		flash_gpio_back_high();
		mdelay(6);
		//flash_gpio_set(FLASHLIGHT_backpwm, 1);
		flash_gpio_backpwm_high();
	}
	PK_DBG(" FL_Enable line=%d\n", __LINE__);
	return 0;
}
//luminjie@wind-mobi.com modify the flashlight mode 20161206 end
static int FL_Disable(void)
{
	//flash_gpio_set(FLASHLIGHT_backpwm, 0);
	flash_gpio_backpwm_low();
	
	mt_set_gpio_mode(GPIO1 | 0x80000000, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO1 | 0x80000000,1);
	//flash_gpio_set(FLASHLIGHT_BACK, 0);
	flash_gpio_back_low();
	
	PK_DBG(" FL_Disable line=%d\n", __LINE__);
	return 0;
}

static int FL_dim_duty(kal_uint32 duty)
{
	PK_DBG(" FL_dim_duty line=%d\n", __LINE__);
	g_duty = duty;
	return 0;
}

static int FL_Init(void)
{
	//flash_gpio_set(FLASHLIGHT_BACK, 0);
	flash_gpio_back_low();
	//flash_gpio_set(FLASHLIGHT_backpwm, 0);
	flash_gpio_backpwm_low();
	PK_DBG(" FL_Init line=%d\n", __LINE__); 
	return 0;
}
//luminjie@wind-mobi.com 20170207 end
static int FL_Uninit(void)
{
	FL_Disable();
	return 0;
}

/*****************************************************************************
User interface
*****************************************************************************/

static void work_timeOutFunc(struct work_struct *data)
{
	FL_Disable();
	PK_DBG("ledTimeOut_callback\n");
}

static enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
	schedule_work(&workTimeOut);
	return HRTIMER_NORESTART;
}

static struct hrtimer g_timeOutTimer;
static void timerInit(void)
{
	INIT_WORK(&workTimeOut, work_timeOutFunc);
	g_timeOutTimeMs = 1000;
	hrtimer_init(&g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	g_timeOutTimer.function = ledTimeOutCallback;
}

static int constant_flashlight_ioctl(unsigned int cmd, unsigned long arg)
{
	int i4RetValue = 0;
	int ior_shift;
	int iow_shift;
	int iowr_shift;

	ior_shift = cmd - (_IOR(FLASHLIGHT_MAGIC, 0, int));
	iow_shift = cmd - (_IOW(FLASHLIGHT_MAGIC, 0, int));
	iowr_shift = cmd - (_IOWR(FLASHLIGHT_MAGIC, 0, int));
/*	PK_DBG
	    ("LM3642 constant_flashlight_ioctl() line=%d ior_shift=%d, iow_shift=%d iowr_shift=%d arg=%d\n",
	     __LINE__, ior_shift, iow_shift, iowr_shift, (int)arg);
*/
	switch (cmd) {

	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %d\n", (int)arg);
		g_timeOutTimeMs = arg;
		break;


	case FLASH_IOC_SET_DUTY:
		PK_DBG("FLASHLIGHT_DUTY: %d\n", (int)arg);
		FL_dim_duty(arg);
		break;


	case FLASH_IOC_SET_STEP:
		PK_DBG("FLASH_IOC_SET_STEP: %d\n", (int)arg);

		break;

	case FLASH_IOC_SET_ONOFF:
		PK_DBG("FLASHLIGHT_ONOFF: %d\n", (int)arg);
		if (arg == 1) {

			int s;
			int ms;

			if (g_timeOutTimeMs > 1000) {
				s = g_timeOutTimeMs / 1000;
				ms = g_timeOutTimeMs - s * 1000;
			} else {
				s = 0;
				ms = g_timeOutTimeMs;
			}

			if (g_timeOutTimeMs != 0) {
				ktime_t ktime;

				ktime = ktime_set(s, ms * 1000000);
				hrtimer_start(&g_timeOutTimer, ktime, HRTIMER_MODE_REL);
			}
			FL_Enable();
		} else {
			FL_Disable();
			hrtimer_cancel(&g_timeOutTimer);
		}
		break;
	default:
		PK_DBG(" No such command\n");
		i4RetValue = -EPERM;
		break;
	}
	return i4RetValue;
}




static int constant_flashlight_open(void *pArg)
{
	int i4RetValue = 0;

	PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

	if (0 == strobe_Res) {
		FL_Init();
		timerInit();
	}
	PK_DBG("constant_flashlight_open line=%d\n", __LINE__);
	spin_lock_irq(&g_strobeSMPLock);


	if (strobe_Res) {
		PK_DBG(" busy!\n");
		i4RetValue = -EBUSY;
	} else {
		strobe_Res += 1;
	}


	spin_unlock_irq(&g_strobeSMPLock);
	PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

	return i4RetValue;

}


static int constant_flashlight_release(void *pArg)
{
	PK_DBG(" constant_flashlight_release\n");

	if (strobe_Res) {
		spin_lock_irq(&g_strobeSMPLock);

		strobe_Res = 0;
		strobe_Timeus = 0;

		/* LED On Status */
		g_strobe_On = FALSE;

		spin_unlock_irq(&g_strobeSMPLock);

		FL_Uninit();
	}

	PK_DBG(" Done\n");

	return 0;

}


FLASHLIGHT_FUNCTION_STRUCT constantFlashlightFunc = {
	constant_flashlight_open,
	constant_flashlight_release,
	constant_flashlight_ioctl
};


MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
	if (pfFunc != NULL)
		*pfFunc = &constantFlashlightFunc;
	return 0;
}



/* LED flash control for high current capture mode*/
ssize_t strobe_VDIrq(void)
{

	return 0;
}
EXPORT_SYMBOL(strobe_VDIrq);
/*
	luminjie@wind-mobi.com 20160119 end
*/