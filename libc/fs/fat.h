#ifndef FAT_H
#define FAT_H

#include "bootdev.h"

// FAT filesystem initialization
int fat_init(boot_device_t* dev);

#endif // FAT_H
