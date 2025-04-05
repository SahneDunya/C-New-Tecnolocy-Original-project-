#include "cnt_i2c.h"

// I2C Başlatma Fonksiyonu
bool I2C_Init(uint32_t port, I2C_InitTypeDef* init) {
    // Donanım bağımlı I2C başlatma mantığı
    // ... I2C başlatma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// I2C Ana Veri Gönderme Fonksiyonu
bool I2C_MasterTransmit(uint32_t port, uint16_t slaveAddress, uint8_t* data, uint32_t size) {
    // Donanım bağımlı I2C ana veri gönderme mantığı
    // ... I2C ana veri gönderme mantığı ...

    // Başarılı ise true döndür
    return true;
}

// I2C Ana Veri Alma Fonksiyonu
bool I2C_MasterReceive(uint32_t port, uint16_t slaveAddress, uint8_t* data, uint32_t size) {
    // Donanım bağımlı I2C ana veri alma mantığı
    // ... I2C ana veri alma mantığı ...

    // Başarılı ise true döndür
    return true;
}

// I2C Köle Veri Gönderme Fonksiyonu
bool I2C_SlaveTransmit(uint32_t port, uint8_t* data, uint32_t size) {
    // Donanım bağımlı I2C köle veri gönderme mantığı
    // ... I2C köle veri gönderme mantığı ...

    // Başarılı ise true döndür
    return true;
}

// I2C Köle Veri Alma Fonksiyonu
bool I2C_SlaveReceive(uint32_t port, uint8_t* data, uint32_t size) {
    // Donanım bağımlı I2C köle veri alma mantığı
    // ... I2C köle veri alma mantığı ...

    // Başarılı ise true döndür
    return true;
}