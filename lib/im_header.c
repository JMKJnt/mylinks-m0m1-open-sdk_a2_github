#include <built_info.h>
#include <image.h>

struct img_head ih =
{
magic:		IH_MAGIC,
hlen:		IH_LEN,
size:		(IH_SIZE+3)&~3 ,
run:		IH_RUN,
#ifdef	CONFIG_RLIB_COMPATIBLE
rver:		2,//force to 2 to compatible with boot loader of A1
#else
rver:		ROM_VER,
#endif
aver:		IH_VER,
flags:		0x0,
mid:		IH_MODEL_SFLASH,
time:		0x0,
chksum:		0x0,
resv:		0x0,
} ;

