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

namespace monitor {

    char page = 0;
    char first = 0;
    void startScreen(void);

    int cardinality() {
        return APP_CARDINALITY_MAX_ONE;
    }

    int numberOfMonitoreableApps() {
        int r = 0;
        for (int i = 0; i < numberOfInstances(); i++)
            if (hasMonitorForInstance(i))
                r++;
        return r;
    }

    int monitoreable(int index) {
        for (int i = 0; i < numberOfInstances(); i++) {
            if (hasMonitorForInstance(i)) {
                if (index == 0)
                    return i;
                index--;
            }
        }
        return -1;
    }

    void drawInstance(int i, int x, int y, int w, int h, int n, int bg) {
        int instance = monitoreable(i);
        if (instance == -1)
            return;
        int dashColor = colorFromRGB(14 * 255 / 100, 21 * 255 / 100, 35 * 255 / 100);
        setColor(dashColor);
        if (first) {
            fillRect(x, y, w, h);
            int dashColor = colorFromRGB(9 * 255 / 100, 14 * 255 / 100, 23 * 255 / 100);
            setColor(dashColor);
            fillRect(x, y, w, 16);

            int lineColor = colorFromRGB(11 * 255 / 100, 17 * 255 / 100, 28 * 255 / 100);
            setColor(colorFromRGB(20 * 255 / 100, 36 * 255 / 100, 51 * 255 / 100));
            setColor(lineColor);
            setStandardFont();
            setPrintX(x + w / 2 - stringWidth(nameForInstance(instance)) / 2);
            setPrintY(y + 2);
            setColor(colorWhite);
            print(nameForInstance(instance));
            setPrintX(x + w / 2 - stringWidth(nameForInstance(instance)) / 2 + 1);
            print(nameForInstance(instance));
        }
        setColor(dashColor);
        callMonitor(instance, x, y + 16, w, h - 16, first);
    }

    void draw() {
        int m = menuBarHeight + 1;
        setColor(colorMonitor);
        if (first)
            fillRect(0, m, 320, verticalResolution - menuBarHeight - toolbarHeight);
        if (listenToTouchesOnScreen())
            return;
        drawInstance(page, 0, m, 160, 90, 5, 0);
        if (listenToTouchesOnScreen())
            return;
        drawInstance(page + 1, 160, m, 160, 90, 4, 1);
        if (listenToTouchesOnScreen())
            return;
        drawInstance(page + 2, 0, m + 90, 160, 89, 4, 1);
        if (listenToTouchesOnScreen())
            return;
        drawInstance(page + 3, 160, m + 90, 160, 89, 4, 0);
        first = 0;
    }

    void timeSlice(int fg, frequency frequency) {
        // We drive every 5 seconds, only if the current screen is ours (if
        // not we would draw on top of all the other apps)
        if (!fg || frequency.type != second)
            return;
        draw();
    }

    void left(int param) {
        if (page > 0)
            page -= 4;
        goToScreen(startScreen);
    }

    void right(int param) {
        page += 4;
        goToScreen(startScreen);
    }

    void startScreen() {
        toolbarAdd(standard, page > 0, MINI_ICON_LEFT, left);
        toolbarAdd(standard, page + 4 < numberOfMonitoreableApps(), MINI_ICON_RIGHT, right);
        toolbarAddHome();
        first = 1;
        draw();
    }

}  // namespace monitor
