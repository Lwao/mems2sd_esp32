#ifndef SD_LIB_INCLUDED 
#define SD_LIB_INCLUDED // define if SD card libraries were included
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#endif

#ifndef _SD_DRIVER_H_
#define _SD_DRIVER_H_

/*

*/
char* merge_filename(const char *filename);

/*

*/
FILE* open_file(const char *filename, char *mode);

/*

*/
void close_file(FILE *file);

/*

*/
void rename_file(const char *actualfname, const char *targetfname);

#endif  // _SD_DRIVER_H_