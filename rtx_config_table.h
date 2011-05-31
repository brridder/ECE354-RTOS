/**
 * @file: rtx_config_table.h
 * @brief: Data structures for the configuration tables
 * @author: Ben Ridder
 * @date: 05/24/2011
 */

#ifndef _RTX_CONFIG_TABLE_H_
#define _RTX_CONFIG_TABLE_H_

#include "rtx_inc.h"

typedef struct _rtx_memory_table {
    UINT32 memory_block_size;
    UINT32 num_created;
} rtx_memory_table;

#endif
