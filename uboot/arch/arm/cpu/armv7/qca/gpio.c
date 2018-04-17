/*
 *
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm/arch-ipq40xx/iomap.h>
#include <asm/arch-qcom-common/gpio.h>
#include <asm/io.h>

#include <asm/mach-types.h>
#include "ipq40xx_cdp.h"


/*******************************************************
Function description: configure GPIO functinality
Arguments :
unsigned int gpio - Gpio number
unsigned int func - Functionality number
unsigned int dir  - direction 0- i/p, 1- o/p
unsigned int pull - pull up/down, no pull range(0-3)
unsigned int drvstr - range (0 - 7)-> (2- 16)MA steps of 2
unsigned int oe - 0 - Disable, 1- Enable.

Return : None
*******************************************************/


void gpio_tlmm_config(unsigned int gpio, unsigned int func,
		unsigned int out, unsigned int pull,
		unsigned int drvstr, unsigned int oe,
		unsigned int gpio_vm, unsigned int gpio_od_en,
		unsigned int gpio_pu_res)
{
	unsigned int val = 0;
	val |= pull;
	val |= func << 2;
	val |= drvstr << 6;
	val |= oe << 9;
	val |= gpio_vm << 11;
	val |= gpio_od_en << 12;
	val |= gpio_pu_res << 13;

	unsigned int *addr = (unsigned int *)GPIO_CONFIG_ADDR(gpio);
	writel(val, addr);

	/* Output value is only relevant if GPIO has been configured for fixed
	 * output setting - i.e. func == 0 */
	if (func == 0) {
		addr = (unsigned int *)GPIO_IN_OUT_ADDR(gpio);
		val = readl(addr);
		val |= out << 1;
		writel(val, addr);
	}

	return;
}

void gpio_set_value(unsigned int gpio, unsigned int out)
{
	unsigned int *addr = (unsigned int *)GPIO_IN_OUT_ADDR(gpio);
	unsigned int val = 0;

	val = readl(addr);
	val &= ~(0x2);
	val |= out << 1;
	writel(val, addr);
}

extern board_ipq40xx_params_t *gboard_param;
void all_led_on()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(2));
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(3));
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(4));
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(52));//wifi
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(49));//mesh
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void all_led_off()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(2));//wifi
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(3));//mesh
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(4));//power
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(52));//wifi
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(49));//mesh
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void mesh_led_on()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(2));
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(3));
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(4));
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(52));//wifi
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(49));//mesh
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void mesh_led_off()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(2));//wifi
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(3));//mesh
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(4));//power
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(52));//wifi
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(49));//mesh
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void wifi_led_on()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(2));
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(3));
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(4));
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(52));//wifi
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(49));//mesh
		//writel(GPIO_OUT, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void wifi_led_off()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(2));//wifi
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(3));//mesh
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(4));//power
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		writel(GPIO_IN, GPIO_IN_OUT_ADDR(52));//wifi
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(49));//mesh
		//writel(GPIO_IN, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void download_led_twinkle()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	unsigned int val;
		
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		if ( readl(GPIO_IN_OUT_ADDR(2)) != 0 ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(2));//wifi
		}

		val = readl(GPIO_IN_OUT_ADDR(3));
		if ( val ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(3));//mesh
		} else {
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(3));
		}
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		if ( readl(GPIO_IN_OUT_ADDR(52)) != 0 ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(52));//wifi
		}

		val = readl(GPIO_IN_OUT_ADDR(49));
		if ( val ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(49));
		} else {
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(49));
		}
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void erase_flash_led_twinkle()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	unsigned int val;
	
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		if ( readl(GPIO_IN_OUT_ADDR(3)) != 0 ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(3));//mesh
		}

		val = readl(GPIO_IN_OUT_ADDR(2));
		if ( val ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(2));//wifi
		} else {
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(2));//wifi
		}
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		if ( readl(GPIO_IN_OUT_ADDR(49)) != 0 ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(49));//mesh
		}

		val = readl(GPIO_IN_OUT_ADDR(52));
		if ( val ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(52));//wifi
		} else {
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(52));//wifi
		}
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

void write_flash_led_twinkle()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	unsigned int val;
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		val = readl(GPIO_IN_OUT_ADDR(2));
		if ( val ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(3));
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(2));//wifi
		} else {
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(2));//wifi
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(3));
		}
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		val = readl(GPIO_IN_OUT_ADDR(52));
		if ( val ) {
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(49));
			writel(GPIO_IN, GPIO_IN_OUT_ADDR(52));//wifi
		} else {
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(52));//wifi
			writel(GPIO_OUT, GPIO_IN_OUT_ADDR(49));
		}
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}


void power_led_on()
{
	u32 status;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(4));
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		writel(GPIO_OUT, GPIO_IN_OUT_ADDR(48));//power
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}
}

u32 get_gpio_status()
{
	u32 status = -1;
	/*gpio_func_data_t *gpio;
	gpio = gboard_param->sw_gpio;
	if (gpio) {
		qca_configure_gpio(gpio, gboard_param->sw_gpio_count);
	}*/
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK01_1_S1:
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
	case MACH_TYPE_IPQ40XX_AP_DK05_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		status = readl(GPIO_IN_OUT_ADDR(63)); 
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C4:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C5:
		status = readl(GPIO_IN_OUT_ADDR(18)); 
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK04_1_C2:
		break;
	 case MACH_TYPE_IPQ40XX_AP_DK06_1_C1:
		break;
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C1:
	case MACH_TYPE_IPQ40XX_AP_DK07_1_C2:
		break;
	case MACH_TYPE_IPQ40XX_DB_DK01_1_C1:
	case MACH_TYPE_IPQ40XX_DB_DK02_1_C1:
		break;
	default:
		break;
	}

	return status;
}

