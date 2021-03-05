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

#define SOLENOID_UPDATE 1

namespace solenoid {
    char port;
    char currentState;
    char goalState;
    char remaining;

    char updated;
    int  time;

    void startScreen();

    void registerVars() {
        pregister(&port);
        pregister(&time);
        mregister(&goalState);
        mregister(&remaining);
        mregister(&currentState);
    }

    void whenCreated() {
        port = 14;
        time = 20;
    }

    void whenPowered() {
        currentState = 0;
        goalState = 0;
        remaining = 0;
    }

    int configured() {
        return port;
    }

    void showState() {
        setColor(colorBG);
        fillRect(0, menuBarHeight + 1, 320, 150);
        setColor(colorBlack);
        print(PSTR("State: "));
        if (currentState)
            print(PSTR("ON"));
        else
            print(PSTR("OFF"));
        if (goalState != currentState) {
            if (goalState)
                print(PSTR(" (ON waiting)"));
            else
                print(PSTR(" (OFF waiting)"));
        }
        println();
        println();
        if (remaining - time > 0) {
            print(PSTR("Ready to be changed."));
        } else {
            updated = SOLENOID_UPDATE;
            print(PSTR("Remaining "));
            print(time - remaining);
            print(PSTR(" seconds."));
        }
    }

    void timeSlice(int fg, frequency frequency) {
        if (!configured())
            return;

        // Business logic
        remaining++;
        if (remaining > time && goalState != currentState) {
            remaining = 0;
            currentState = goalState;
            pmDigitalWrite(port, goalState);
        }

        // Should we draw?
        if (!fg || !currentScreenIs(startScreen))
            return;
        showState();
    }

    const char* actionsDef(int kind) {
        if (kind == 0)
            return PSTR("Turn On at Least 1 minute");
        if (kind == 1)
            return PSTR("Turn Off at Least 1 minute");
        return 0;
    }

    void performAction(int kind, uint16_t* params) {
        if (kind == 0)
            goalState = 1;
        if (kind == 1)
            goalState = 0;
    }

    void portCallback(int p) {
        if (p > 0) {
            port = p;
            currentState = 0;
        }
        goToScreen(startScreen);
        updated = SOLENOID_UPDATE;
    }

    void configurePort(int param) {
        showDigitalPortDialog(port, portCallback);
    }

    void on(int param) {
        updated = SOLENOID_UPDATE;
        performAction(0, 0);
    }

    void off(int param) {
        updated = SOLENOID_UPDATE;
        performAction(1, 0);
    }

    void numEntered(int success, float value) {
        if (success) {
            time = (int)value;
        }
        remaining = 0;
        goToScreen(startScreen);
    }

    void configure(int param) {
        showNumPad(PSTR("How many seconds between switches?"), time, 5.0, 100000, numEntered, 0);
    }

    void startScreen() {
        if (!configured()) {
            println(
                PSTR("This app allows to turn a digital port ON or OFF, but ensures "
                     "that at most one state change "
                     "takes place every 5 seconds"));
            if (!configured()) {
                println();
                println(PSTR("Not configured. Configure a digital port."));
            }
        } else {
            showState();
        }

        updated = SOLENOID_UPDATE;
        toolbarAdd(standard, configured(), PSTR("  On  "), on);
        toolbarAdd(standard, configured(), PSTR(" Off "), off);
        toolbarAdd(PSTR("|Time"), configure);
        toolbarAdd(standard, isenabled, PSTR("| Port"), configurePort);
        toolbarAddHome();
    }

}  // namespace solenoid
