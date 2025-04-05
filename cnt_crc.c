#include "cnt_crc.h"

// CRC Hesaplama Fonksiyonu
uint32_t CRC32_Calculate(const uint8_t* data, uint32_t length, uint32_t polynomial, uint32_t initialValue, bool reflectInput, bool reflectOutput, uint32_t finalXor) {
    uint32_t crc = initialValue;

    for (uint32_t i = 0; i < length; i++) {
        uint8_t byte = data[i];

        if (reflectInput) {
            byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4);
            byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2);
            byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1);
        }

        crc ^= (byte << 24);

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }

    if (reflectOutput) {
        crc = ((crc & 0xF0000000) >> 4) | ((crc & 0x0F000000) << 4);
        crc = ((crc & 0xCC000000) >> 2) | ((crc & 0x33000000) << 2);
        crc = ((crc & 0xAA000000) >> 1) | ((crc & 0x55000000) << 1);

        crc = ((crc & 0x00F00000) >> 4) | ((crc & 0x000F0000) << 4);
        crc = ((crc & 0x00CC0000) >> 2) | ((crc & 0x00330000) << 2);
        crc = ((crc & 0x00AA0000) >> 1) | ((crc & 0x00550000) << 1);

        crc = ((crc & 0x0000F000) >> 4) | ((crc & 0x00000F00) << 4);
        crc = ((crc & 0x0000CC00) >> 2) | ((crc & 0x00003300) << 2);
        crc = ((crc & 0x0000AA00) >> 1) | ((crc & 0x00005500) << 1);

        crc = ((crc & 0x000000F0) >> 4) | ((crc & 0x0000000F) << 4);
        crc = ((crc & 0x000000CC) >> 2) | ((crc & 0x00000033) << 2);
        crc = ((crc & 0x000000AA) >> 1) | ((crc & 0x00000055) << 1);
    }

    return crc ^ finalXor;
}