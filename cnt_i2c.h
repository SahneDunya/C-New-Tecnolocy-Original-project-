#ifndef CNT_I2C_H
#define CNT_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// I2C Modu
typedef enum {
    I2C_MODE_MASTER,
    I2C_MODE_SLAVE,
    // ... diğer I2C modları ...
} I2C_ModeTypeDef;

// I2C Adresleme Modu
typedef enum {
    I2C_ADDRESSING_MODE_7BIT,
    I2C_ADDRESSING_MODE_10BIT,
    // ... diğer I2C adresleme modları ...
} I2C_AddressingModeTypeDef;

// I2C Yapılandırma Yapısı
typedef struct {
    I2C_ModeTypeDef Mode;
    I2C_AddressingModeTypeDef AddressingMode;
    uint32_t ClockSpeed;
    uint16_t OwnAddress;
} I2C_InitTypeDef;

// I2C Başlatma Fonksiyonu
bool I2C_Init(uint32_t port, I2C_InitTypeDef* init);

// I2C Veri Gönderme Fonksiyonu
bool I2C_MasterTransmit(uint32_t port, uint16_t slaveAddress, uint8_t* data, uint32_t size);

// I2C Veri Alma Fonksiyonu
bool I2C_MasterReceive(uint32_t port, uint16_t slaveAddress, uint8_t* data, uint32_t size);

// I2C Köle Veri Gönderme Fonksiyonu
bool I2C_SlaveTransmit(uint32_t port, uint8_t* data, uint32_t size);

// I2C Köle Veri Alma Fonksiyonu
bool I2C_SlaveReceive(uint32_t port, uint8_t* data, uint32_t size);

#endif