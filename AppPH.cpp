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

namespace ph {
    // PERSISTANT VARS
    char  port;
    int   analogValueFor40, analogValueFor70;
    float real40, real70;

    // VOLATILE VARS
    float last10[10];

    void startScreen();
    void screenConfigure();
    void screenPoints();
    void screenSolution();

    // IMPLEMENTATION

    void registerVars() {
        // mregister(last10, 10);
        pregister(&port);
        pregister(&analogValueFor40);
        pregister(&analogValueFor70);
        pregister(&real40);
        pregister(&real70);
    }

    void whenCreated() {
        port = analogValueFor40 = analogValueFor70 = 0;
        real40 = 4.1;
        real70 = 7.65;
        //
        port = 13;
        analogValueFor70 = 500;
        analogValueFor40 = 200;
    }

    void whenPowered() {
        for (int i = 0; i < 10; i++)
            last10[i] = 2 + i * 3;
    }

    // FUNCTIONS
    int configured() {
        return port * analogValueFor70 * analogValueFor40;
    }

    float ph() {
        if (analogValueFor40 * analogValueFor70 == 0 || analogValueFor40 == analogValueFor70)
            return 7.0;
        return (analogRead(port) - analogValueFor40) * (real70 - real40) / (analogValueFor70 - analogValueFor40) +
               real40;
    }

    // CONDITIONS
    const char* conditionsDef(int kind) {
        if (kind == 0)
            return PSTR("PH < [PH Value]");
        if (kind == 1)
            return PSTR("PH > [PH Value]");
        return 0;
    }

    int evalCondition(int kind, uint16_t* params) {
        if (kind == 0)
            return ph() < params[0] / 10;
        if (kind == 1)
            return ph() > params[0] / 10;
        return 0;
    }

    void setConditionParameterCont(int success, float value) {
        continueSetConditionParameter(success, (int)(value * 10));
    }

    void setConditionParameter(int condition, int parameter) {
        showNumPad(PSTR("Enter the PH Value"), 7.0, 1.0, 13.0, setConditionParameterCont, 1);
    }

    void printConditionParameter(int condition, int parameter, uint16_t phValue) {
        print(phValue / (float)10.0);
    }

    // MONITOR
    void monitor(int x, int y, int w, int h) {
        fillRect(x, y, w, h);
        if (configured()) {
            setColor(colorWhite);
            setPrintX(x + 69);
            setPrintY(y + 30);
            print(ph());
        }
    }

    // LOG
    void logFormatter(unsigned char val) {
        print(val * 4);
    }

    void log(int param) {
        showLogDialog(startScreen, logFormatter);
    }

    // SHOW STATE
    void showCalibrationPoints() {
        print(PSTR("> Low calibration point is: "));
        println(real40);
        println();
        print(PSTR("> High calibration point is: "));
        println(real70);
        println();
    }

    void showAnalogValues() {
        if (analogValueFor40) {
            print(PSTR("> Solution reading for low point is: "));
            println(analogValueFor40);
            println();
        } else {
            pushColor();
            setColor(colorDarkGray);
            println(PSTR("> No solution reading for low point."));
            println();
            popColor();
        }
        if (analogValueFor70) {
            print(PSTR("> Solution reading for high point is: "));
            println(analogValueFor70);
            println();
        } else {
            pushColor();
            setColor(colorDarkGray);
            println(PSTR("> No solution reading for high point."));
            popColor();
            println();
        }
    }

    void showState() {
        setColor(colorBlack);
        if (port) {
            print(PSTR("Listening on analog port: "));
            println(port);
            println();
        } else {
            println(PSTR("No port set."));
            println();
        }
        showCalibrationPoints();
        showAnalogValues();
    }

    // DIALOGS
    void rules(int param) {
        showRulesDialog(startScreen);
    }

    void setPortCont(int p) {
        if (p)
            port = p;
        goToScreen(screenConfigure);
    }

    void setPort(int param) {
        showAnalogPortDialog(port, setPortCont);
    }

    void setSolutionHigh(int param) {
        analogValueFor70 = analogRead(port);
        message(PSTR("High point calibrated."), screenSolution, DRAWSCREEN_CLEAR_MIDDLE);
    }

    void setSolutionLow(int param) {
        analogValueFor40 = analogRead(port);
        message(PSTR("Low point calibrated."), screenSolution, DRAWSCREEN_CLEAR_MIDDLE);
    }

    void setHighPointCont(int success, float v) {
        if (success) {
            real70 = v;
            message(PSTR("Set high point"), screenPoints);
        } else
            goToScreen(screenPoints);
    }

    void setHighPoint(int param) {
        showNumPad(PSTR("Select the high PH point (usually 7.1 or similar)."), 4.1, real40, 13.0, setHighPointCont, 1);
    }

    void setLowPointCont(int success, float v) {
        if (success) {
            real40 = v;
            message(PSTR("Set low point"), screenPoints);
        } else
            goToScreen(screenPoints);
    }

    void setLowPoint(int param) {
        showNumPad(PSTR("Select the low PH point (usually 4.1 or similar)."), 4.1, 2.0, real70, setLowPointCont, 1);
    }

    // NAVIGATION SCREENS
    void screenSolution() {
        println(PSTR(
            "You should put the probe in the low PH calibration solution and in the high PH calibration solution."));
        println();
        showAnalogValues();
        toolbarAdd(PSTR("Low"), setSolutionLow);
        toolbarAdd(PSTR("High"), setSolutionHigh);
        toolbarAdd(mini, 1, PSTR("<<"), screenConfigure);
    }

    void screenPoints() {
        println(
            PSTR("To calibrate the probe, you should set the calibration points, that is, the PH of the calibration "
                 "solutions. Normally, the low calibration point is around 4.0, and the high about 7.0."));
        println();
        showCalibrationPoints();
        toolbarAdd(PSTR("Low"), setLowPoint);
        toolbarAdd(PSTR("High"), setHighPoint);
        toolbarAdd(mini, 1, PSTR("<<"), screenConfigure);
    }

    int state() {
        if (!configured())
            return STATE_NOT_CONFIGURED;
        return ph();
    }

    void showPH() {
        if (!port || !analogValueFor70 || !analogValueFor40) {
            println(
                PSTR("The app is not calibrated. Press Configure to set the port, the calibration points and calibrate "
                     "using calibration solutions."));
            return;
        }
        print(PSTR("Listening on analog port: "));
        print(port);
        setPrintX(140);
        setPrintY(100);
        print(PSTR("PH: "));
        cleanRestOfLine();
        print(ph());
    }

    void screenConfigure() {
        showState();
        toolbarAdd(PSTR("| Port"), setPort);
        toolbarAdd(PSTR("| Points"), screenPoints);
        toolbarAdd(standard, port, PSTR("| Solution"), screenSolution);
        toolbarAdd(mini, 1, PSTR("<<"), startScreen);
    }

    // TIME SLICE
    void timeSlice(int fg, frequency frequency) {
        if (fg && currentScreenIs(startScreen) && configured())
            showPH();
    }

    // START SCREEN
    void startScreen() {
        showPH();
        toolbarAdd(PSTR("Configure"), screenConfigure);
        toolbarAdd(PSTR("Rules"), rules);
        toolbarAdd(PSTR("Log"), log);
        toolbarAddHome();
    }

}  // namespace ph
