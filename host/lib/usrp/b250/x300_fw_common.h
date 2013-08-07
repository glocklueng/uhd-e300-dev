//
// Copyright 2013 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef INCLUDED_X300_FW_COMMON_H
#define INCLUDED_X300_FW_COMMON_H

#include <stdint.h>

/*!
 * Structs and constants for x300 communication.
 * This header is shared by the firmware and host code.
 * Therefore, this header may only contain valid C code.
 */
#ifdef __cplusplus
extern "C" {
#endif

//shared memory sections - in between the stack and the program space
#define X300_FW_SHMEM_BASE 0x6000
#define X300_FW_SHMEM_GPSDO_STATUS 0
#define X300_FW_SHMEM_UART_RX_INDEX 2
#define X300_FW_SHMEM_UART_TX_INDEX 3
#define X300_FW_SHMEM_CLAIM_STATUS 5
#define X300_FW_SHMEM_CLAIM_TIME 6
#define X300_FW_SHMEM_UART_RX_POOL 256
#define X300_FW_SHMEM_UART_TX_POOL 512
#define X300_FW_SHMEM_UART_POOL_WORDS32 256

#define X300_FW_NUM_BYTES (1 << 15) //64k
#define X300_FW_COMMS_MTU (1 << 13) //8k
#define X300_FW_COMMS_UDP_PORT 49152

#define X300_VITA_UDP_PORT 49153
#define X300_GPSDO_UDP_PORT 49156
#define X300_FPGA_PROG_UDP_PORT 49157

#define X300_FW_COMMS_FLAGS_ACK        (1 << 0)
#define X300_FW_COMMS_FLAGS_ERROR      (1 << 1)
#define X300_FW_COMMS_FLAGS_POKE32     (1 << 2)
#define X300_FW_COMMS_FLAGS_PEEK32     (1 << 3)

#define X300_FPGA_PROG_FLAGS_ACK       (1 << 0)
#define X300_FPGA_PROG_FLAGS_ERROR     (1 << 1)
#define X300_FPGA_PROG_FLAGS_INIT      (1 << 2)
#define X300_FPGA_PROG_FLAGS_CLEANUP   (1 << 3)
#define X300_FPGA_PROG_FLAGS_ERASE     (1 << 4)
#define X300_FPGA_PROG_FLAGS_VERIFY    (1 << 5)
#define X300_FPGA_PROG_CONFIGURE       (1 << 6)
#define X300_FPGA_PROG_CONFIG_STATUS   (1 << 7)

typedef struct
{
    uint32_t flags;
    uint32_t sequence;
    uint32_t addr;
    uint32_t data;
} x300_fw_comms_t;

typedef struct
{
    uint32_t flags;
    uint32_t sector;
    uint32_t index;
    uint32_t size;
    uint16_t data[128];
} x300_fpga_prog_t;

typedef struct
{
    uint32_t flags;
} x300_fpga_prog_flags_t;

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_X300_FW_COMMON_H */
