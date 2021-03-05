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

namespace dice {

    long next;

    void startScreen();

    void drawDicePoint(int x, int y) {
        setColor(colorBlack);
        int d[] = {3, 2, 1, 0, 0, 0, 0, 1, 2, 3};
        for (int i = 0; i < 10; i++)
            drawLine(x + d[i] - 5, y + i - 5, x + 5 - d[i], y + i - 5);
    }

    void drawDice(int x, int y, int w, int h) {
        setColor(colorDarkGray);
        drawRect(x, y, w, h);
        drawRect(x + 1, y + 1, w - 2, h - 2);
        setColor(colorWhite);
        fillRect(x + 2, y + 2, w - 4, h - 4);
        int diceLast = random(6) + 1;
        int d = 4 + w / 7;
        int x0 = x + d, x1 = x + w / 2, x2 = x + w - d;
        int y0 = y + d, y1 = y + h / 2, y2 = y + h - d;
        if (diceLast == 1 || diceLast == 5 || diceLast == 3)
            drawDicePoint(x1, y1);
        if (diceLast != 1) {
            drawDicePoint(x0, y0);
            drawDicePoint(x2, y2);
        }
        if (diceLast == 4 || diceLast == 5 || diceLast == 6) {
            drawDicePoint(x0, y2);
            drawDicePoint(x2, y0);
        }
        if (diceLast == 6) {
            drawDicePoint(x0, y1);
            drawDicePoint(x2, y1);
        }
    }

    void throwDice(int param) {
        if (param == 2) {
            drawDice(20, 40, 130, 130);
            drawDice(170, 40, 130, 130);
        } else if (param == 4) {
            drawDice(50, 20, 80, 80);
            drawDice(190, 20, 80, 80);
            drawDice(50, 110, 80, 80);
            drawDice(190, 110, 80, 80);
        } else {
            drawDice(80, 26, 160, 160);
        }
    }

    void monitor(int x, int y, int w, int h) {
        drawDice(x + 5 + w / 2 - h / 2, y + 10, min(w, h) - 20, min(w, h) - 20);
    }

    void one(int param) {
        cleanMiddle();
        throwDice(1);
    }

    void two(int param) {
        cleanMiddle();
        throwDice(2);
    }

    void four(int param) {
        cleanMiddle();
        throwDice(4);
    }

    void startScreen() {
        next = millis();
        toolbarAdd(PSTR(" Throw 1 "), one);
        toolbarAdd(PSTR(" Throw 2 "), two);
        toolbarAdd(PSTR(" Throw 4 "), four);
        toolbarAddHome();
    }

}  // namespace dice
