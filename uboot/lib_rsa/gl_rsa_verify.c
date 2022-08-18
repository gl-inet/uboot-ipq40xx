

#include <common.h>
#include <gl_config.h>
#include "gl/gl_ipq40xx_api.h"
#include "polarssl/config.h"
#include "polarssl/rsa.h"
#include "polarssl/aes.h"


#ifdef CONFIG_RSA
#define BUFFERSIZE	2048
#define RSA_E   "10001"

#ifndef CFG_LOAD_ADDR
#define CFG_LOAD_ADDR CONFIG_FIRMWARE_START
#endif

int gl_CurrentStatus = 0;

int align(int len)
{
	len += 65536 - ((len%65536)==0 ? 65536 : (len%65536));
	return len;
}


void print_hex(char *name, unsigned char *data, int len)
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

int check_endian(void)  
{  
    int i = 0x12345678;  
    char *c = &i;  
    return ((c[0] == 0x78) && (c[1] == 0x56) && (c[2] == 0x34) && (c[3] == 0x12));  
}

void test_endian(void)
{
	if( check_endian() )  
        printf("little endian\n");  
    else  
        printf("big endian\n");
}

int check_endian_test(void)
{
	test_endian();

	return 0;
}

int update_sign()
{
	int i;
	char cmd[128] = {0};
	volatile unsigned char *p_sig = NULL;
	volatile unsigned char *p_art_sig = NULL;
	volatile unsigned char *p_fw = NULL;
	volatile unsigned int *p_fw_len = NULL;
	volatile unsigned int *p_sig_flag = NULL;
	volatile unsigned int *p_deadcode = NULL;
	volatile unsigned short *p_sig_len = NULL;

	int fw_len;

	/* read the length of the firmware */
	sprintf(cmd, "cp.b 0x%x 0x80800000 0x40", CFG_LOAD_ADDR);
	run_command(cmd, 0);
	p_fw_len = (volatile unsigned int *)(0x80800000 + 60);
	fw_len = *p_fw_len;
	if (fw_len < 0) {
		printf("Can't find the firmware.\n");
		return 1;
	}
	//printf("p_fw_len = %d (0x%08X) \n", fw_len, fw_len);

	/* read firmware and get sign  */
	sprintf(cmd, "cp.b 0x%x 0x80800000 0x%x", (CFG_LOAD_ADDR + fw_len - CONFIG_DEADCODE_LEN), CONFIG_SIGN_PADDING_LEN);
	run_command(cmd, 0);

	p_deadcode = (volatile unsigned int *)0x80800000;
	//printf("p_deadcode = 0x%08x\n", *p_deadcode);
	if (*p_deadcode != 0xdeadc0de)
		return 1;
	
	p_sig_flag = (volatile unsigned int *)(0x80800000 + CONFIG_DEADCODE_LEN);

	//printf("p_sig_flag = 0x%08x\n", *p_sig_flag);
	if (*p_sig_flag == 0x78563412) {

		gl_CurrentStatus = CONFIG_STATUS_UPDATE_SIGN;

		/* get the length of the sign   */
		p_sig_len = (volatile unsigned short *)(0x80800000 + CONFIG_DEADCODE_LEN + CONFIG_SIGN_FLAG_LEN);
		//printf("p_sig_len = %d (0x%04x)\n", *p_sig_len, *p_sig_len);
		p_sig = (volatile unsigned char *)(0x80800000 + CONFIG_DEADCODE_LEN + CONFIG_SIGN_FLAG_LEN + CONFIG_SIGN_LEN);

		/* read art from flash to mem */
		sprintf(cmd, "cp.b 0x9f050000 0x80801000 0x10000");
		run_command(cmd, 0);
		p_art_sig = (volatile unsigned char *)(0x80801000 + 0xFBFF);

		/* update sign */
		for (i=0; i<*p_sig_len; i++) {
			*p_art_sig++ = *p_sig++;
		}

		printf("------------------------------\n");
		printf("Update Sign To FLASH ...\n");
		sprintf(cmd, "erase 0x9f050000 +0x10000");
		run_command(cmd, 0);
		sprintf(cmd, "cp.b 0x80801000 0x9f050000 0x10000");
		run_command(cmd, 0);

		printf("------------------------------\n");
		printf("Remove Sign From Firmware ...\n");
		sprintf(cmd, "erase 0x%x +0x%x", (CFG_LOAD_ADDR + fw_len - CONFIG_DEADCODE_LEN), CONFIG_SIGN_PADDING_LEN);
		//printf("cmd = %s\n", cmd);
		run_command(cmd, 0);
		sprintf(cmd, "cp.b 0x80800000 0x%x 0x4", (CFG_LOAD_ADDR + fw_len - CONFIG_DEADCODE_LEN));
		run_command(cmd, 0);

		printf("------------------------------\n");
		printf("Update Sign Success ...\n");
	}

	return 0;
}

int firmware_verify()
{
	volatile unsigned int *p_fw_len = NULL;
	unsigned int sig_len = 0, fw_len = 0;
	char cmd[128] = {0};
	unsigned char buf[POLARSSL_MPI_MAX_SIZE];
	char *RSA_N = NULL;
	rsa_context rsa;
	unsigned char sha1sum[20];
	ulong load_dst;
	unsigned char aes_key[32];
	unsigned char aes_key_iv[16];
	int len;
	char dec_out[BUFFERSIZE] = {0}; 
	char *pout = dec_out;
	int ret;
	int sig_flag_size = 4;

	int encrypt_update = 1;//enalbe
	const char *s = getenv("rsa");

	volatile unsigned char *p_fw = NULL;
	volatile unsigned char *p_fw_sig = NULL;
	volatile unsigned char *p_rsa_key = NULL;
	volatile unsigned short *p_rsa_key_len = NULL;
	volatile unsigned char *p_aes_key = NULL;
	volatile unsigned char *p_aes_key_iv = NULL;
	volatile unsigned short *p_aes_key_len = NULL;

	if ( s != NULL ) {
		if (strcmp(s, "disable") == 0) {
			return 0;
		}
	}

	/* firmware start addr */
	load_dst = 0x80800000;
	
	/* get key data from flash */
	sprintf(cmd, "cp.b 0x%x 0x%x 0x1000", CONFIG_RSA_PUBKEY_START, 0x80800000);
	run_command(cmd, 0);

	/* read rsa pub key */
	p_rsa_key_len = (volatile unsigned short *)load_dst;
	p_rsa_key = (volatile unsigned char *)(load_dst + 0x2);

	/* read aes key */
	p_aes_key_len = (volatile unsigned short *)(load_dst + 0x800);
	p_aes_key = (volatile unsigned char *)(load_dst + 0x802);
	p_aes_key_iv = (volatile unsigned char *)(load_dst + 0x802 + *p_aes_key_len);
	if ( *p_aes_key_len > 32 ) {
		printf("Key error.\n");
		return 1;
	}
	strncpy(aes_key, p_aes_key, *p_aes_key_len);
	aes_key[*p_aes_key_len] = '\0';
	strncpy(aes_key_iv, p_aes_key_iv, 16);
	aes_key_iv[16] = '\0';

	/* decrypt */
	len = device_aes_decrypt(aes_key, 32, aes_key_iv, p_rsa_key, *p_rsa_key_len, pout, BUFFERSIZE);
	RSA_N = (char *)malloc( len + 1);
	memset(RSA_N, 0x00, len + 1);
	strncpy(RSA_N, pout, len);

	rsa_init( &rsa, RSA_PKCS_V15, 0 );
	mpi_read_string( &rsa.N , 16, RSA_N  );
	mpi_read_string( &rsa.E , 16, RSA_E  );
	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
	sig_len = rsa.len;

	if( rsa_check_pubkey(  &rsa ) != 0 ) {
		printf( "\n rsa_check_pubkey failed\n" );
		goto exit;
	}

	/* read firmware.sig  */
	p_fw_sig = (volatile unsigned char *)(CONFIG_RSA_PUBKEY_START + 0x800 + 0x400);
	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, p_fw_sig, sig_len);
	//print_hex("p_fw_sig", buf, sig_len);

	/* read the length of the firmware */
	sprintf(cmd, "cp.b 0x%x 0x80800000 0x40", CFG_LOAD_ADDR);
	run_command(cmd, 0);
	p_fw_len = (volatile unsigned int *)(0x80800000 + 60);
	if (*p_fw_len < 0) {
		printf("Can't find the firmware.\n");
		return 1;
	}

	/* read firmware and get sha1sum  */
	sprintf(cmd, "cp.b 0x%x 0x80800000 0x%x", CFG_LOAD_ADDR, *p_fw_len);
	run_command(cmd, 0);
	p_fw = (volatile unsigned char *)0x80800000;
	fw_len = *p_fw_len - 4;
	sha1( p_fw, fw_len, sha1sum );
	print_hex("sha1sum", sha1sum, 20);

	if( ( ret = rsa_pkcs1_verify( &rsa, NULL, NULL, RSA_PUBLIC,
								  POLARSSL_MD_SHA1, 20, sha1sum, buf ) ) != 0 ) {
		printf( " \n  ! rsa_pkcs1_verify returned -0x%0x\n\n", -ret );
		goto exit;
	}

	printf("Verify PASS!\n\n");

	free(RSA_N);
	rsa_free( &rsa );
	return 0;
exit:	
	free(RSA_N);
	rsa_free( &rsa );
	return 1;
}

#if 1
int rsa_verify(ulong filesize)
{
	unsigned int sig_len = 0, fw_len = 0;
	char cmd[128] = {0};
	unsigned char buf[POLARSSL_MPI_MAX_SIZE];
	char *RSA_N = NULL;
	rsa_context rsa;
	unsigned char sha1sum[20];
	ulong load_dst;
	unsigned char aes_key[32];
	unsigned char aes_key_iv[16];
	int len;
	char dec_out[BUFFERSIZE] = {0};	
	char *pout = dec_out;
	int ret;
	int is_burn_fw = 0;
	int is_burn_ub = 0;
	u32 t_download_addr = 0;
	int encrypt_update = 1;//enalbe
	int sig_flag_size = 4;
	const char *s = getenv("rsa");

	volatile unsigned char *p_fw = NULL;
	volatile unsigned char *p_fw_sig = NULL;
	volatile unsigned char *p_rsa_key = NULL;
	volatile unsigned short *p_rsa_key_len = NULL;
	volatile unsigned char *p_aes_key = NULL;
	volatile unsigned char *p_aes_key_iv = NULL;
	volatile unsigned short *p_aes_key_len = NULL;
	volatile unsigned int *p_sig_flag = NULL;

	u32 rsa_key_len, aes_key_len;

	if (gl_CurrentStatus != CONFIG_STATUS_DOWNLOAD_FW)
		return 0;
	else
		gl_CurrentStatus = 0;

	/* if filesize > 1M, the file cannot be uboot.bin but firmware.bin */
	if (filesize > 1024*1024) {
		printf("\nDownload firmware...Verify...\n");
		is_burn_fw = 1;
		
	} else if (filesize > 64*1024) {
		printf("\nDownload uboot...Verify...\n");
		is_burn_ub = 1;
		
	} else {/* Don't check the little files */
		//printf("\nDownload other...Don't verify...\n");
		return 0;
	}

	if ( s != NULL ) {
		if (strcmp(s, "disable") == 0) {
			return 0;
		}
	}
	
	t_download_addr = 0x88000000;

	p_fw = (volatile unsigned char *)t_download_addr;
	load_dst = align(t_download_addr + filesize + 0x1000);
	
	/* get data from flash */
	sprintf(cmd, "sf probe && sf read 0x%x 0x%x 0x1000", load_dst, CONFIG_RSA_PUBKEY_START);
	printf("cmd = %s\n", cmd);
	run_command(cmd, 0);

	/* read rsa pub key */
	p_rsa_key_len = (volatile unsigned short *)load_dst;
	p_rsa_key = (volatile unsigned char *)(load_dst + 0x2);
	rsa_key_len = htons(*p_rsa_key_len);

	/* read aes key */
	p_aes_key_len = (volatile unsigned short *)(load_dst + 0x800);
	aes_key_len = htons(*p_aes_key_len);
	p_aes_key = (volatile unsigned char *)(load_dst + 0x802);
	p_aes_key_iv = (volatile unsigned char *)(load_dst + 0x802 + aes_key_len);
	printf("aes_key_len = %d\n", aes_key_len);
	if ( aes_key_len > 32 ) {
		printf("Key error.\n");
		return 1;
	}
	strncpy(aes_key, p_aes_key, aes_key_len);
	aes_key[aes_key_len] = '\0';
	strncpy(aes_key_iv, p_aes_key_iv, 16);
	aes_key_iv[16] = '\0';
	printf("aes_key = %s, key_iv = %s\n", aes_key, aes_key_iv);

	/* decrypt */
	len = device_aes_decrypt(aes_key, 32, aes_key_iv, p_rsa_key, rsa_key_len, pout, BUFFERSIZE);
	RSA_N = (char *)malloc( len + 1);
	memset(RSA_N, 0x00, len + 1);
	strncpy(RSA_N, pout, len);
	printf("RSA_N = %s\n", RSA_N);

	rsa_init( &rsa, RSA_PKCS_V15, 0 );
	mpi_read_string( &rsa.N , 16, RSA_N  );
    mpi_read_string( &rsa.E , 16, RSA_E  );
	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;

	if( rsa_check_pubkey(  &rsa ) != 0 ) {
        printf( "\n rsa_check_pubkey failed\n" );
        goto exit;
    }

	/*get  firmware.bin sha1sum */
	sig_len = rsa.len;
	if (is_burn_fw) {
		fw_len = filesize - CONFIG_SIGN_PADDING_LEN - CONFIG_DEADCODE_LEN;
		p_fw_sig = (volatile unsigned char *)(t_download_addr + fw_len + CONFIG_DEADCODE_LEN + CONFIG_SIGN_FLAG_LEN + CONFIG_SIGN_LEN);
	} else {
		fw_len = filesize - CONFIG_SIGN_PADDING_LEN;
		p_fw_sig = (volatile unsigned char *)(t_download_addr + fw_len + CONFIG_SIGN_FLAG_LEN + CONFIG_SIGN_LEN);
	}

	sha1( p_fw, fw_len, sha1sum );

	/*get  firmware.sig  */
	memset(buf, 0x00, sizeof(buf));
	memcpy(buf, p_fw_sig, sig_len);

    if( ( ret = rsa_pkcs1_verify( &rsa, NULL, NULL, RSA_PUBLIC,
                                  POLARSSL_MD_SHA1, 20, sha1sum, buf ) ) != 0 ) {
        printf( " \n  ! rsa_pkcs1_verify returned -0x%0x\n\n", -ret );
		goto exit;
    }

	printf("Verify PASS!\n\n");

	if (is_burn_ub) {
		char cbuf[10];
		sprintf(cbuf, "0x%lX", (filesize - CONFIG_SIGN_PADDING_LEN));
		setenv("filesize", cbuf);
	}
	
	free(RSA_N);
	rsa_free( &rsa );
	return 0;
exit:	
	free(RSA_N);
	rsa_free( &rsa );
	return 1;
}
#else
int rsa_verify_test()
{
#if defined(POLARSSL_RSA_SELF_TEST)
	rsa_self_test(1);
#endif
#if  defined(AES_SELF_TEST)
	aes_self_test();
#endif
	check_endian_test();
	return 0;
}

#endif

#endif
