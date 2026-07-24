//
// Created by Lenovo on 2026/7/16.
//

// crc32_ota.h
#ifndef __MYCRC32_H__
#define __MYCRC32_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t crc;
} CRC32_Context;
extern CRC32_Context crc_ctx;
void CRC32_Init(CRC32_Context *ctx);
void CRC32_Update(CRC32_Context *ctx, const uint8_t *data, uint32_t len);
uint32_t CRC32_Final(CRC32_Context *ctx, int swap_endian);

#ifdef __cplusplus
}
#endif

#endif //APP1_CRC_H
