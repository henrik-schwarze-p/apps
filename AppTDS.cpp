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

namespace tds {

    // PERSISTANT VARS

    char port;                      // 0 == not configured
    int  calibratedAnalogValue;     // -1 == not configured
    int  calibrationSolutionValue;  // -1 == not configured

    // actions and screens
    void startScreen();
    void configure();

    void whenCreated() {
        port = 10;
        calibratedAnalogValue = 313;
        calibrationSolutionValue = 100;
    }

    void registerVars() {
        pregister(&port);
        pregister(&calibratedAnalogValue);
        pregister(&calibrationSolutionValue);
    }

    float tds(unsigned int analogValue) {
        return analogValue * calibrationSolutionValue / calibratedAnalogValue;
    }

    const char* conditionsDef(int kind) {
        if (kind == 0)
            return PSTR("TDS < [TDS Value]");
        if (kind == 1)
            return PSTR("TDS >= [TDS Value]");
        return 0;
    }

    int evalCondition(int kind, uint16_t* params) {
        if (kind == 0)
            return tds(analogRead(port)) < params[0];
        if (kind == 1)
            return tds(analogRead(port)) >= params[0];
        return 0;
    }

    // RULES

    void rules(int param) {
        showRulesDialog(startScreen);
    }

    // CONFIGURING ANALOG PORT

    void portSet(int p) {
        if (p)
            port = p;
        goToScreen(configure);
    }

    void portDialog(int param) {
        showAnalogPortDialog(port, portSet);
    }

    void solutionSet(int valid, float value) {
        if (valid)
            calibrationSolutionValue = (int)value;
        goToScreen(configure);
    }

    void solution(int params) {
        showNumPad(PSTR("Solution TDS Value"), calibrationSolutionValue, 0.1, 1000, solutionSet, 0);
    }

    void read(int params) {
        calibratedAnalogValue = analogRead(port);
        message(PSTR("The value was read"), configure);
    }

    void configure() {
        print(PSTR("The configuration consists of three steps. First, set an analog port. Then, calibrate the probe "));
        println(PSTR("by entering the solution TDS value and then put the probe in the solution, to read its value."));
        println();
        print(PSTR("  > "));
        if (port) {
            print(PSTR("The current analog port is "));
            println(port);
        } else {
            println(PSTR("Port is not set"));
        }
        setPrintY(printY() + margin);

        print(PSTR("  > "));
        if (calibrationSolutionValue) {
            print(PSTR("The solution TDS value is "));
            println(calibrationSolutionValue);
        } else {
            println(PSTR("No value for the solution TDS has been entered"));
        }
        setPrintY(printY() + margin);

        print(PSTR("  > "));
        if (calibratedAnalogValue != -1) {
            print(PSTR("The analog reading is "));
            println(calibratedAnalogValue);
        } else
            println(PSTR("There is no analog reading yet using the probe"));

        toolbarAdd(PSTR("| Port"), portDialog);
        toolbarAdd(PSTR("| Solution"), solution);
        toolbarAdd(PSTR("| Probe"), read);
        toolbarAdd(mini, 1, PSTR("<<"), startScreen);
    }

    void monitor(int x, int y, int w, int h) {
        if (!port)
            return;
        fillRect(x, y, w, h);
        setColor(colorWhite);
        setPrintX(x + 69);
        setPrintY(y + 30);
        print(tds(analogRead(port)));
    }

    int state() {
        return analogRead(port);
    }

    void showState() {
        setColor(colorBlack);
        print(PSTR("Listening on analog port "));
        println(port);
        setBoldStyle();
        setPrintX(130);
        setPrintY(100);
        setNormalStyle();
        print(PSTR("TDS: "));
        cleanRestOfLine();
        println(tds(analogRead(port)));
    }

    void timeSlice(int fg, frequency frequency) {
        if (frequency.type == day)
            logEvent(analogRead(port) / 4);
        if (fg && currentScreenIs(startScreen) && port && calibratedAnalogValue != -1 && calibrationSolutionValue != -1)
            showState();
    }

    void logFormatter(unsigned char val) {
        print(val * 4);
    }

    void log(int param) {
        showLogDialog(startScreen, logFormatter);
    }

    void startScreen() {
        if (port && calibratedAnalogValue != -1 && calibrationSolutionValue != -1)
            showState();
        else
            println(PSTR("Please, configure the nanoapp."));
        toolbarAdd(PSTR("Configure"), configure);
        toolbarAdd(standard, port, PSTR("Rules"), rules);
        toolbarAdd(standard, port, PSTR("Log"), log);
        toolbarAddHome();
    }

}
