/* File: TsyncOneWire.h
*
* Author: Seth Kerr
*
* Copyright (c) 2023 Seth Kerr & Oak Technology Holdings, LLC.
*
* License: MIT
*/

#ifndef TSYNCONEWIRE_H
#define TSYNCONEWIRE_H

#include <vector>

#define MAX_FRAME_SIZE 256

class TSyncOneWire
{
public:
    TSyncOneWire(const uint8_t ID,
                 const int pinNum);

    virtual ~TSyncOneWire(void);

    virtual bool polling(void);

    virtual bool fillTxBuffer(const Vector<uint8_t>& data, const bool lastByte);

    virtual bool unloadRxBuffer(Vector<uint8_t>& data);

private:
    // Disallowed methods
    TSyncOneWire() = delete;
    TsyncOneWire() = TSyncOneWire(const &TSyncOneWire);
    TSyncOneWire operator = (const &TSyncOneWire);

    struct TSyncPacket
    {
        uint8_t destId;
        uint8_t srcId;
        uint8_t numDataBytes;
        uint8_t dataFrame[MAX_FRAME_SIZE];
        bool lastFrame;
    };

    virtual void receiveData(void);

    virtual void transmitData(void);

    TSyncPacket* sessionPacket;
    uint8_t hostID;
    int dataPin;
    bool receivingData = false; // true by default
    Vector<uint8_t> txBuffer;
    Vector<uint8_t> rxBuffer;
};

#endif