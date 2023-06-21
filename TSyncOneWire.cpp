/* File: TsyncOneWire.cpp
 *
 * Author: Seth Kerr
 *
 * Copyright (c) 2023 Seth Kerr & Oak Technology Holdings, LLC.
 *
 * License: MIT
 */

#include "TSyncOneWire.h"

TSyncOneWire::TSyncOneWire(const uint8_t ID, const int pinNum)
    : hostID(ID), dataPin(pinNum), dutyCycle(100) {
  receivingData = true; // even though the default is true, we'll
                        // still assert it here.
  sessionPacket->srcId = hostID;
  sessionPacket->destId = 0;
  sessionPacket->numDataBytes = 0;
  sessionPacket->lastFrame = true;
}

TSyncOneWire::~TSyncOneWire() {}

bool polling(void) {
  uint8_t count = 0;
  while (receivingData) {
    uint8_t start = digitalRead(pinNum);
    while (start) {
      count++ start = digitalRead(pinNum);
    }
    dutyCycle = count;
  }
}

bool TSyncOneWire::fillTxBuffer(const uint8_t *data, const bool lastByte) {
  bool ret;
  if (data == nullptr) {
    ret = false;
  } else {
    if (lastByte) {
      txBuffer.push(*data);
      sessionPacket->lastByte = true;
      receivingData = false;
    } else {
      txBuffer.push(*data);
    }
    ret = true;
  }
  return ret;
}

bool TSyncOneWire::unloadRxBuffer(void) {
  bool ret = false;
  if (rxBuffer.size() == 0) {
    ret = false;
  } else {
    while (rxBuffer.size > 0) {
      bool srcRcv = false;
      bool dstRcv = false;
      bool numBytesRcv = false;
      bool dataFrameRcv = false;
      if (!srcRcv) {
        sessionPacket->srcId = rxBuffer.pop();
        srcId = true;
      }
      if (!dstRcv) {
        sessionPacket->dstId = rxBuffer.pop();
        dstRcv = true;
      }
      if (hostID == dstId) {
        if (!numBytesRcv) {
          sessionPacket->numDataBytes = rxBuffer.pop();
        }
        if (!dataFrameRcv) {
          sessionPacket->dataFrame.clear();
          for (int j = 0; j < sessionPacket->numDataBytes; j++) {
            sessionPacket->dataFrame = rxBuffer.pop();
          }
        }
        ret = true;
      } else {
        ret = false;
      }
    }
  }
  return ret;
}

void TSyncOneWire::receiveData(void) {
  rxBuffer.clear();
  while (receivingData == true) {
    pinMode(dataPin, INPUT);

    for (int i = 0; i < 8; i++) {
      int inData = digitalRead(dataPin);
      data << inData;
      delay(dutyCycle);
    }
    rxBuffer.push(data);
  }
}

void TSyncOneWire::transmitData(void) {
  while (receivingData == false) {
    pinMode(dataPin, OUTPUT);
    while (txBuffer.size() > 0) {
      sessionPacket->dataFrame.push(txBuffer.pop());
    }
    bool sending = true;
    bool srcSent = false;
    bool dstSent = false;
    bool numBytesSent = false;
    bool dataFrameSent = false;
    while (sending) {
      if (!srcSent) {
        int dataOut = sessionPacket->srcId;
        for (int i = 0; i < 8; i++) {
          digitalWrite(dataPin, (dataOut << 1));
          delay(dutyCycle);
        }
        srcSent = true;
      }
      if (!dstSent) {
        int dataOut = sessionPacket->dstId;
        for (int i = 0; i < 8; i++) {
          digitalWrite(dataPin, (dataOut << 1));
          delay(dutyCycle);
        }
        dstSent = true;
      }
      if (!numBytesSent) {
        int dataOut = sessionPacket->numDataBytes;
        for (int i = 0; i < 8; i++) {
          digitalWrite(dataPin, (dataOut << 1));
          delay(dutyCycle);
        }
        numBytesSent = true;
      }
      if (!dataFrameSent) {
        for (int j = 0; j < MAX_FRAME_SIZE; j++) {
          int dataOut = sessionPacket->dataFrame[j];
          for (int i = 0; i < 8; i++) {
            digitalWrite(dataPin, (dataOut << 1));
            delay(dutyCycle);
          }
        }
        dataFrameSent = true;
      }
      if (sessionPacket->lastByte) {
        digitalWrite(dataPin, sessionPacket->lastByte);
        delay(dutyCycle);
      }
      if (srcSent && dstSent && numBytesSent && dataFrameSent) {
        sending = false;
      }
    }
    receivingData = true;
  }
}