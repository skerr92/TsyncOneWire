/* File: TsyncOneWire.cpp
*
* Author: Seth Kerr
*
* Copyright (c) 2023 Seth Kerr & Oak Technology Holdings, LLC.
*
* License: MIT
*/

#include "TSyncOneWire.h"

TSyncOneWire::TSyncOneWire(const uint8_t ID,
                           const int pinNum)
: hostID(ID)
, dataPin(pinNum)
{
    receivingData = true; // even though the default is true, we'll
                          // still assert it here.
    sessionPacket->srcId = hostID;
    sessionPacket->destId = 0;
    sessionPacket->numDataBytes = 0;
    sessionPacket->lastFrame = true;
}

TSyncOneWire::~TSyncOneWire()
{
}

bool TSyncOneWire::fillTxBuffer(const uint8_t* data, const bool lastByte)
{
    bool ret;
    if (data == nullptr)
    {
        ret = false;
    }
    else
    {
        if (lastByte)
        {
            txBuffer.push(*data);
            receivingData = false;
        }
        else
        {
            txBuffer.push(*data);
        }
        ret = true;
    }
    return ret;
}

bool TSyncOneWire::unloadRxBuffer(Vector<uint8_t>& data)
{
    bool ret = false;
    if (rxBuffer.size() == 0)
    {
        ret = false;
    }
    else
    {
        while (rxBuffer.size > 0)
        {
            data.push(rxBuffer.pop());
        }
        ret = true;
    }
    return ret;
}

void TSyncOneWire::receiveData(void)
{
    while (receivingData == true)
    {
        uint8_t data = 0;
        for (int i = 0; i < 8; i++)
        {
            int inData = digitalRead(dataPin);
            data << inData;
        }
        rxBuffer.push(data);
    }
}

void TSyncOneWire::transmitData(void)
{
    while (receivingData == false)
    {
        pinMode(dataPin, OUTPUT);
        while (txBuffer.size() > 0)
        {
            sessionPacket->dataFrame.push(txBuffer.pop());
        }
        
    }
}