#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <image.h>

#define FL_HI 16
#define FL_LO 17
#define CS_HI 28
#define CS_LO 29

static void usage();
static int do_chksum( char *ifile, char *ofile);
#define F_PATH "./firmware.tmp"
static void usage(char *name)
{
	fprintf(stderr, "usage: %s ifile ofile\n", name);
	exit(-1);
}
int temp_file_exist(void)
{
    FILE*fp=NULL;//D¨¨¨°a¡Á¡é¨°a
    fp=fopen(F_PATH,"r");
    if(NULL==fp)
    {
        return -1;//¨°a¡¤¦Ì??¡ä¨ª?¨®¡ä¨²??
    }
    fclose(fp);
    fp=NULL;//D¨¨¨°a???¨°??¡ê?¡¤??¨°?¨¢???¨°?-¡ä¨°?a???t¦Ì??¡¤
   
     return 0;
}
#define _LINE_LENGTH 300
static int do_chksum(char *ifile, char *ofile)
{
	FILE *ifp=0;
	FILE *ofp=0;
	FILE *tempfp=0;
	FILE *popenfile=0;
	int rc = -1;
	struct stat st;
	unsigned char *buf=0;
	char  temp_buf[33] = {0};
	char *temp_file = NULL;
	unsigned int data;
	unsigned int chksum;
	int i;
	int len;
	char header_len = 0;
	char cmd[128] = {0};
	char line[_LINE_LENGTH];
	
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
	memcpy(temp_buf,line,32);
	printf("\nmd5 = %s--%s---filelen=%d\n",line,temp_buf,len+32);
	}
	pclose(popenfile);
	}
	else
	{
	printf("make md5sum ERR------------\n");
	pclose(popenfile);
	goto err;
	}

	if ( (rc=fwrite(temp_buf, 32, 1, ofp)) != 1 )
	{
	printf("ERR: fwrite(%s) rc=%d-----filelen=%d\n", ofile, rc,len+32);
	goto err;
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

	return rc;
}

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		usage(argv[0]);
	}

	if (do_chksum(argv[1], argv[2]) < 0) 
	{
		return -1;
	}
	return 0;
}
