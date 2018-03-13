/*
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/byteorder.h>

#if defined(CONFIG_CMD_HTTPD)
#include "httpd.h"

#include "../httpd/uipopt.h"
#include "../httpd/uip.h"
#include "../httpd/uip_arp.h"
#include "gl_config.h"

static int arptimer = 0;

void HttpdHandler(void){
	int i;

	for(i = 0; i < UIP_CONNS; i++){
		uip_periodic(i);

		if(uip_len > 0){
			uip_arp_out();
			NetSendHttpd();
		}
	}

	if(++arptimer == 20){
		uip_arp_timer();
		arptimer = 0;
	}
}

// start http daemon
void HttpdStart(void){
	uip_init();
	httpd_init();
}

int do_http_upgrade( const ulong size, const int upgrade_type )
{
	char cmd[128] = {0};

	if ( upgrade_type == WEBFAILSAFE_UPGRADE_TYPE_UBOOT ) {
		printf( "\n\n****************************\n*     U-BOOT UPGRADING     *\n* DO NOT POWER OFF DEVICE! *\n****************************\n\n" );

		sprintf(cmd, "sf probe && sf erase 0x%x 0x%x && sf write 0x84000000 0x%x 0x%x", 
			CONFIG_UBOOT_START, CONFIG_UBOOT_SIZE, CONFIG_UBOOT_START, size);
		run_command(cmd, 0);
		return 0;

	} else if ( upgrade_type == WEBFAILSAFE_UPGRADE_TYPE_FIRMWARE ) {

		printf( "\n\n****************************\n*    FIRMWARE UPGRADING    *\n* DO NOT POWER OFF DEVICE! *\n****************************\n\n" );
		sprintf(cmd, "sf probe && sf erase 0x%x 0x%x && sf write 0x84000000 0x%x 0x%x",
				CONFIG_FIRMWARE_START, CONFIG_FIRMWARE_SIZE, CONFIG_FIRMWARE_START, size);
		run_command(cmd, 0);
		return 0;
		
	} else if ( upgrade_type == WEBFAILSAFE_UPGRADE_TYPE_ART ) {

		printf( "\n\n****************************\n*      ART  UPGRADING      *\n* DO NOT POWER OFF DEVICE! *\n****************************\n\n" );
		sprintf(cmd, "sf probe && sf erase 0x170000 0x10000 && sf write 0x84000000 0x170000 0x10000");
		run_command(cmd, 0);
		return 0;

	} else if ( upgrade_type == WEBFAILSAFE_UPGRADE_TYPE_QSDK_FIRMWARE ) {

		printf( "\n\n****************************\n*      FIRMWARE UPGRADING      *\n* DO NOT POWER OFF DEVICE! *\n****************************\n\n" );
		sprintf(cmd, "imgaddr=0x84000000 && source $imgaddr:script");
		run_command(cmd, 0);
		return 0;

	}else {
		return(-1);
	}
	return(-1);
}

// info about current progress of failsafe mode
int do_http_progress(const int state){
	unsigned char i = 0;

	/* toggle LED's here */
	switch(state){
		case WEBFAILSAFE_PROGRESS_START:

			// blink LED fast 10 times
			for(i = 0; i < 10; ++i){
				//all_led_on();
				udelay(25000);
				//all_led_off();
				udelay(25000);
			}

			printf("HTTP server is ready!\n\n");
			break;

		case WEBFAILSAFE_PROGRESS_TIMEOUT:
			//printf("Waiting for request...\n");
			break;

		case WEBFAILSAFE_PROGRESS_UPLOAD_READY:
			printf("HTTP upload is done! Upgrading...\n");
			break;

		case WEBFAILSAFE_PROGRESS_UPGRADE_READY:
			printf("HTTP ugrade is done! Rebooting...\n\n");
			break;

		case WEBFAILSAFE_PROGRESS_UPGRADE_FAILED:
			printf("HTTP ugrade failed!\n\n");

			// blink LED fast for 4 sec
			for(i = 0; i < 80; ++i){
				//all_led_on();
				udelay(25000);
				//all_led_off();
				udelay(25000);
			}

			// wait 1 sec
			udelay(1000000);

			break;
	}

	return(0);
}
#endif /* CONFIG_CMD_HTTPD */
