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

#include <stdio.h>

#include "AquaOS.h"

namespace wa {
    void startScreen(void);
    void waWhenTimeSliced(frequency frequency);
    void screenConfigure(void);

    char port;
    char touching;

    int configured() {
        return port;
    }

    /*
     * This is a static method. It is called once for all the instances of this
     * class. You can define:
     * - The cardinality of the application (APP_CARDINALITY_ALWAYS_ONE,
     * APP_CARDINALITY_MAX_ONE, APP_CARDINALITY_UNDEFINED) Default is
     * APP_CARDINALITY_UNDEFINED, meaning any number of instances can exist for the
     * app.
     */

    /*
     * This method is called just once, when the istance is created.
     * It is not called when the controller is restarted.
     */
    void whenCreated() {
        port = 17;
    }

    int state() {
        return port * 10 + touching;
    }

    void registerVars() {
        mregister(&touching);
        pregister(&port);
    }

    void whenPowered() {
        touching = 0;
    }

    /*
     * The name of the conditions.
     * The use of PSTR is required! If not, the app will crash!
     * Return 0 for "no more conditions defined"
     */
    const char* conditionsDef(int kind) {
        if (kind == 0)
            return PSTR("Touching Water");
        if (kind == 1)
            return PSTR("Not Touching Water");
        return 0;
    }

    /*
     * The dashboard is giving this instance the rect defined by
     * the coordinates x,y and the dimensions w,h.
     * The parameter n indicates how many monitors are on the screen.
     * The possible combinations for n, w and h are:
     * (1,320,180), (2, 159,180), (4, 159,89).
     * The rect's background is black.
     * The font is the standard one (you can change it).
     */
    void monitor(int x, int y, int w, int h) {
        setPrintX(x + margin);
        setPrintY(y + margin);
        if (!configured()) {
            setColor(colorWhite);
            drawString(PSTR("Not Configured"), x + margin, y + margin);
        }
        fillRect(x, y, w, h);
        setColor(colorRed);
        if (!touching)
            setColor(colorGreen);
        fillRect(x + 5 * margin, y + 2 * margin, w - 10 * margin, h - 4 * margin);
    }

    // LOG

    void logfn(unsigned char value) {
        if (value)
            print(PSTR("Touching"));
        else
            print(PSTR("Not touching"));
    }

    void screenLog() {
        showLogDialog(startScreen, logfn);
    }

    void formatter(unsigned char value) {
        if (value)
            print(PSTR("Touching"));
        else
            print(PSTR("Not touching"));
    }

    void seeLog(int param) {
        showLogDialog(startScreen, formatter);
    }

    // CONFIGURE SCREEN

    void clickedConfigureActions(int param) {
        showRulesDialog(screenConfigure);
    }

    void configurePortCallback(int selectedPort) {
        if (selectedPort) {
            port = selectedPort;
        }
        goToScreen(screenConfigure);
    }

    void clickedConfigurePort(int param) {
        showDigitalPortDialog(port, configurePortCallback);
    }

    void screenConfigure() {
        printDefinition(PSTR("| Port"));
        println(PSTR("Change the port where the sensor is connected."));
        printDefinition(PSTR("Rules"));
        println(PSTR("Configure Rules for this instance."));
        toolbarAdd(PSTR("| Port"), clickedConfigurePort);
        toolbarAdd(PSTR("| Rules"), clickedConfigureActions);
        toolbarAdd(mini, isenabled, PSTR("<<"), startScreen);
    }

    const char* getLabelForPort(int p, int digital) {
        return configured() && digital && p == port ? PSTR("Water Alarm") : 0;
    }

    void draw() {
        cleanMiddle();

        setColor(colorBlack);
        print(PSTR("Listening on digital port "));
        println(port);
        println();

        setPrintX(50);
        setPrintY(130);
        setColor(colorGreen);
        if (touching)
            setColor(colorRed);

        fillRect(160 - 32, menuBarHeight + 5 * margin, 64, 64);
        setColor(colorBlack);
        setPrintX(64 + 2 * margin);

        if (touching)
            drawCenteredString(PSTR("The sensor is touching water."));
        else
            drawCenteredString(PSTR("The sensor is not touching water."));
    }

    void timeSlice(int fg, frequency frequency) {
        int exTouching = touching;
        if (configured()) {
            pinMode(port, INPUT);
            touching = pmDigitalRead(port);
            if (exTouching != touching)
                logEvent(touching);
        }
        if (!configured() || !currentScreenIs(startScreen) || !fg)
            return;

        draw();
    }

    void startScreen() {
        if (!configured())
            println(PSTR("The Water Level probe is not configured."));
        else
            draw();
        toolbarAdd(PSTR("Configure"), screenConfigure);
        toolbarAdd(standard, configured(), PSTR("Log"), seeLog);
        toolbarAddHome();
    }
}  // namespace wa
