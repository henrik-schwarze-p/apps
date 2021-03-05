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

namespace button {
    void startScreen();
    void show();

    char port;

    // CALLBACKS

    void whenCreated() {
        port = 0;
    }

    void registerVars() {
        pregister(&port);
    }

    void whenPowered() {
        if (port)
            pinMode(port, INPUT);
    }

    int configured() {
        return port;
    }

    const char* getLabelForPort(int p, int digital) {
        return digital && port == p ? PSTR("Button") : 0;
    }

    const char* conditionsDef(int kind) {
        if (kind == 0)
            return PSTR("Button pressed");
        if (kind == 1)
            return PSTR("Button not pressed");
        return 0;
    }

    int evalCondition(int kind, uint16_t* param) {
        if (!port)
            return 0;
        if (kind == 0)
            return digitalRead(port);
        return !digitalRead(port);
    }

    int state() {
        if (!configured())
            return STATE_NOT_CONFIGURED;
        return pmDigitalRead(port);
    }

    void timeSlice(int fg, frequency frequency) {
        if (fg && currentScreenIs(startScreen))
            show();
    }

    // GUI

    void setPort(int selectedPort) {
        if (selectedPort) {
            port = selectedPort;
            pinMode(port, INPUT);
        }
        goToScreen(startScreen);
    }

    void changePort(int param) {
        showDigitalPortDialog(port, setPort);
    }

    void changeRules(int param) {
        showRulesDialog(startScreen);
    }

    void show() {
        if (!configured())
            println(PSTR("Port not set."));
        else {
            print(PSTR("Listening on digital port "));
            println(port);
        }
        if (!configured())
            return;
        cleanRestOfLine();
        if (pmDigitalRead(port))
            print(PSTR("Pressed"));
        else
            print(PSTR("Not pressed"));
    }

    void startScreen() {
        show();
        toolbarAdd(PSTR("| Port"), changePort);
        toolbarAdd(PSTR("| Rules"), changeRules);
        toolbarAddHome();
    }

}  // namespace button
