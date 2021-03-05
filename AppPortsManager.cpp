/*
 MIT License

 Copyright (c) 2021 Kurt Schwarze

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "AquaOS.h"
#include "stdio.h"

#define UNAVAILABLE 5

#define IS_ANALOG 1
#define IS_DIGITAL 0

/*

 FIRST DIGITALS, THEN ANALOGS

 DIGITAL:
 BIT 0: touched
 BIT 1: read pulse
 BIT 2: configured
 BIT 3: 0 LOW, 1 HIGH

 ANALOG:
 BIT 0: touched
 BIT 1: read pulse
 BIT 2: configured
 BIT 3, 4, 5, 6, 7 => Value 0 -> 31

 => bits: 4 * 54 + 8 x 16 = 43 bytes
 */

#define PORT_STATE_SIZE 43

unsigned char portState[PORT_STATE_SIZE];

namespace pm {

    int cardinality() {
        return APP_CARDINALITY_ALWAYS_ONE;
    }

    void whenPowered() {
        for (int i = 0; i < PORT_STATE_SIZE; i++) {
            portState[i] = 0;
        }
    }

    // BIT ARITHMETIC

    void setBit(int isAnalog, int port, int position, int value) {
        if (isAnalog)
            position = 4 * 54 + port * 8 + position;
        else
            position = port * 4 + position;
        int byteOffset = position / 8;
        int bitOffset = position % 8;
        int mask = 1 << bitOffset;
        if (value)
            portState[byteOffset] = portState[byteOffset] | mask;
        else
            portState[byteOffset] = (portState[byteOffset] | mask) - mask;
    }

    int getBit(int isAnalog, int port, int position) {
        if (isAnalog)
            position = 4 * 54 + port * 8 + position;
        else
            position = port * 4 + position;
        int byteOffset = position / 8;
        int bitOffset = position % 8;
        int mask = 1 << bitOffset;
        return (portState[byteOffset] & mask) > 0;
    }

    void setTouched(int isAnalog, int port, int isTouched) {
        setBit(isAnalog, port, 0, isTouched);
    }

    void setConfigured(int isAnalog, int port, int configured) {
        setBit(isAnalog, port, 2, configured);
    }

    void setValue(int isAnalog, int port, int val) {
        if (isAnalog) {
            val = val / 32;
            setBit(isAnalog, port, 3, val >> 4);
            setBit(isAnalog, port, 4, (val >> 3) % 2);
            setBit(isAnalog, port, 5, (val >> 2) % 2);
            setBit(isAnalog, port, 6, (val >> 1) % 2);
            setBit(isAnalog, port, 7, val % 2);
        } else
            setBit(isAnalog, port, 3, val);
    }

    int isConfigured(int isAnalog, int port) {
        return getBit(isAnalog, port, 2);
    }

    int isAccessed(int isAnalog, int port) {
        return getBit(isAnalog, port, 1);
    }

    void setAccessed(int isAnalog, int port) {
        setBit(isAnalog, port, 1, 1 - isAccessed(isAnalog, port));
    }

    int value(int isAnalog, int port) {
        if (isAnalog)
            return 31 * (getBit(isAnalog, port, 3) * 16 + getBit(isAnalog, port, 4) * 8 +
                         getBit(isAnalog, port, 5) * 4 + getBit(isAnalog, port, 6) * 2 + getBit(isAnalog, port, 7));
        return getBit(isAnalog, port, 3);
    }

    int isTouched(int isAnalog, int port) {
        return getBit(isAnalog, port, 0);
    }

    // READING BITS

    int readBit(unsigned char* array, int position) {
        int byteOffset = position / 8;
        int bitOffset = position % 8;
        int mask = 1 << bitOffset;
        return (array[byteOffset] & mask) > 0;
    }

    // VALIDATION

    void invalid(int analog, int port) {
        if ((analog && port < 0) || (analog && port > 15) || (!analog && port < 0) || (!analog && port > 53))
            fatalError(1001, port);
    }

    int _analogRead(int port) {
        invalid(IS_ANALOG, port);
        int val = analogRead(port);
        setConfigured(IS_ANALOG, port, 1);
        setTouched(IS_ANALOG, port, 1);
        setValue(IS_ANALOG, port, val / 32);
        setAccessed(IS_ANALOG, port);
        return val;
    }

    int _digitalRead(int port) {
        invalid(IS_DIGITAL, port);
        int val = digitalRead(port);
        setConfigured(IS_DIGITAL, port, 1);
        setTouched(IS_DIGITAL, port, 1);
        setAccessed(IS_DIGITAL, port);
        setValue(IS_DIGITAL, port, val);
        return val;
    }

    void _analogWrite(int port, int value) {
        invalid(IS_ANALOG, port);
        setConfigured(IS_ANALOG, port, 1);
        setTouched(IS_ANALOG, port, 1);
        setValue(IS_ANALOG, port, value);
        analogWrite(port, value);
    }

    void _digitalWrite(int port, int value) {
        invalid(IS_DIGITAL, port);
        setConfigured(IS_DIGITAL, port, 1);
        setTouched(IS_DIGITAL, port, 1);
        setAccessed(IS_DIGITAL, port);
        setValue(IS_DIGITAL, port, value);
        digitalWrite(port, value);
    }

    // draws a port

    void drawReading(int x, int y) {
        setColor(colorBlack);
        fillRect(x + 1, y + 1, 4, 4);
        setColor(colorGreen);
        fillRect(x + 2, y + 2, 2, 2);
    }

    void drawUk(int x, int y, int h) {
        setColor(colorWhite);
        fillRect(x, y, h, h);
        setColor(colorDarkGray);
        for (int i = 1; i < h - 1; i++)
            for (int j = 1; j < h - 1; j++)
                if ((i + j) % 4 == 0)
                    fillRect(x + i, y + j, 1, 1);
    }

    void drawDegrade(int x, int y, int h) {
        setColor(colorWhite);
        fillRect(x, y, h, h);
        for (int i = 0; i < h; i++) {
            int c = 255 * i / h;
            setColor(colorFromRGB(c, c, c));
            fillRect(x + i, y, 1, h);
        }
    }

    void drawPort(int analog, int port, int state) {
        int x = port % 10 * 14 + 21;
        int y = port / 10 * 14 + analog * 119 + 39;
        if (state == UNAVAILABLE) {
            setColor(colorBG);
            if (port == 0)
                fillRect(x - 1, y - 1, 14, 14);
            else
                fillRect(x, y, 14, 14);
            return;
        }

        if (!isConfigured(analog, port)) {
            drawUk(x, y, 13);
            return;
        }

        if (analog) {
            int val = 255 - value(analog, port) / 4;
            setColor(colorFromRGB(val, val, val));
        } else {
            int val = value(analog, port);
            setColor(colorWhite);
            if (val == 1)
                setColor(colorBlack);
        }
        fillRect(x, y, 13, 13);

        if (isAccessed(analog, port)) {
            drawReading(x, y);
        }
    }

    void refreshPorts() {
        for (int i = 0; i < 16; i++)
            if (isTouched(IS_ANALOG, i))
                drawPort(IS_ANALOG, i, 0);
        for (int i = 0; i < 54; i++)
            if (isTouched(IS_DIGITAL, i))
                drawPort(IS_DIGITAL, i, 0);
    }

    void drawPortsTable(const char* title, int rows, int x, int y) {
        setColor(colorDarkGray);
        fillRect(2 + x, 24 + y, 16, 15);
        setColor(colorWhite);
        setPrintX(7 + x);
        setPrintY(23 + y);
        setBigFont();
        print(title);
        setStandardFont();
        setColor(colorDarkerGray);
        for (int i = 0; i < rows; i++) {
            setPrintX(4 + x);
            setPrintY(40 + i * 14 + y);
            print(i);
            print(0);
        }
        for (int i = 0; i < rows + 1; i++) {
            drawLine(20 + x, y + 40 + i * 14 - 2, x + 160, y + 40 + i * 14 - 2);
        }
        for (int i = 0; i < 10; i++) {
            setPrintX(margin + i * 14 + 18 + x);
            setPrintY(26 + y);
            print(i);
        }
        for (int i = 0; i < 11; i++) {
            drawLine(margin + 12 + i * 14 + x, y + 38, x + margin + 12 + i * 14, y + rows * 14 + 38);
        }
    }

    void timeSlice(int fg, frequency frequency) {
        if (!fg)
            return;
        refreshPorts();
        for (int i = 0; i < 16; i++)
            setTouched(IS_ANALOG, i, 0);
        for (int i = 0; i < 54; i++)
            setTouched(IS_DIGITAL, i, 0);
    }

    void drawLegend() {
        setPrintX(200);
        setPrintY(22);
        setBoldStyle();
        setColor(colorDarkerGray);
        println(PSTR("Legend"));
        setNormalStyle();
        setMargins(222, margin);
        setPrintY(46);
        int y = -4;
        for (int i = 0; i < 5; i++)
            drawRect(200, 50 + y + 16 * i, 12, 12);
        int cols[] = {colorBlack, colorWhite, colorWhite, colorWhite, colorWhite};
        for (int i = 0; i < 5; i++) {
            setColor(cols[i]);
            fillRect(201, 67 - 16 + y + 16 * i, 10, 10);
        }
        drawDegrade(201, 67 - 16 + y + 16 * 1, 10);
        drawReading(201, 67 - 16 + y + 16 * 3);
        drawUk(201, 67 - 16 + y + 16 * 4, 10);
        setColor(colorBlack);
        println(PSTR("High"));
        println(PSTR("Analog"));
        println(PSTR("Low"));
        println(PSTR("Accessed"));
        println(PSTR("Unknown"));
    }

    void wd15(int p) {
        _digitalWrite(15, 1);
    }

    void wa100(int p) {
        _analogWrite(7, 599);
    }

    void startScreen() {
        setColor(colorWhite);
        fillRect(21, 39, 140, 84);
        fillRect(21, 158, 140, 27);
        drawPortsTable(PSTR("D"), 6, 0, 0);
        drawPortsTable(PSTR("A"), 2, 0, 119);
        for (int i = 16; i < 20; i++) {
            drawPort(IS_ANALOG, i, UNAVAILABLE);
        }
        for (int i = 54; i < 60; i++) {
            drawPort(IS_DIGITAL, i, UNAVAILABLE);
        }
        for (int i = 0; i < 16; i++) {
            setTouched(IS_ANALOG, i, 1);
        }
        for (int i = 1; i < 54; i++) {
            setTouched(IS_DIGITAL, i, 1);
        }

        drawPort(0, 0, UNAVAILABLE);
        toolbarAddHome();
        drawLegend();
        refreshPorts();
    }

    const char* getLabelForPort(int port, int digital) {
        if (port >= 2 && port <= 12 && digital)
            return PSTR("LCD");
        return 0;
    }

    const char* actionsDef(int kind) {
        if (kind == 0)
            return PSTR("No Action");
        if (kind == 1)
            return PSTR("Activate D. Port [Port]");
        if (kind == 2)
            return PSTR("Deactivate D. Port [Port]");
        if (kind == 3)
            return PSTR("Set Analog Port [Port] with [An. Value]");
        return 0;
    }

    const char* conditionsDef(int kind) {
        if (kind == 0)
            return PSTR("No condition");
        if (kind == 1)
            return PSTR("Digital port [D.Port] set");
        if (kind == 2)
            return PSTR("Digital port [D.Port] not set");
        if (kind == 3)
            return PSTR("Analog port [A.Port] > [A.Value]");
        if (kind == 4)
            return PSTR("Analog port [A.Port] < [A.Value]");
        return 0;
    }

    int evalCondition(int kind, uint16_t* params) {
        // the "default" port 0 is not evaluated
        if (params[0] == 0)
            return 0;
        if (kind == 0)
            return 0;
        if (kind == 1)
            return digitalRead(params[0]);
        if (kind == 2)
            return !digitalRead(params[0]);
        return 0;
    }

    void performAction(int kind, uint16_t* params) {
        if (kind == 2)
            digitalWrite(params[0], 1);
    }

}  // namespace pm

int pmDigitalRead(int port) {
    return pm::_digitalRead(port);
}

void pmDigitalWrite(int port, int value) {
    pm::_digitalWrite(port, value);
}

void pmAnalogWrite(int port, int value) {
    pm::_analogWrite(port, value);
}
