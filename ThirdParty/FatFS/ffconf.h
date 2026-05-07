/*---------------------------------------------------------------------------/
/  FatFs configuration for STM32F411RE data logger
/---------------------------------------------------------------------------*/

#define FFCONF_DEF	80386

/*---------------------------------------------------------------------------/
/ Function Configurations
/---------------------------------------------------------------------------*/

#define FF_FS_READONLY	0       /* 0 = read+write (we need to write log files) */
#define FF_FS_MINIMIZE	0       /* 0 = full basic API */
#define FF_USE_FIND		0
#define FF_USE_MKFS		0       /* card will be pre-formatted on a PC */
#define FF_USE_FASTSEEK	0
#define FF_USE_EXPAND	0
#define FF_USE_CHMOD	0
#define FF_USE_LABEL	0
#define FF_USE_FORWARD	0
#define FF_USE_STRFUNC	1       /* 1 = enable f_printf / f_puts for easy log writes */
#define FF_PRINT_LLI	0
#define FF_PRINT_FLOAT	0
#define FF_STRF_ENCODE	0

/*---------------------------------------------------------------------------/
/ Locale and Namespace Configurations
/---------------------------------------------------------------------------*/

#define FF_CODE_PAGE	437     /* US English — smallest code page, no DBCS tables */

#define FF_USE_LFN		0       /* 0 = disable long filenames — saves ~4 KB flash */
#define FF_MAX_LFN		255     /* no effect when FF_USE_LFN = 0 */
#define FF_LFN_UNICODE	0
#define FF_LFN_BUF		255
#define FF_SFN_BUF		12
#define FF_FS_RPATH		0
#define FF_PATH_DEPTH	10

/*---------------------------------------------------------------------------/
/ Drive/Volume Configurations
/---------------------------------------------------------------------------*/

#define FF_VOLUMES		1       /* one SD card */
#define FF_STR_VOLUME_ID	0
#define FF_VOLUME_STRS		"SD"
#define FF_MULTI_PARTITION	0

#define FF_MIN_SS		512     /* SD cards always use 512-byte sectors */
#define FF_MAX_SS		512

#define FF_LBA64		0
#define FF_MIN_GPT		0x10000000
#define FF_USE_TRIM		0

/*---------------------------------------------------------------------------/
/ System Configurations
/---------------------------------------------------------------------------*/

#define FF_FS_TINY		0
#define FF_FS_EXFAT		0       /* FAT32 only — exFAT requires LFN */

#define FF_FS_NORTC		1       /* 1 = no RTC on this board; use fixed timestamp */
#define FF_NORTC_MON	1
#define FF_NORTC_MDAY	1
#define FF_NORTC_YEAR	2025

#define FF_FS_CRTIME	0
#define FF_FS_NOFSINFO	0
#define FF_FS_LOCK		0

#define FF_FS_REENTRANT	0       /* 0 = single-task access; driver handles its own mutex */
#define FF_FS_TIMEOUT	1000

/*--- End of configuration options ---*/
