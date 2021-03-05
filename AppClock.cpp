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

namespace cc {

    void          startScreen();
    unsigned char first = 1;

    int cardinality() {
        return APP_CARDINALITY_ALWAYS_ONE;
    }

    int ccToolbarSet = 0;

    void print2Digits(int number, const char* suffix) {
        print(number, 2);
        print(suffix);
    }

    void ccDrawClock() {
        setBigFont();
        setColor(colorDarkGray);
        fillRect(85, 68, 160, 22);
        setColor(colorBlack);
        drawRect(85, 68, 160, 22);
        setColor(colorWhite);
        setPrintX(95);
        setPrintY(70);
        print(clockYear() + 2000);
        print(PSTR("-"));
        print2Digits(clockMonth(), PSTR("-"));
        print2Digits(clockDay(), PSTR("  "));
        print2Digits(clockHours(), PSTR(":"));
        print2Digits(clockMins(), PSTR(":"));
        print2Digits(clockSecs(), PSTR(""));
    }

    void setToolbarSet(int param) {
        ccToolbarSet = param;
        goToScreen(startScreen);
    }

    int correctValue(int value, int min, int max) {
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    }

    void setConditionParameterCont(int success, float value) {
        continueSetConditionParameter(success, (int)(value));
    }

    void setConditionParameter(int condition, int parameter) {
        if (parameter == 0)
            showNumPad(PSTR("Enter the Hours (0-23)"), 0, 0, 23, setConditionParameterCont, 0);
        if (parameter == 1)
            showNumPad(PSTR("Enter the Minutes (0-59)"), 0, 0, 59, setConditionParameterCont, 0);
    }

    const char* conditionsDef(int kind) {
        return !kind ? PSTR("Time is [Hour]:[Min]") : 0;
    }

    int evalCondition(int kind, uint16_t* params) {
        return 0 == clockSecs() && params[0] == clockHours() && params[1] == clockMins();
    }

    void addUnit(int addYear, int addMonth, int addDay, int addHours, int addMins, int addSecs) {
        setClockYear(correctValue(clockYear() + addYear, 0, 100));
        setClockMonth(correctValue(clockMonth() + addMonth, 1, 12));
        setClockHours(correctValue(clockHours() + addHours, 0, 23));
        setClockMins(correctValue(clockMins() + addMins, 0, 59));
        setClockSecs(correctValue(clockSecs() + addSecs, 0, 59));
        int daysInMonth = clockMonth() == 2 ? 28 + (clockYear() % 4 == 0 ? 1 : 0) : 31 - (clockMonth() - 1) % 7 % 2;
        setClockDay(correctValue(clockDay() + addDay, 1, daysInMonth));
        ccDrawClock();
    }

    void addHours(int param) {
        addUnit(0, 0, 0, param, 0, 0);
    }

    void addMins(int param) {
        addUnit(0, 0, 0, 0, param, 0);
    }

    void addSecs(int param) {
        addUnit(0, 0, 0, 0, 0, param);
    }

    void addYear(int param) {
        addUnit(param, 0, 0, 0, 0, 0);
    }

    void addMonth(int param) {
        addUnit(0, param, 0, 0, 0, 0);
    }

    void addDay(int param) {
        addUnit(0, 0, param, 0, 0, 0);
    }

    void genericScreen(const char* labelPlus, const char* labelMinus, void (*fn)(int)) {
        ccDrawClock();
        toolbarAdd(labelPlus, fn, 1);
        toolbarAdd(labelMinus, fn, -1);
        toolbarAdd(PSTR("<<"), startScreen);
    }

    void screenHour() {
        genericScreen(PSTR("+ Hours"), PSTR("- Hours"), addHours);
    }

    void screenMin() {
        genericScreen(PSTR("+ Mins"), PSTR("- Mins"), addMins);
    }

    void screenSec() {
        genericScreen(PSTR("+ Secs"), PSTR("- Secs"), addSecs);
    }

    void screenYear() {
        genericScreen(PSTR("+ Year"), PSTR("- Year"), addYear);
    }

    void screenMonth() {
        genericScreen(PSTR("+ Month"), PSTR("- Month"), addMonth);
    }

    void screenDay() {
        genericScreen(PSTR("+ Day"), PSTR("- Day"), addDay);
    }

    void startScreen() {
        println(PSTR("The current time is:"));
        if (ccToolbarSet == 0) {
            toolbarAdd(PSTR("| Hour"), screenHour);
            toolbarAdd(PSTR("| Min"), screenMin);
            toolbarAdd(PSTR("| Sec"), screenSec);
            toolbarAdd(mini, isenabled, PSTR("..."), setToolbarSet, 1);
            toolbarAddHome();
        }
        if (ccToolbarSet == 1) {
            toolbarAdd(PSTR("| Year"), screenYear);
            toolbarAdd(PSTR("| Month"), screenMonth);
            toolbarAdd(PSTR("| Day"), screenDay);
            toolbarAdd(mini, isenabled, PSTR("..."), setToolbarSet, 0);
            toolbarAddHome();
        }
        setPrintY(120);
        println(PSTR("Press '...' to set different time units."));
        ccDrawClock();
    }

    void timeSlice(int fg, frequency frequency) {
        // The time, top right
        setBigFont();
        setPrintX(250);
        setMaxPrintX(319);
        setPrintY(0);
        setColor(colorWhite);
        // no flickering
        if (clockSecs() > 0 && !first) {
            int x = 250;
            x += intWidth(clockHours() / 10) + intWidth(clockHours() % 10) + stringWidth(PSTR(":"));
            x += intWidth(clockMins() / 10) + intWidth(clockMins() % 10) + stringWidth(PSTR(":"));
            setPrintX(x);
            fillRect(x, 0, 20, menuBarHeight);
            setColor(colorBlack);
        } else {
            fillRect(250, 0, horizontalResolution - 250, menuBarHeight);
            setColor(colorBlack);
            print2Digits(clockHours(), PSTR(":"));
            print2Digits(clockMins(), PSTR(":"));
        }
        first = 0;
        print2Digits(clockSecs(), PSTR(""));
        if (!currentScreenIs(startScreen) && !currentScreenIs(screenHour) && !currentScreenIs(screenMin) &&
            !currentScreenIs(screenSec) && !currentScreenIs(screenYear) && !currentScreenIs(screenMonth) &&
            !currentScreenIs(screenDay))
            return;
        ccDrawClock();
        setMaxPrintX(319 - margin);
    }
}  // namespace cc
