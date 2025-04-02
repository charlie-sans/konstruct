#ifndef ISO9660_H
#define ISO9660_H

#include "bootdev.h"
#include <stdint.h>
#include <stddef.h>

// ISO9660 constants
#define ISO9660_SECTOR_SIZE     2048
#define ISO9660_SIGNATURE       "CD001"
#define ISO9660_SIGNATURE_LEN   5
#define ISO9660_PRIMARY_VOL_DESC 1
#define ISO9660_END_VOL_DESC     255

// ISO9660 date/time structure
typedef struct {
    uint8_t year[4];      // ASCII digits (e.g., "2023")
    uint8_t month[2];     // ASCII digits (e.g., "01" for January)
    uint8_t day[2];       // ASCII digits (e.g., "01" for the 1st)
    uint8_t hour[2];      // ASCII digits (e.g., "12")
    uint8_t minute[2];    // ASCII digits
    uint8_t second[2];    // ASCII digits
    uint8_t hundredths[2]; // ASCII digits
    int8_t  timezone;     // Offset from GMT in 15 min intervals
} __attribute__((packed)) iso9660_datetime_t;

// Directory entry structure
typedef struct {
    uint8_t  length;                  // Length of directory record
    uint8_t  ext_attr_length;         // Extended attribute record length
    uint32_t extent_location_lsb;     // Location of extent (LBA) - LSB first
    uint32_t extent_location_msb;     // Location of extent (LBA) - MSB first
    uint32_t data_length_lsb;         // Data length - LSB first
    uint32_t data_length_msb;         // Data length - MSB first
    iso9660_datetime_t recording_date; // Recording date and time
    uint8_t  file_flags;              // File flags
    uint8_t  file_unit_size;          // File unit size (0 = unspecified)
    uint8_t  interleave_gap;          // Interleave gap size (0 = unspecified)
    uint16_t volume_sequence_lsb;     // Volume sequence number - LSB first
    uint16_t volume_sequence_msb;     // Volume sequence number - MSB first
    uint8_t  filename_length;         // Length of filename
    char     filename[1];             // Filename (variable length)
} __attribute__((packed)) iso9660_dir_entry_t;

// Primary volume descriptor
typedef struct {
    uint8_t  type;                    // Volume descriptor type
    char     id[5];                   // "CD001"
    uint8_t  version;                 // Volume descriptor version
    uint8_t  unused1;                 // Unused
    char     system_id[32];           // System identifier
    char     volume_id[32];           // Volume identifier
    uint8_t  unused2[8];              // Unused
    uint32_t volume_space_lsb;        // Volume space size - LSB first
    uint32_t volume_space_msb;        // Volume space size - MSB first
    uint8_t  unused3[32];             // Unused
    uint16_t volume_set_size_lsb;     // Volume set size - LSB first
    uint16_t volume_set_size_msb;     // Volume set size - MSB first
    uint16_t volume_seq_num_lsb;      // Volume sequence number - LSB first
    uint16_t volume_seq_num_msb;      // Volume sequence number - MSB first
    uint16_t logical_block_size_lsb;  // Logical block size - LSB first
    uint16_t logical_block_size_msb;  // Logical block size - MSB first
    uint32_t path_table_size_lsb;     // Path table size - LSB first
    uint32_t path_table_size_msb;     // Path table size - MSB first
    uint32_t l_path_table_loc;        // Location of L-path table
    uint32_t opt_l_path_table_loc;    // Location of optional L-path table
    uint32_t m_path_table_loc;        // Location of M-path table
    uint32_t opt_m_path_table_loc;    // Location of optional M-path table
    iso9660_dir_entry_t root_dir_entry; // Root directory record
    char     volume_set_id[128];      // Volume set identifier
    char     publisher_id[128];       // Publisher identifier
    char     data_preparer_id[128];   // Data preparer identifier
    char     application_id[128];     // Application identifier
    char     copyright_file_id[38];   // Copyright file identifier
    char     abstract_file_id[36];    // Abstract file identifier
    char     bibliographic_file_id[37]; // Bibliographic file identifier
    iso9660_datetime_t creation_date;  // Volume creation date/time
    iso9660_datetime_t modification_date; // Volume modification date/time
    iso9660_datetime_t expiration_date;  // Volume expiration date/time
    iso9660_datetime_t effective_date;   // Volume effective date/time
    uint8_t  file_structure_version;  // File structure version
    uint8_t  unused4;                 // Unused
    uint8_t  application_data[512];   // Application use
    uint8_t  reserved[653];           // Reserved
} __attribute__((packed)) iso9660_pvd_t;

// Function prototypes
int iso9660_init(boot_device_t* dev);
int iso9660_read_file(const char* path, void* buffer, size_t size);
int iso9660_list_directory(const char* path, char* buffer, size_t size);

#endif // ISO9660_H
