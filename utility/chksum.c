#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <openssl/aes.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
//#include "../include/fw_signature_key.h"
//#include <image.h>

#define FL_HI 16
#define FL_LO 17
#define CS_HI 28
#define CS_LO 29

static void usage();
static int do_chksum( char *ifile, char *ofile, char* keyfile);
#define F_PATH "./firmware.tmp"
#define ECC_PRVKEY_PATH "secp256k1-prvkey.pem"

typedef enum{
	AES = 0,
	ECC,
	NOENC
}enc_meth;

struct option lopts[] = {
	{"aes", required_argument, NULL, 'e'},
	{"ecc", required_argument, NULL, 'e'},
	{0, 0,0 ,0},
};
enc_meth encMeth = NOENC;

static void usage(char *name)
{
	fprintf(stderr, "usage: %s ifile ofile\n", name);
	exit(-1);
}
int temp_file_exist(void)
{
    FILE*fp=NULL;//需要注意
    fp=fopen(F_PATH,"r");
    if(NULL==fp)
    {
        return -1;//要返回错误代码
    }
    fclose(fp);
    fp=NULL;//需要指向空，否则会指向原打开文件地址
   
     return 0;
}
int getAesKey(char* keyfile, unsigned char* key)
{
	struct stat st;
	FILE *keyfp=0;
	int rc = -1;
	int tlen = 0, klen = 0;
	char *buf = NULL, *pch1 = NULL, *pch2 = NULL;

	if (rc=stat(keyfile, &st)) {
		fprintf(stderr, "file (%s) stat err =%d, %s\n", keyfile, rc, strerror(rc));
		goto err;	
	}
	if (st.st_size==0) {
		fprintf(stderr, "file (%s) size=0\n", keyfile);
		goto err;
	}
	if ((keyfp = fopen(keyfile, "r")) == NULL ) {
		fprintf(stderr, "ERR: fopen(%s)\n", keyfile);
		goto err;
	}
	
	tlen = (int)st.st_size - 1;//substract end character
	if( (buf=(char*)malloc(tlen))==0 ) {
		fprintf(stderr, "ERR: malloc file buf\n");
		goto err;
	}

	memset(buf, 0, tlen);
		
	if ( (rc=fread(buf, tlen, 1, keyfp)) != 1 )
	{
		fprintf(stderr, "ERR: fread(%s) rc=%d\n",keyfile, rc);
		rc = -1;
		goto err;
	}
	pch1 = strstr(buf, "FW_AESKEY");
	pch1 = strchr(pch1,   '"');
	pch2 = strchr(pch1+1, '"');
	klen = pch2 - pch1 - 1;

	if(klen != AES_BLOCK_SIZE){
		fprintf(stderr, "ERR: size of key = %d is incorrect, must be %d\n",klen, AES_BLOCK_SIZE);
		goto err;
	}

	memcpy(key, pch1+1, klen);

	rc = 0;
err:
	if(keyfp)
		fclose(keyfp);
	if(buf)
		free(buf);

	return rc; 
			
}

int halve_MD5size( unsigned char *MD5buf,unsigned char *MD5)
{
   char MD5_TMP[16];
    unsigned char Temp[3]={0};
   int i=0,j=0;
   char strMD5[32+1];
   strcpy(strMD5,MD5buf);
   strMD5[32] = '\0';
   for(i=0;i<32;i=i+2)
   {
       Temp[0] = strMD5[i];
       Temp[1] = strMD5[i+1];
       Temp[2] = '\0';
       MD5_TMP[j]= strtol(Temp, NULL, 16);  
       j++;
   }
   memcpy(MD5,MD5_TMP,16);
#if 0
   printf("%s:\n",__func__);
   for(i=0;i<16;i++)
	printf("%02x",MD5[i]);
#endif
   return 16;
}

#define _LINE_LENGTH 300
static int do_chksum(char *ifile, char *ofile, char* keyfile)
{
	FILE *ifp=0, *keyfp=0;
	FILE *ofp=0;
	FILE *tempfp=0;
	FILE *popenfile=0;
	int rc = -1;
	struct stat st;
	unsigned char *buf=0;
	char  MD5_tmp[33] = {0};
	char *temp_file = NULL;
	unsigned int data;
	unsigned int chksum;
	int i;
	int len;
	char header_len = 0;
	char cmd[128] = {0};
	char line[_LINE_LENGTH];
	unsigned char key[AES_BLOCK_SIZE] = {0};
	AES_KEY aeskey;
	BIO *outbio = NULL;
	EC_KEY 	 *prvkey = NULL;
	const EC_GROUP *ecgrp;
	int	eccgrp;
	FILE *pkfp = NULL;
	FILE *pubkfp = NULL;
	ECDSA_SIG *sign_result = NULL;
	unsigned char MD5[16] = {0};
	int ret;
	unsigned char bn_r[32] = {0}, bn_s[32]={0};

	if (rc=stat(ifile, &st))
	{
		fprintf(stderr, "stat err =%d, %s\n", rc, strerror(rc));
		goto err;	
	}

	
	if (st.st_size==0)
	{
		fprintf(stderr, "file (%s) size=0\n", ifile);
		goto err;
	}

	if ((ifp = fopen(ifile, "r")) == NULL )
	{
		fprintf(stderr, "ERR: fopen(%s)\n", ifile);
		goto err;
	}

	if ((ofp = fopen(ofile, "w")) == NULL )
	{
		fprintf(stderr, "ERR: fopen(%s)\n", ofile);
		goto err;
	}

	len=(int)st.st_size;
	//fprintf(stderr, "file size=%d (0x%x)\n",len, len);

	if ((buf=(char*) malloc( len+1024 ))==0)
	{
		fprintf(stderr, "ERR: malloc\n");
		goto err;
	}
	memset(buf, 0, len+256);

	if ( (rc=fread(buf, len, 1,  ifp)) != 1 )
	{
		fprintf(stderr, "ERR: fread(%s) rc=%d\n",ifile, rc);
		rc = -1;
		goto err;
	}
	else
		rc =0;
	len=(len+3)&~3; //align to 4 bytes

	buf[FL_LO] |= (1<<2); /* IH_CHKSUM_EN */ 
	chksum=0;
	for (i=0; i<len; i+=2)
	{
		data = (buf[i]<<8) | buf[i+1];
		chksum += data;
		while (chksum>>16)
			chksum = (chksum&0xffff) + (chksum>>16);
		//printf("%04x\n", chksum);
	}
	printf("org=0x%04x ", chksum);
	chksum = (0xffff ^ chksum)  ;
	printf("chksum=0x%04x\n", chksum);
	//chksum++;
	buf[CS_HI]=(chksum>>8)&0xff;
	buf[CS_LO]=(chksum)&0xff;

	rc = 0;

	if ( (rc=fwrite(buf, len, 1, ofp)) != 1 )
	{
		fprintf(stderr, "ERR: fwrite(%s) rc=%d\n", ofile, rc);
		goto err;
	}
	
	if(ofp)
	{
		fclose(ofp);
		ofp = 0;
	}

	printf("---------------out file ---%s\n",ofile);

	if ((ofp = fopen(ofile, "a+")) == NULL )
	{
		fprintf(stderr, "ERR: fopen(%s)\n", ofile);
		goto err;
	}

	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"md5sum  %s",ofile);
	popenfile = popen(cmd,"r"); 
	if (popenfile != NULL)
	{
		if (fgets(line, _LINE_LENGTH, popenfile) != NULL) 
		{
			memcpy(MD5_tmp,line,32);
			printf("\nmd5 = %s-----filelen=%d\n",line,len+16);

			halve_MD5size(MD5_tmp,MD5);

			if(encMeth == AES){
				if(rc = getAesKey(keyfile, key)){
					fprintf(stderr, "ERR: failed to get key\n");
					goto err;
				}
					
				if (AES_set_encrypt_key(key, 128, &aeskey) < 0) {
	    	    	fprintf(stderr, "Unable to set encryption key in AES\n");
					rc = -1;
					goto err;
	    		}
	
				AES_cbc_encrypt(MD5, MD5, sizeof(MD5), &aeskey, key, AES_ENCRYPT);
				printf("succeed to encrypt md5 by AES\n");
			}
			else if(encMeth == ECC){
				if ((pkfp = fopen(ECC_PRVKEY_PATH, "r")) == NULL ) {
					fprintf(stderr, "ERR: fopen(%s)\n", ECC_PRVKEY_PATH);
					goto err;
				}
				outbio  = BIO_new(BIO_s_file());
				outbio = BIO_new_fp(stdout, BIO_NOCLOSE);
	
				prvkey = PEM_read_ECPrivateKey(pkfp, NULL, NULL, NULL);
	
				ecgrp = EC_KEY_get0_group(prvkey);
				printf("ECC Key type: %s\n", OBJ_nid2sn(EC_GROUP_get_curve_name(ecgrp)));
				
				sign_result = ECDSA_do_sign(MD5, sizeof(MD5), prvkey);
				if (sign_result == NULL)
				{
				    printf("Failed to generate EC Signature\n");
					goto err;
				}
				BN_bn2bin(sign_result->r, bn_r);
				BN_bn2bin(sign_result->s, bn_s);
				
				printf("succeed to encrypt md5 by ECC\n");
				}
			}

		pclose(popenfile);
	}
	else
	{
		printf("make md5sum ERR------------\n");
		pclose(popenfile);
		goto err;
	}
	if(encMeth == ECC){
		
	if ( (rc=fwrite(bn_r, 32, 1, ofp)) != 1 || (rc=fwrite(bn_s, 32, 1, ofp)) != 1)
	{
		printf("ERR: fwrite(%s) rc=%d-----filelen=%d\n", ofile, rc,len+64);
		goto err;
	}
	}
	else{
	if ( (rc=fwrite(MD5, 16, 1, ofp)) != 1 )
	{
		printf("ERR: fwrite(%s) rc=%d-----filelen=%d\n", ofile, rc,len+32);
		goto err;
	}
	}
	
err:
	if (buf)
		free(buf);

	if(tempfp)
		fclose(tempfp);
	
	if (ifp)
		fclose(ifp);

	if (ofp)
		fclose(ofp);
	if(pkfp)
		fclose(pkfp);

	return rc;
}
int main(int argc, char* argv[])
{
	int opt, opt_idx;
	char *ifile = NULL, *ofile = NULL, *keyfile = NULL;
	if (argc < 3) 
	{
		usage(argv[0]);
	}

	while( (opt=getopt_long(argc, argv, "i:o:", lopts, &opt_idx )) != -1 ){
		switch(opt){
		case 'i':
			ifile = optarg;
			break;
		case 'o':
			ofile = optarg;
			break;
		case 'e':
			if(opt_idx == AES)
				encMeth = AES;
			else if(opt_idx == ECC)
				encMeth = ECC;
			keyfile = optarg;
			OpenSSL_add_all_algorithms();
			break;
		default:
			printf("ERR:invalid option\n");
			return -1;
			break;
		}
	}

	if (do_chksum(ifile, ofile, keyfile) < 0) 
	{
		return -1;
	}
	return 0;
}
