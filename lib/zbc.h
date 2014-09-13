/*
 * This file is part of libzbc.
 * 
 * Copyright (C) 2009-2014, HGST, Inc.  This software is distributed
 * under the terms of the GNU Lesser General Public License version 3,
 * or any later version, "as is," without technical support, and WITHOUT
 * ANY WARRANTY, without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  You should have received a copy
 * of the GNU Lesser General Public License along with libzbc.  If not,
 * see <http://www.gnu.org/licenses/>.
 * 
 * Authors: Damien Le Moal (damien.lemoal@hgst.com)
 *          Christoph Hellwig (hch@infradead.org)
 */

#ifndef __LIBZBC_INTERNAL_H__
#define __LIBZBC_INTERNAL_H__

/***** Including files *****/

#include <libzbc/zbc.h>
#include "zbc_log.h"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>

/***** Macro definitions *****/

/**
 * Device interface.
 */
#define ZBC_DEV_ZBC                             0x01
#define ZBC_DEV_ZAC                             0x02

/**
 * Device types.
 */
#define ZBC_DEV_TYPE_STANDARD                   0x0
#define ZBC_DEV_TYPE_HOST_AWARE                 0x0
#define ZBC_DEV_TYPE_HOST_MANAGED               0x14

/***** Type definitions *****/

/**
 * Device operations.
 */
typedef struct zbc_ops {
    /**
     * Open device.
     */
    int         (*zbd_open)(const char *filename, int flags,
                            struct zbc_device **pdev);

    /**
     * Read from a ZBC device
     */
    int32_t     (*zbd_pread)(struct zbc_device *,
                             zbc_zone_t *,
                             void *,
                             uint32_t,
                             uint64_t);

    /**
     * Write to a ZBC device
     */
    int32_t     (*zbd_pwrite)(struct zbc_device *,
                              zbc_zone_t *,
                              const void *,
                              uint32_t,
                              uint64_t);

    /**
     * Flush to a ZBC device cache.
     */
    int         (*zbd_flush)(struct zbc_device *,
                             uint64_t,
                             uint32_t,
                             int immediate);

    /**
     * Report a device zone information.
     * (mandatory)
     */
    int         (*zbd_report_zones)(struct zbc_device *,
                                    uint64_t,
                                    enum zbc_reporting_options,
                                    zbc_zone_t *,
                                    unsigned int *);

    /**
     * Reset a zone or all zones write pointer.
     * (mandatory)
     */
    int         (*zbd_reset_wp)(struct zbc_device *,
                                uint64_t);

    /**
     * Change a device zone configuration.
     * For emulated drives only (optional).
     */
    int         (*zbd_set_zones)(struct zbc_device *,
                                 uint64_t,
                                 uint64_t);
    
    /**
     * Change a zone write pointer.
     * For emulated drives only (optional).
     */
    int         (*zbd_set_wp)(struct zbc_device *,
                              uint64_t,
                              uint64_t);

} zbc_ops_t;

/**
 * Device descriptor.
 */
typedef struct zbc_device {

    /**
     * Device file path.
     */
    char                *zbd_filename;

    /**
     * Open flags (access rights).
     */
    int                 zbd_flags;

    /**
     * Device info.
     */
    zbc_device_info_t   zbd_info;

    /**
     * Device file descriptor.
     */
    int                 zbd_fd;

    /**
     * Device operations.
     */
    zbc_ops_t           *zbd_ops;

    /**
     * Cached zone information for emulation.
     */
    unsigned int        zbd_nr_zones;
    struct zbc_zone     *zbd_zones;
    int                 zbd_meta_fd;

} zbc_device_t;

/***** Internal device functions *****/

/**
 * ZBC SCSI device operations.
 */
extern zbc_ops_t zbc_scsi_ops;

/**
 * ZBC emulation (file or block device).
 */
extern struct zbc_ops zbc_file_ops;

/**
 * Allocate and initialize a device handle.
 */
extern zbc_device_t *
zbc_dev_alloc(const char *filename,
              int flags);

/**
 * Free a device handle.
 */
extern void
zbc_dev_free(zbc_device_t *dev);

/**
 * Close a device.
 */
extern int
zbc_dev_close(zbc_device_t *dev);

#endif /* __LIBZBC_INTERNAL_H__ */
