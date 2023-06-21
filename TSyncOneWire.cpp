/* File: TsyncOneWire.cpp
 *
 * Author: Seth Kerr
 *
 * Copyright (c) 2023 Seth Kerr & Oak Technology Holdings, LLC.
 *
 * License: MIT
 */

#include "TsyncOneWire.h"
/*!
 * @brief Constructor
 * @param [in] ID - host ID for bus protocol
 * @param [in] pinNum - arduino pin number for communicating on
 * @return
 */
TSyncOneWire::TSyncOneWire(const uint8_t ID, const int pinNum)
    : hostID(ID), dataPin(pinNum), dutyCycle(100) {
  receivingData = true; // even though the default is true, we'll
                        // still assert it here.
  sessionPacket->srcId = hostID;
  sessionPacket->destId = 0;
  sessionPacket->numDataBytes = 0;
  sessionPacket->lastFrame = true;
}

/*!
 * @brief Destructor
 * @return
 */

TSyncOneWire::~TSyncOneWire() {}

/*!
 * @brief polls the bus pin for a high digital signal until it's not high
 *        Then sets the duty cycle for the transmission
 * @return bool once complete
 */

bool polling(void) {
  uint8_t count = 0;
  while (receivingData) {
    uint8_t start = digitalRead(pinNum);
    while (start) {
      count++ start = digitalRead(pinNum);
    }
    dutyCycle = count;
  }
  return true;
}

/*!
 * @brief Fills the TX Buffer with data to send out
 * @param [in] data - pointer to the data to be pushed
 *                    into the TX buffer
 * @param [in] lastByte - specifies if this is the last
 *                        byte to be sent
 * @return bool once complete
 */

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
/*!
 * @brief unloads the buffer of received data into the data struct
 * @return bool once complete
 */
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
/*!
 * @brief receives data on the bus until there are 4 cycles
 *        of 0s received
 * @return bool once complete
 */
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
/*!
 * @brief transmits data over the bus
 * @return bool once complete
 */
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