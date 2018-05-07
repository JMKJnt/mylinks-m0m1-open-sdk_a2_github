/*=============================================================================+
|                                                                              |
| Copyright 2015                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
#define	VER_PSTR			"v"
#define	VER_MAJOR			2
#define	VER_MINOR			2
//#define	VER_PATCH			6

#define	_VERNUM_(m,n,p)		(((m)<<8)|((n)))
#define STR(num)			_STR(num)
#define _STR(num)  			#num
#define _VERSTR_(ps,m,n)  ps STR(m) "." STR(n) 

#define MT_MODEL_ID			0x8000
#define	MT_SDK_VER_STR		_VERSTR_ (VER_PSTR,  VER_MAJOR, VER_MINOR)
#define MT_SDK_VER_NUM		_VERNUM_ 			(VER_MAJOR, VER_MINOR)

extern const char *sw_build_time;
extern const char *sw_build_sdk;
extern const int sw_build_count;
