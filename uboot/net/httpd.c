/*
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/byteorder.h>
#include "httpd.h"

#include "../httpd/uipopt.h"
#include "../httpd/uip.h"
#include "../httpd/uip_arp.h"
#include "gl_config.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
char *str = "hello";
//#include <gpio.h>
//#include <spi_api.h>

static int arptimer = 0;
static int  HttpdTimeoutCountMax = 3;
static int HttpdTimeoutCount = 0;
static ulong HttpdTimeoutMSecs = 1000;
extern int	webfailsafe_is_running;

void HttpdHandler( void )
{
	int i;
	
	if ( uip_len == 0 ) {
		for ( i = 0; i < UIP_CONNS; i++ ) {
			uip_periodic( i );
			if ( uip_len > 0 ) {
				uip_arp_out();
				NetSendHttpd();
			}
		}

		if ( ++arptimer == 20 ) {
			uip_arp_timer();
			arptimer = 0;
		}
	} else {
		//printf("uip_len = %d\n", uip_len);
		if ( BUF->type == htons( UIP_ETHTYPE_IP ) ) {
			uip_arp_ipin();
			uip_input();
			if ( uip_len > 0 ) {
				uip_arp_out();
				NetSendHttpd();
			}
		} else if ( BUF->type == htons( UIP_ETHTYPE_ARP ) ) {
			uip_arp_arpin();
			if ( uip_len > 0 ) {
				NetSendHttpd();
			}
		}
	}

}

// start http daemon
void HttpdStart( void )
{
	uip_init();
	httpd_init();
}

int do_http_upgrade( const ulong size, const int upgrade_type )
{
	char cmd[128] = {0};
	int encrypt_update = 1;//enalbe
#ifdef CONFIG_RSA
	const char *s = getenv("rsa");
	printf("s = %s\n", s);
	if ( s != NULL ) {
		if (strcmp(s, "disable") == 0) {
			encrypt_update = 0;
		} else {
			encrypt_update = 1;
		}
	}
#endif

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
int do_http_progress( const int state )
{
	unsigned char i = 0;

	/* toggle LED's here */
	switch ( state ) {
		case WEBFAILSAFE_PROGRESS_START:

			// blink LED fast 10 times
			for ( i = 0; i < 10; ++i ) {
				/* LEDON(); */
				udelay( 25000 );
				/* LEDOFF(); */
				udelay( 25000 );
			}

			printf( "HTTP server is ready!\n\n" );
			break;

		case WEBFAILSAFE_PROGRESS_TIMEOUT:
			//printf("Waiting for request...\n");
			break;

		case WEBFAILSAFE_PROGRESS_UPLOAD_READY:

			// blink LED fast 10 times
			for ( i = 0; i < 10; ++i ) {
				/* LEDON(); */
				udelay( 25000 );
				/* LEDOFF(); */
				udelay( 25000 );
			}
			printf( "HTTP upload is done! Upgrading...\n" );
			break;

		case WEBFAILSAFE_PROGRESS_UPGRADE_READY:

			// blink LED fast 10 times
			for ( i = 0; i < 10; ++i ) {
				/* LEDON(); */
				udelay( 25000 );
				/* LEDOFF(); */
				udelay( 25000 );
			}
			printf( "HTTP ugrade is done! Rebooting...\n\n" );
			break;

		case WEBFAILSAFE_PROGRESS_UPGRADE_FAILED:
			printf( "## Error: HTTP ugrade failed!\n\n" );

			// blink LED fast for 4 sec
			for ( i = 0; i < 80; ++i ) {
				/* LEDON(); */
				udelay( 25000 );
				/* LEDOFF(); */
				udelay( 25000 );
			}

			// wait 1 sec
			udelay( 1000000 );

			break;
	}

	return( 0 );
}
