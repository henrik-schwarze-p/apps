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

#define TOUCHING 0

namespace led {
    void startScreen();

    char     port;
    char     portOn;
    uint16_t counter;

    int configured() {
        return port;
    }

    void registerVars() {
        pregister(&port);
        mregister(&portOn);
        mregister(&counter);
    }
    void whenCreated() {
        port = 0;
        counter = 0;
    }

    // CHOOSE PORT

    void configurePortCallback(int selectedPort) {
        if (selectedPort) {
            port = selectedPort;
            pinMode(port, OUTPUT);
        }
        goToScreen(startScreen);
    }

    void screenConfigure(int param) {
        showDigitalPortDialog(port, configurePortCallback);
    }

    // ACTIONS

    void setActionParameterCont(int success, float value) {
        continueSetConditionParameter(success, (int)(value));
    }

    void setActionParameter(int action, int parameter) {
        showNumPad(PSTR("Enter how many seconds"), 0, 0, 50000, setActionParameterCont, 0);
    }

    const char* actionsDef(int kind) {
        if (kind == 0)
            return PSTR("Turn LED On");
        else if (kind == 1)
            return PSTR("Turn LED Off");
        if (kind == 2)
            return PSTR("Turn LED On [Sec] Seconds");
        return 0;
    }

    void performAction(int kind, uint16_t* params) {
        if (kind == 0) {
            portOn = 1;
        }
        if (kind == 1) {
            portOn = 0;
        }
        if (kind == 2) {
            portOn = 1;
            counter = params[0];
        }
        pmDigitalWrite(port, portOn);
    }

    // Toolbar actions

    void turnOn(int param) {
        performAction(0, 0);
        goToScreen(startScreen);
    }

    void turnOff(int param) {
        performAction(1, 0);
        goToScreen(startScreen);
    }

    void timeSlice(int fg, frequency frequency) {
        if (counter == 1)
            turnOff(0);
        if (counter > 0)
            counter--;
    }

    void startScreen() {
        if (!configured()) {
            println(PSTR("Not configured, please set the port."));
        } else {
            print(PSTR("Configured on port "));
            print(port);
            println(PSTR("."));
            print(PSTR("State: "));
            if (portOn)
                print(PSTR("On"));
            else
                print(PSTR("Off"));
        }
        toolbarAdd(PSTR("| Port"), screenConfigure);
        toolbarAdd(standard, configured(), PSTR("  On   "), turnOn);
        toolbarAdd(standard, configured(), PSTR("  Off  "), turnOff);
        toolbarAddHome();
    }

    const char* getLabelForPort(int p, int digital) {
        return digital && port == p ? PSTR("LED") : 0;
    }
}  // namespace led
