
/*************************************************************************
        > File Name: gl_ipq40xx_cmd.c
        > Author: Lancer
        > Mail: luoyejiang0701@qq.com
        > Created Time: 2017��08��22�� ���ڶ� 14ʱ55��45��
 ************************************************************************/


#include <common.h>
#include <gl_config.h>
#include "gl/gl_ipq40xx_api.h"
#include <mmc.h>
#include "ipq40xx_cdp.h"

#define BUFFERSIZE	2048


extern board_ipq40xx_params_t *gboard_param;


/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

extern int TftpdownloadStatus;

int auto_update_flags = 0;
int g_run_sf_probe = 1;

static int atoi(const char *s)
{
	int i = 0;

	while(is_digit(*s)){
		i = i * 10 + *(s++) - '0';
	}

	return(i);
}

unsigned char a2x(const char c)
{
	switch(c) {
		case '0'...'9':
			return (unsigned char)atoi(&c);
		case 'a'...'f':
			return 0xa + (c-'a');
		case 'A'...'F':
			return 0xa + (c-'A');
		default:
			return -1;
	}
}

unsigned long hex2int(char *a, unsigned int len)
{
    int i;
    unsigned long val = 0;

    for(i=0;i<len;i++)
       if(a[i] <= 57)
        val += (a[i]-48)*(1<<(4*(len-1-i)));
       else
        val += (a[i]-55)*(1<<(4*(len-1-i)));

    return val;
}

/* convert a string of length 18 to a macaddress data type. */
#define MAC_LEN_IN_BYTE 6     
#define COPY_STR2MAC(mac, str)  \
	do { \
		int i; \
		for (i = 0; i < MAC_LEN_IN_BYTE; i++) { \
			mac[i] = (a2x(str[i*2]) << 4) + a2x(str[i*2 + 1]); \
		} \
	} while(0)  

void do_run_usage()
{
	printf("Usage:\n");
	printf("run lu:\n"
	"    - download %s from your tftp server and replace the current one\n"
	"run lf:\n"
	"    - download %s from your tftp server and write on flash\n"
	"run lfq:\n"
	"    - download %s from your tftp server and write on flash\n\n",
	uboot_name, openwrt_fw_name, qsdk_fw_name);
}

void do_flash_usage()
{
	printf("Usage: flash read/write, only for debug\n");
	printf(
	"flash r <eth0/eth1/ath0/ath1/ath2>         - read mac address from the flash\n"
	"flash w <eth0/eth1/ath0/ath1/ath2> <data>  - write mac address\n"
	"\t eg: flash w eth0 e4956e123456\n\n");
}

/*
static void print_hex(char *name, unsigned char *data, int len)
{	
	int i;	
	printf("\n%s:\t", name);	
	for (i=1; i<len+1; i++) {		
		printf("%02X ", data[i-1]);		
		if (i%24 == 0) {			
			printf("\n\t\t");		
		}	
	}	printf("\n");
}
*/

/*
static void hexify( unsigned char *obuf, const unsigned char *ibuf, int len )
{    
	unsigned char l, h;    
	while( len != 0 ) {       
		h = *ibuf / 16;        
		l = *ibuf % 16;        
		if( h < 10 )            
			*obuf++ = '0' + h;       
		else          
			*obuf++ = 'a' + h - 10;      
		if( l < 10 )          
			*obuf++ = '0' + l;       
		else            
			*obuf++ = 'a' + l - 10;      
		++ibuf;        
		len--;    
	}
}
*/

#if 0

#define RSA_E   "10001"

#if 0
int rsa_verify()
{
	unsigned int sig_len = 0, fw_len;
	unsigned char buf[POLARSSL_MPI_MAX_SIZE];
	unsigned char RSA_N[256];
	char cmd[128] = {0};
	rsa_context rsa;
	unsigned char sha1sum[20];
	int ret;

	volatile unsigned char *tmp = (volatile unsigned char *)0x88000000;
	volatile unsigned short *key_len = (volatile unsigned short *)0x88000000;

	sprintf(cmd, "sf probe && sf read 0x88000000 0x%x 4", (CONFIG_ART_START + 0x1000));
	run_command(cmd, 0);

	memset(RSA_N, 0x00, sizeof(RSA_N));
	hexify(RSA_N, tmp, 128);
	RSA_N[256] = '\0';
	printf("RSA_N=%s\n", RSA_N);
	
	rsa_init( &rsa, RSA_PKCS_V15, 0 );
	mpi_read_string( &rsa.N , 16, RSA_N  );
    mpi_read_string( &rsa.E , 16, RSA_E  );
	rsa.len = 128;

	if( rsa_check_pubkey(  &rsa ) != 0 ) {
        printf( "failed\n" );
        return( 1 );
    }

	/*download firmware*/
	if (run_command("tftpboot 0x88000000 config-gl-b1300.bin", 0) != GL_OK) return 1;
	fw_len = NetBootFileXferSize;
	sha1( tmp, fw_len, sha1sum );
	print_hex("sha1sum", sha1sum, 20);

	/*download firmware.sign*/
	if (run_command("tftpboot 0x88000000 config-gl-b1300.bin.sig.bin", 0) != GL_OK) return 1;
	sig_len = NetBootFileXferSize;

	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, tmp, 128);
	print_hex("buf", buf, 128);

	printf("sig_len = %08x, fw_len = %08x\n", sig_len, fw_len);

	
    if( ( ret = rsa_pkcs1_verify( &rsa, NULL, NULL, RSA_PUBLIC,
                                  POLARSSL_MD_SHA1, 20, sha1sum, buf ) ) != 0 ) {
        printf( " \n  ! rsa_pkcs1_verify returned -0x%0x\n\n", -ret );
		return 1;
    }

	printf("verify pass!\n");
	rsa_free( &rsa );
	
	return 0;
}
#else
int rsa_verify(char *filename, int use_http)
{
	unsigned int sig_len = 0, fw_len;
	unsigned char buf[POLARSSL_MPI_MAX_SIZE];
	char *RSA_N = NULL;
	char cmd[128] = {0};
	rsa_context rsa;
	unsigned char sha1sum[20];
	int ret;
	unsigned short rsa_key_len, aes_key_len;
	//char key[32] = "01234567891234560123456789123456";
	//char iv[16] = "9876543210654321";
	char dec_out[BUFFERSIZE] = {0};	
	char *pout = dec_out;
	int len;
	char aes_key[32];
	char aes_key_iv[16];

	volatile unsigned char *tmp = (volatile unsigned char *)0x88000000;
	volatile unsigned short *p_rsa_key_len = (volatile unsigned short *)0x88000000;
	volatile unsigned char *rsa_key = (volatile unsigned char *)0x88000002;

	volatile unsigned short *p_aes_key_len = (volatile unsigned short *)(0x88000000 + 0x800);
	volatile unsigned char *p_aes_key = (volatile unsigned char *)(0x88000000 + 0x802);

	sprintf(cmd, "sf probe && sf read 0x88000000 0x%x 0x1000", CONFIG_RSA_PUBKEY_START);
	run_command(cmd, 0);

	rsa_key_len = htons(*p_rsa_key_len);
	aes_key_len = htons(*p_aes_key_len);

	if ( aes_key_len > 32 ) {
		printf("aes key lenght error.\n");
		return 1;
	}
	strncpy(aes_key, p_aes_key, aes_key_len);
	aes_key[aes_key_len] = '\0';
	p_aes_key = p_aes_key + aes_key_len; //get key_iv
	strncpy(aes_key_iv, p_aes_key, 16);
	aes_key_iv[16] = '\0';
	
	printf("aes_key_len = %x, %d, %s, %s\n", aes_key_len, aes_key_len, aes_key, aes_key_iv);

	len = device_aes_decrypt(aes_key, aes_key_len, aes_key_iv, rsa_key, rsa_key_len, pout, BUFFERSIZE);
	
	//len = device_aes_decrypt(aes_key, aes_key_len, iv, rsa_key, rsa_key_len, pout, BUFFERSIZE);
	//printf("len = %d, KEY=%s\n", len, pout);

	RSA_N = (char *)malloc( len + 1);
	memset(RSA_N, 0x00, len + 1);
	strncpy(RSA_N, pout, len);
	//printf("RSA_N=%s\n", RSA_N);
	
	rsa_init( &rsa, RSA_PKCS_V15, 0 );
	mpi_read_string( &rsa.N , 16, RSA_N  );
    mpi_read_string( &rsa.E , 16, RSA_E  );
	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;

	if( rsa_check_pubkey(  &rsa ) != 0 ) {
        printf( "failed\n" );
        goto exit;
    }

	/*download firmware.sign*/
	sprintf(cmd, "tftpboot 0x88000000 %s.sig", filename);
	if (run_command(cmd, 0) != GL_OK) 
		return 1;
	sig_len = NetBootFileXferSize;
	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, tmp, sig_len);
	//print_hex("buf", buf, sig_len);


	/*download firmware*/
	sprintf(cmd, "tftpboot 0x88000000 %s.bin", filename);
	if (run_command(cmd, 0) != GL_OK) 
		return 1;
	fw_len = NetBootFileXferSize;
	sha1( tmp, fw_len, sha1sum );
	//print_hex("sha1sum", sha1sum, 20);

	
    if( ( ret = rsa_pkcs1_verify( &rsa, NULL, NULL, RSA_PUBLIC,
                                  POLARSSL_MD_SHA1, 20, sha1sum, buf ) ) != 0 ) {
        printf( " \n  ! rsa_pkcs1_verify returned -0x%0x\n\n", -ret );
		goto exit;
    }

	printf("verify pass!\n");
	free(RSA_N);
	rsa_free( &rsa );

	return 0;
exit:
	free(RSA_N);
	rsa_free( &rsa );
	return 1;
}

#endif
#endif


/*
int write_rsa_pubkey()
{
	int i;
	volatile unsigned char *tmp = (volatile unsigned char *)0x88000000;
	volatile unsigned char *key = (volatile unsigned char *)0x84010000;

	memset(key, 0xFF, CONFIG_RSA_PUBKEY_MAX_SIZE);

	if (NetBootFileXferSize > CONFIG_RSA_PUBKEY_MAX_SIZE)
		return 1;
	
	for (i=0; i<NetBootFileXferSize; i++) {
		*key++ = *tmp++;
	}

	return 0;
}

int write_aes_key()
{
	int i;
	volatile unsigned char *tmp = (volatile unsigned char *)0x88000000;
	volatile unsigned char *key = (volatile unsigned char *)(0x84010000 + CONFIG_RSA_PUBKEY_MAX_SIZE);

	memset(key, 0xFF, CONFIG_AES_KEY_MAX_SIZE);

	if (NetBootFileXferSize > CONFIG_AES_KEY_MAX_SIZE)
		return 1;
	
	for (i=0; i<NetBootFileXferSize; i++) {
		*key++ = *tmp++;
	}

	return 0;
}
*/

/**
*0 burning qsdk firmware. 1 burning lede firmware. 2. emmc DOS/MBR image
*/
int do_checkout_firmware(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile unsigned char *F = (volatile unsigned char *)0x8800005c;
	volatile unsigned char *l = (volatile unsigned char *)0x8800005d;
	volatile unsigned char *a = (volatile unsigned char *)0x8800005e;
	volatile unsigned char *s = (volatile unsigned char *)0x8800005f;
	volatile unsigned char *h = (volatile unsigned char *)0x88000060;

	volatile unsigned char *m1_55 = (volatile unsigned char *)0x880001fe;
	volatile unsigned char *m2_aa = (volatile unsigned char *)0x880001ff;

	if (*F==0x46 && *l==0x6c && *a==0x61 && *s==0x73 && *h==0x68 ) {
		return FW_TYPE_QSDK;
	}
	if (*m1_55==0x55 && *m2_aa==0xAA)
		return FW_TYPE_OPENWRT_EMMC;

	return FW_TYPE_OPENWRT;
}

int upgrade(){
	char cmd[128] = {0};
	int ret = -1;
	int fw_type=do_checkout_firmware(NULL, 0, 0, NULL);
	if ( fw_type == FW_TYPE_OPENWRT ) {
		switch (gboard_param->machid) {
			case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
			case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
			case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
				if(openwrt_firmware_size < hex2int(getenv("filesize"), strlen(getenv("filesize")))){
					printf("Firmware too large! Not flashing.\n");
					return 0;
				}
				sprintf(cmd, "sf probe && sf erase 0x%x 0x%x && sf write 0x88000000 0x%x $filesize",
					openwrt_firmware_start, openwrt_firmware_size, openwrt_firmware_start);
				break;
			case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
				sprintf(cmd, "nand device 1 && nand erase 0x%x 0x%x && nand write 0x88000000 0x%x $filesize",
					openwrt_firmware_start, openwrt_firmware_size, openwrt_firmware_start);
				break;
			default:
				break;
		}
	}else if (fw_type == FW_TYPE_OPENWRT_EMMC){
			switch (gboard_param->machid) {
				case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
				case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
					//erase 531MB, defealt partion 16MB kernel + 512MB rootfs
					sprintf(cmd, "mmc erase 0x0 0x109800 && mmc write 0x88000000 0x0 0x%lx", (unsigned long int) (hex2int(getenv("filesize"), strlen(getenv("filesize"))) / 512 + 1) );
					printf("%s\n", cmd);
					break;
				default:
					break;
			}
	}else {
		sprintf(cmd, "sf probe && imgaddr=0x88000000 && source $imgaddr:script");
	}
	ret = run_command(cmd, 0);
	return ret;
}

#if defined(GL_IPQ40XX_CMD_RUN)
int do_run (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int ret = -1;
	char cmd[128] = {0};
	int encrypt_update = 1;//enalbe
	
	if (argc < 2) {
		do_run_usage();
		return CMD_RET_FAILURE;
	}

	if (!strncmp(argv[1], "lu", 2)) {
		sprintf(cmd, "tftpboot 0x88000000 %s", uboot_name);
		if ( (ret = run_command(cmd, 0)) == GL_OK) {
			if (TftpdownloadStatus != GL_OK) {
				printf("%s failed. Please re-download.\n", cmd);
				return CMD_RET_FAILURE;
			} else {
				TftpdownloadStatus = -1;
			}
		}

		if ( ret == GL_OK ) {
			sprintf(cmd, "sf probe && sf erase 0x%x 0x%x && sf write 0x88000000 0x%x $filesize", 
				CONFIG_UBOOT_START, CONFIG_UBOOT_SIZE, CONFIG_UBOOT_START);
			if(CONFIG_UBOOT_SIZE < hex2int(getenv("filesize"), strlen(getenv("filesize"))))
				return 0;
			ret = run_command(cmd, 0);			
		}

	} else if (!strncmp(argv[1], "lf", 2) && (strlen(argv[1]) == 2)) {
		sprintf(cmd, "tftpboot 0x88000000 %s", openwrt_fw_name);
		if ( (ret = run_command(cmd, 0)) == GL_OK) {
			if (TftpdownloadStatus != GL_OK) {
				printf("%s failed. Please re-download.\n", cmd);
				return CMD_RET_FAILURE;
			} else {
				TftpdownloadStatus = -1;
#ifdef CONFIG_RSA
				ret = rsa_verify(NetBootFileXferSize);
#endif
			}
		}

		if ( ret == GL_OK ) {
			ret = upgrade();
		}
	} else if (!strncmp(argv[1], "lfq", 3) && (strlen(argv[1]) == 3)) {
		sprintf(cmd, "tftpboot 0x88000000 %s", qsdk_fw_name);
		if ( (ret = run_command(cmd, 0)) == GL_OK) {
			if (TftpdownloadStatus != GL_OK) {
				printf("%s failed. Please re-download.\n", cmd);
				return CMD_RET_FAILURE;
			} else {
				TftpdownloadStatus = -1;
			}
		}			

		if ( ret == GL_OK ) {
			ret = upgrade();
		}
/*	}else if (!strncmp(argv[1], "lc", 2)) {
		sprintf(cmd, "tftpboot 0x88000000 %s", config_name);
		if (run_command(cmd, 0) == GL_OK) {
			if (TftpdownloadStatus != GL_OK) {
				printf("%s failed. Please re-download\n", cmd);
				return CMD_RET_FAILURE;
			} else {
				TftpdownloadStatus = -1;

				sprintf(cmd, "sf probe && sf read 0x84010000 0x%x 0x%x", CONFIG_ART_START, CONFIG_ART_SIZE);
				run_command(cmd, 0);
	
				change_ethernet_mac();
				change_wifi_mac();
				sprintf(cmd, "sf erase 0x%x 0x%x && sf write 0x84010000 0x%x 0x%x", 
					CONFIG_ART_START, CONFIG_ART_SIZE, CONFIG_ART_START, CONFIG_ART_SIZE);
				ret = run_command(cmd, 0);
			}
		}
*/
#ifdef CONFIG_HTTPD
	} else if (!strncmp(argv[1], "httpd", 5)) {
		if(gboard_param->machid == MACH_TYPE_IPQ40XX_AP_DK04_1_C3)
			gpio_set_value(GPIO_B2200_POWER_WHITE_LED, 0);
		HttpdLoop();
#endif
	} else { 
		printf("Usage: %s\n", cmdtp->usage);
		return CMD_RET_USAGE;
	}

	if (ret == GL_OK) {
		printf("TFTP download and flash OK.\n");
	} else {
		printf("TFTP download failed. Please re-download.\n");
	}
	gpio_set_value(g_gpio_power_led, !g_is_power_led_active_low);
	LED_INIT();

	return CMD_RET_SUCCESS;
}

int do_flash (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int ret = -1;
	char cmd[128] = {0};
	ulong	addr, dest, count;
	int	size;
	
	if (argc < 3) {
		do_flash_usage();
		return CMD_RET_FAILURE;
	}

	if (g_run_sf_probe) {
		g_run_sf_probe = 0;
		sprintf(cmd, "sf probe");
		run_command(cmd, 0);
	}

	if (!strncmp(argv[2], "eth", 3) || !strncmp(argv[2], "ath", 3)) {
		volatile unsigned char *mac = NULL;
		sprintf(cmd, "sf read 0x88000100 0x%x 0x%x", CONFIG_ART_START, CONFIG_ART_SIZE);
		run_command(cmd, 0);

		if (!strncmp(argv[1], "r", 1)) {
			if (!strncmp(argv[2], "eth0", 4)) {
				mac = (volatile unsigned char *)0x88000100;
			} else if (!strncmp(argv[2], "eth1", 4)) {
				mac = (volatile unsigned char *)0x88000106;
			} else if (!strncmp(argv[2], "ath0", 4)) {
				mac = (volatile unsigned char *)0x88001106;
			} else if (!strncmp(argv[2], "ath1", 4)) {
				mac = (volatile unsigned char *)0x88005106;
			} else if (!strncmp(argv[2], "ath2", 4)) {
				mac = (volatile unsigned char *)0x88009106;
			} else {
				do_flash_usage();
				return CMD_RET_FAILURE;
			}	
		} else if (!strncmp(argv[1], "w", 1)) {
			if (strlen(argv[3]) != 12) {
				puts("Invalid mac address length\n");
				return 1;
			}
			
			if (!strncmp(argv[2], "eth0", 4)) {
				mac = (volatile unsigned char *)0x88000100;
			} else if (!strncmp(argv[2], "eth1", 4)) {
				mac = (volatile unsigned char *)0x88000106;
			} else if (!strncmp(argv[2], "ath0", 4)) {
				mac = (volatile unsigned char *)0x88001106;
			} else if (!strncmp(argv[2], "ath1", 4)) {
				mac = (volatile unsigned char *)0x88005106;
			} else if (!strncmp(argv[2], "ath2", 4)) {
				mac = (volatile unsigned char *)0x88009106;
			} else {
				do_flash_usage();
				return CMD_RET_FAILURE;
			}
			COPY_STR2MAC(mac, argv[3]);
			if (!strncmp(argv[2], "ath0", 4) || !strncmp(argv[2], "ath1", 4) || !strncmp(argv[2], "ath2", 4)){
				int checksum_2g = 0;
				int checksum_5g = 0;
				int checksum_5g_9886 = 0;
				volatile unsigned short *wifi_2g_checksum = (volatile unsigned short *)0x88001102;
				volatile unsigned short *wifi_5g_checksum = (volatile unsigned short *)0x88005102;
				volatile unsigned short *wifi_5g_9886_checksum = (volatile unsigned short *)0x88009102;
				volatile unsigned short *wifi_2g_art = (volatile unsigned short *)0x88001100;
				volatile unsigned short *wifi_5g_art = (volatile unsigned short *)0x88005100;
				volatile unsigned short *wifi_5g_9886_art = (volatile unsigned short *)0x88009100;
				*wifi_2g_checksum = *wifi_5g_checksum = *wifi_5g_9886_checksum = -1;
				for (i=0; i<12064; i+=2) {
				   checksum_2g ^= *wifi_2g_art;
				   checksum_5g ^= *wifi_5g_art;
				   checksum_5g_9886 ^= *wifi_5g_9886_art;
				   wifi_2g_art++;
				   wifi_5g_art++;
				   wifi_5g_9886_art++;
				}
				*wifi_2g_checksum = checksum_2g;
				*wifi_5g_checksum = checksum_5g;
				*wifi_5g_9886_checksum = checksum_5g_9886;
			}
			puts ("Copy to flash... ");
			sprintf(cmd, "sf erase 0x%x 0x%x && sf write 0x88000100 0x%x 0x%x", 
				CONFIG_ART_START, CONFIG_ART_SIZE, CONFIG_ART_START, CONFIG_ART_SIZE);
			run_command(cmd, 0);
			puts ("done\n");
		
		} else {
			do_flash_usage();
			return CMD_RET_FAILURE;
		}
		printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", argv[2], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return 0;
	}

	if (argc < 4) {
		do_flash_usage();
		return CMD_RET_FAILURE;
	}

	addr = simple_strtoul(argv[2], NULL, 16);
	dest = simple_strtoul(argv[3], NULL, 16);
	count = simple_strtoul(argv[4], NULL, 16);
	
	if (!strncmp(argv[1], "r", 1)) {
		printf("read addr 0x%08x to dest 0x%08x %x bytes\n", addr, dest, count);
	} else if (!strncmp(argv[1], "w", 1)) {
		printf("write addr 0x%08x to dest 0x%08x %x bytes\n", addr, dest, count);
	}
	
	return 0;
}
#endif

#ifdef CHECK_ART_REGION
int check_network(int tryCount)
{
	int argc = 2;
	char *argv[2];
	argv[0] = "2";
	argv[1] = "192.168.1.2";

	if (tryCount <= 0) {
		return 1;
	}

	while (tryCount--) {
		if (do_ping(NULL, 0, argc, argv) == 0) {
			auto_update_flags = 1;
			break;
		}
		udelay (1000000);
	}

	if (auto_update_flags) {
		return 0;
	} else {
		return 1;
	}
}

int find_calibration_data()
{
	int ret = -1;
	volatile unsigned short *cal_2g_data = NULL;
	volatile unsigned short *cal_5g_data = NULL;
	volatile unsigned short *cal_5g_9886_data = NULL;

	char cmd[128] = {0};
	sprintf(cmd, "sf probe && sf read 0x88000000 0x%x 0x%x", CONFIG_ART_START, CONFIG_ART_SIZE);
	run_command(cmd, 0);

	cal_2g_data = (volatile unsigned short *)(0x88000000+0x1000);
	cal_5g_data = (volatile unsigned short *)(0x88000000+0x5000);
	cal_5g_9886_data = (volatile unsigned short *)(0x88000000+0x9000);

	//printf("cal_2g_data = 0x%x, cal_5g_data = 0x%x\n", *cal_2g_data, *cal_5g_data);
	printf("Checking calibration status...\n");

	if (*cal_2g_data == 0x2f20 && *cal_5g_data == 0x2f20 && (gboard_param->machid == MACH_TYPE_IPQ40XX_AP_DK04_1_C3 ? *cal_5g_9886_data == 0x2f20:1)) {
		printf("Device is calibrated, checking test status...\n");
		ret = 0;
	} else {
		printf("Device isn't calibrated, booting the calibration firmware...\n");
		ret = -1;
	}
	
	return ret;
}

int check_test()
{
	int ret = 0;
	volatile unsigned char *f1f = NULL;
	volatile unsigned char *f2i = NULL;
	volatile unsigned char *f3r = NULL;
	volatile unsigned char *f4s = NULL;
	volatile unsigned char *f5t = NULL;
	volatile unsigned char *f6t = NULL;
	volatile unsigned char *f7e = NULL;
	volatile unsigned char *f8s = NULL;
	volatile unsigned char *f9t = NULL;

	volatile unsigned char *s0s = NULL;
	volatile unsigned char *s1e = NULL;
	volatile unsigned char *s2c = NULL;
	volatile unsigned char *s3o = NULL;
	volatile unsigned char *s4n = NULL;
	volatile unsigned char *s5d = NULL;
	volatile unsigned char *s6t = NULL;
	volatile unsigned char *s7e = NULL;
	volatile unsigned char *s8s = NULL;
	volatile unsigned char *s9t = NULL;

	char cmd[128] = {0};
	sprintf(cmd, "sf read 0x88000000 0x%x 16 && sf read 0x88000010 0x%x 16",
		(CONFIG_ART_START + 0x50), (CONFIG_ART_START + 0x60));
	run_command(cmd, 0);

	f1f = (volatile unsigned char *)0x88000000;
	f2i = (volatile unsigned char *)0x88000001;
	f3r = (volatile unsigned char *)0x88000002;
	f4s = (volatile unsigned char *)0x88000003;
	f5t = (volatile unsigned char *)0x88000004;
	f6t = (volatile unsigned char *)0x88000005;
	f7e = (volatile unsigned char *)0x88000006;
	f8s = (volatile unsigned char *)0x88000007;
	f9t = (volatile unsigned char *)0x88000008;

	s0s = (volatile unsigned char *)0x88000010;
	s1e = (volatile unsigned char *)0x88000011;
	s2c = (volatile unsigned char *)0x88000012;
	s3o = (volatile unsigned char *)0x88000013;
	s4n = (volatile unsigned char *)0x88000014;
	s5d = (volatile unsigned char *)0x88000015;
	s6t = (volatile unsigned char *)0x88000016;
	s7e = (volatile unsigned char *)0x88000017;
	s8s = (volatile unsigned char *)0x88000018;
	s9t = (volatile unsigned char *)0x88000019;

	
	if (*f1f==0x66 && *f2i==0x69 && *f3r==0x72 && *f4s==0x73 && \
			*f5t==0x74 && *f6t==0x74 && *f7e==0x65 && *f8s==0x73 && \
			*f9t==0x74 && \
			*s0s==0x73 && *s1e==0x65 && *s2c==0x63 && *s3o==0x6f && \
			*s4n==0x6e && *s5d==0x64 && *s6t==0x74 && *s7e==0x65 && \
			*s8s==0x73 && *s9t==0x74) {
		printf("Device is tested, checking MAC info...\n");
		ret = 0;
	} else {
		printf("Device isn't tested: please test device in calibration firmware...\n");
		ret = -1;
	}

	return ret;

}

int check_config()
{
	int i = 0;
	u8 addr[6];
	u8 addr_tmp[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	volatile unsigned char *tmp = NULL;

	char cmd[128] = {0};
	sprintf(cmd, "sf read 0x88000000 0x%x 16 && sf read 0x88000010 0x%x 16 && sf read 0x88000020 0x%x 16 && sf read 0x88000030 0x%x 16", 
		CONFIG_ART_START, (CONFIG_ART_START+0x1000), (CONFIG_ART_START+0x5000), (CONFIG_ART_START+0x9000));
	run_command(cmd, 0);

	/*check eth0 mac*/
	for (i=0; i<6; i++) {
		tmp = (volatile unsigned char *)0x88000000 + i;
		addr[i] = *tmp;
	}
	//printf("eth0: %x:%x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (!memcmp(addr, addr_tmp, 6)) {
		printf("Device doesn't have eth0 MAC info: please write MAC in calibration firmware...\n");
		return -1;
	}

	/*check eth1 mac*/
	for (i=0; i<6; i++) {
		tmp = (volatile unsigned char *)0x88000006 + i;
		addr[i] = *tmp;
	}
	//printf("eth1: %x:%x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (!memcmp(addr, addr_tmp, 6)) {
		printf("Device doesn't have eth1 MAC info: please write MAC in calibration firmware...\n");
		return -1;
	}

	/*check 2G mac*/
	for (i=0; i<6; i++) {
		tmp = (volatile unsigned char *)0x88000016 + i;
		addr[i] = *tmp;
	}
	//printf("2G: %x:%x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (!memcmp(addr, addr_tmp, 6)) {
		printf("Device doesn't have 2GHz MAC info: please write MAC in calibration firmware...\n");
		return -1;
	}

	/*check 5G mac*/
	for (i=0; i<6; i++) {
		tmp = (volatile unsigned char *)0x88000026 + i;
		addr[i] = *tmp;
	}
	//printf("5G: %x:%x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (!memcmp(addr, addr_tmp, 6)) {
		printf("Device doesn't have 5GHz MAC info: please write MAC in calibration firmware...\n");
		return -1;
	}

	/*check 5G_9886 mac*/
	if(gboard_param->machid == MACH_TYPE_IPQ40XX_AP_DK04_1_C3 ){
	for (i=0; i<6; i++) {
		tmp = (volatile unsigned char *)0x88000036 + i;
		addr[i] = *tmp;
	}
	//printf("5G: %x:%x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	if (!memcmp(addr, addr_tmp, 6)) {
		printf("Device doesn't have 5GHz MAC info: please write MAC in calibration firmware...\n");
		return -1;
	}
	}
	printf("Device has MAC info, starting firmware...\n\n");
	
	return 0;
}

int auto_update_by_tftp()
{
	int ret = -1;
	char cmd[128] = {0};
	char cmd1[128] = {0};

	if (check_network(3) != GL_OK) {
		//printf("host no alive.\n");
		return -1;
	}

	sprintf(cmd, "tftpboot 0x88000000 %s", openwrt_fw_name);
	sprintf(cmd1, "tftpboot 0x88000000 %s", qsdk_fw_name);

	if (run_command(cmd, 0) == GL_OK) {
		if (TftpdownloadStatus != GL_OK) {
			printf("TFTP download of firmware.bin failed. Please re-download.\n");
			return CMD_RET_FAILURE;
		} else {
			ret = upgrade();
			return ret;
		}
	}

	if (run_command(cmd1, 0) == GL_OK) {
		if (TftpdownloadStatus != GL_OK) {
			printf("TFTP download of firmware.bin failed. Please re-download.\n");
			return CMD_RET_FAILURE;
		} else {
			ret = upgrade();
			return ret;
		}
	}
	return ret;
}

#endif

void wan_led_toggle(void)//@by luoyejiang
{

}

char uboot_name[64];
char openwrt_fw_name[64];
char qsdk_fw_name[64];
int openwrt_firmware_start;
int openwrt_firmware_size;
int g_gpio_power_led;
int g_gpio_led_tftp_transfer_flashing;
int g_gpio_led_upgrade_write_flashing_1;
int g_gpio_led_upgrade_write_flashing_2;
int g_gpio_led_upgrade_erase_flashing;
int g_is_flashing_power_led=0;
int g_is_power_led_active_low=0;
int dos_boot_part_lba_start, dos_boot_part_size, dos_third_part_lba_start;


#ifdef CONFIG_QCA_MMC
static qca_mmc *host = &mmc_host;
#endif


void get_mmc_part_info(){
	block_dev_desc_t *blk_dev;
	blk_dev = mmc_get_dev(host->dev_num);
	if(blk_dev->part_type == PART_TYPE_DOS){
		printf("\n\n");
		print_part(blk_dev);
		printf("\n\n");
	}
}

void gl_names_init()
{
	switch (gboard_param->machid) {
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
		sprintf(uboot_name, "uboot-gl-%s.bin", "s1300");
		sprintf(openwrt_fw_name, "openwrt-gl-%s.bin", "s1300");
		sprintf(qsdk_fw_name, "qsdk-gl-%s.bin", "s1300");
		openwrt_firmware_start=0x180000;
		openwrt_firmware_size=0xe80000;
		g_gpio_power_led=GPIO_S1300_POWER_LED;
		g_gpio_led_tftp_transfer_flashing=GPIO_S1300_MESH_LED;
		g_gpio_led_upgrade_write_flashing_1=GPIO_S1300_MESH_LED;
		g_gpio_led_upgrade_write_flashing_2=GPIO_S1300_WIFI_LED;
		g_gpio_led_upgrade_erase_flashing=GPIO_S1300_WIFI_LED;
		get_mmc_part_info();
		break;
	case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
		sprintf(uboot_name, "uboot-gl-%s.bin", "b2200");
		sprintf(openwrt_fw_name, "openwrt-gl-%s.bin", "b2200");
		sprintf(qsdk_fw_name, "qsdk-gl-%s.bin", "b2200");
		openwrt_firmware_start=0x180000;
		openwrt_firmware_size=0xe80000;
		g_gpio_power_led=GPIO_B2200_POWER_BLUE_LED;
		g_gpio_led_tftp_transfer_flashing=GPIO_B2200_POWER_BLUE_LED;
		g_gpio_led_upgrade_write_flashing_1=GPIO_B2200_POWER_BLUE_LED;
		g_gpio_led_upgrade_write_flashing_2=GPIO_B2200_POWER_BLUE_LED;
		g_gpio_led_upgrade_erase_flashing=GPIO_B2200_POWER_BLUE_LED;
		g_is_power_led_active_low=0;
		get_mmc_part_info();
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
		sprintf(uboot_name, "uboot-gl-%s.bin", "ap1300");
		sprintf(openwrt_fw_name, "openwrt-gl-%s.bin", "ap1300");
		sprintf(qsdk_fw_name, "qsdk-gl-%s.bin", "ap1300");
		openwrt_firmware_start=0x0;
		openwrt_firmware_size=0x8000000;
		g_gpio_power_led=GPIO_AP1300_POWER_LED;
		g_gpio_led_tftp_transfer_flashing=GPIO_AP1300_POWER_LED;
		g_gpio_led_upgrade_write_flashing_1=GPIO_AP1300_POWER_LED;
		g_gpio_led_upgrade_write_flashing_2=GPIO_AP1300_POWER_LED;
		g_gpio_led_upgrade_erase_flashing=GPIO_AP1300_POWER_LED;
		g_is_flashing_power_led=1;
		break;
	case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
		sprintf(uboot_name, "uboot-gl-%s.bin", "b1300");
		sprintf(openwrt_fw_name, "openwrt-gl-%s.bin", "b1300");
		sprintf(qsdk_fw_name, "qsdk-gl-%s.bin", "b1300");
		openwrt_firmware_start=0x180000;
		openwrt_firmware_size=0x1e80000;
		g_gpio_power_led=GPIO_B1300_POWER_LED;
		g_gpio_led_tftp_transfer_flashing=GPIO_B1300_MESH_LED;
		g_gpio_led_upgrade_write_flashing_1=GPIO_B1300_MESH_LED;
		g_gpio_led_upgrade_write_flashing_2=GPIO_B1300_WIFI_LED;
		g_gpio_led_upgrade_erase_flashing=GPIO_B1300_WIFI_LED;
		break;
	default:
		break;
	}
}

void LED_INIT(void)
{
	switch (gboard_param->machid) {
		case MACH_TYPE_IPQ40XX_AP_DK01_1_C1:
			gpio_set_value(GPIO_B1300_MESH_LED, 0);
			gpio_set_value(GPIO_B1300_WIFI_LED, 0);
			break;
		case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
			gpio_set_value(GPIO_AP1300_POWER_LED, 1);
			gpio_set_value(GPIO_AP1300_INET_LED, 0);
			break;
		case MACH_TYPE_IPQ40XX_AP_DK04_1_C1:
			gpio_set_value(GPIO_S1300_MESH_LED, 0);
			gpio_set_value(GPIO_S1300_WIFI_LED, 0);
			break;
		case MACH_TYPE_IPQ40XX_AP_DK04_1_C3:
			//NB: b2200 white led active low; only light on power blue led
			gpio_set_value(GPIO_B2200_INET_WHITE_LED, 1);
			gpio_set_value(GPIO_B2200_INET_BLUE_LED, 0);
			gpio_set_value(GPIO_B2200_POWER_BLUE_LED, 1);
			gpio_set_value(GPIO_B2200_POWER_WHITE_LED, 1);
			break;
		default:
			break;
	}
}

void LED_BOOTING(void)
{
	switch (gboard_param->machid) {
		case MACH_TYPE_IPQ40XX_AP_DK01_1_C2:
			gpio_set_value(GPIO_AP1300_POWER_LED, 1);
			gpio_set_value(GPIO_AP1300_INET_LED, 0);
			break;
		default:
			break;
	}
}
