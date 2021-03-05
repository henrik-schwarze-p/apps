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

namespace timer {
    void startScreen();

    int cardinality() {
        return APP_CARDINALITY_MAX_ONE;
    }

    void setConditionParameterCont(int success, float value) {
        continueSetConditionParameter(success, (int)(value));
    }

    void setConditionParameter(int condition, int parameter) {
        if (condition == 0) {
            if (parameter == 0)
                showNumPad(PSTR("Enter the Hours (0-23)"), 0, 0, 23, setConditionParameterCont, 0);
            if (parameter == 1)
                showNumPad(PSTR("Enter the Minutes (0-59)"), 0, 0, 59, setConditionParameterCont, 0);
            if (parameter == 2)
                showNumPad(PSTR("Enter the Seconds (0-59)"), 0, 0, 59, setConditionParameterCont, 0);
        }
        if (condition == 1) {
            if (parameter == 0)
                showNumPad(PSTR("Enter the Number (1-...)"), 1, 1, 60000, setConditionParameterCont, 0);
            if (parameter == 1)
                showNumPad(PSTR("Enter the Second Number"), 0, 0, 60000, setConditionParameterCont, 0);
        }
    }

    const char* conditionsDef(int kind) {
        if (kind == 0)
            return PSTR("Time is [Hour]:[Min]:[Secs]");
        if (kind == 1)
            return PSTR("Seconds mod [Number] == [Number2]");
        return 0;
    }

    int evalCondition(int kind, uint16_t* params) {
        if (kind == 0)
            return params[2] == clockSecs() && params[0] == clockHours() && params[1] == clockMins();
        if (kind == 1) {
            if (params[0]==0)
                return 0;
            return clockEpoch() % params[0] == params[1];
        }
        return 0;
    }

    int state() {
        return 1;
    }

    void changeRules(int param) {
        showRulesDialog(startScreen);
    }

    void startScreen() {
        println(PSTR("The Timer has no state, it only provides conditions"));
        toolbarAdd(PSTR("| Rules"), changeRules);
        toolbarAddHome();
    }

}
