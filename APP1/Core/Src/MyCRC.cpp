//
// Created by Lenovo on 2026/7/16.
//
#ifndef __MYCRC_H_
#define __MYCRC_H_
#include "../Inc/MyCRC.h"
#include "main.h"

extern "C" {

CRC32_Context crc_ctx;

void CRC32_Init(CRC32_Context *ctx) {
    ctx->crc = 0xFFFFFFFF;
}

void CRC32_Update(CRC32_Context *ctx, const uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        ctx->crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            ctx->crc = (ctx->crc >> 1) ^ (0xEDB88320 & -(ctx->crc & 1));
        }
    }
}

static uint32_t swap_uint32(uint32_t val) {
    return ((val >> 24) & 0xFF)
           | ((val >> 8)  & 0xFF00)
           | ((val << 8)  & 0xFF0000)
           | ((val << 24) & 0xFF000000);
}

uint32_t CRC32_Final(CRC32_Context *ctx, int swap_endian)
{
    uint32_t crc = ctx->crc ^ 0xFFFFFFFF;
    return swap_endian ? swap_uint32(crc) : crc;
}

}
#endif

