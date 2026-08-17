/**
 * @file    i2c_slave_0x50.ino
 * @brief   Arduino Uno R3 — I2C slave at address 0x50
 *
 * Wiring to Raspberry Pi 4 BSC1:
 *   RPi GPIO2 (SDA1, pin 3) --> Arduino A4 (SDA)
 *   RPi GPIO3 (SCL1, pin 5) --> Arduino A5 (SCL)
 *   RPi GND   (pin 6)       --> Arduino GND
 *
 * NOTE: RPi pull-ups are enabled in driver (GPIO_PUD_UP on GPIO2/GPIO3).
 *       Do NOT add additional external pull-ups — RPi 3.3V rail is enough.
 *       Arduino runs at 5V logic but Wire library is tolerant at 3.3V with
 *       RPi driving — works in practice at 100kHz.
 *
 * Behaviour:
 *   WRITE (master sends 4 bytes):
 *     Stores received bytes into rxBuf[0..3].
 *     Serial prints each byte received.
 *
 *   READ (master reads 4 bytes):
 *     Sends back exactly what was last written (echo).
 *     If nothing was written yet, sends 0x00 x4.
 *
 * Test vectors from RPi app:
 *   TX: 0xA1 0xB2 0xC3 0xD4  (TC-08)
 *   RX expected: 0xA1 0xB2 0xC3 0xD4  (TC-09 / TC-10)
 */

#include <Wire.h>

#define SLAVE_ADDR      0x50
#define BUFFER_SIZE     4

static uint8_t rxBuf[BUFFER_SIZE] = {0, 0, 0, 0};
static uint8_t rxCount = 0;

/* -------------------------------------------------------------------------
 * onReceive: called when master writes bytes to this slave
 * ------------------------------------------------------------------------- */
void onReceive(int numBytes)
{
    rxCount = 0;

    Serial.print("[SLAVE] RX ");
    Serial.print(numBytes);
    Serial.print(" bytes: ");

    while (Wire.available() && rxCount < BUFFER_SIZE)
    {
        rxBuf[rxCount] = Wire.read();
        Serial.print("0x");
        if (rxBuf[rxCount] < 0x10) { Serial.print("0"); }
        Serial.print(rxBuf[rxCount], HEX);
        Serial.print(" ");
        rxCount++;
    }

    /* Drain any extra bytes master sent beyond our buffer */
    while (Wire.available()) { Wire.read(); }

    Serial.println();
}

/* -------------------------------------------------------------------------
 * onRequest: called when master reads from this slave
 * Sends back the last received bytes (echo).
 * ------------------------------------------------------------------------- */
void onRequest(void)
{
    Wire.write(rxBuf, BUFFER_SIZE);

    Serial.print("[SLAVE] TX echo: ");
    for (uint8_t i = 0; i < BUFFER_SIZE; i++)
    {
        Serial.print("0x");
        if (rxBuf[i] < 0x10) { Serial.print("0"); }
        Serial.print(rxBuf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

/* -------------------------------------------------------------------------
 * setup
 * ------------------------------------------------------------------------- */
void setup(void)
{
    Serial.begin(9600);
    while (!Serial) {}   /* wait for Serial port to connect */

    Serial.println("[SLAVE] Arduino I2C slave starting");
    Serial.print("[SLAVE] Address: 0x");
    Serial.println(SLAVE_ADDR, HEX);

    Wire.begin(SLAVE_ADDR);
    Wire.setClock(100000UL);   /* 100 kHz — match RPi BSC1 */
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);

    Serial.println("[SLAVE] Ready — waiting for RPi master");
}

/* -------------------------------------------------------------------------
 * loop
 * ------------------------------------------------------------------------- */
void loop(void)
{
    /* Nothing needed — all work done in callbacks */
    delay(100);
}
