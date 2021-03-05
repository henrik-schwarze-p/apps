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

namespace desktop {
    int page = 0;

    int cardinallity() {
        return APP_CARDINALITY_ALWAYS_ONE;
    }

    void previousPage(int param) {
        page--;
        goToScreen(startScreen);
    }

    void nextPage(int param) {
        page++;
        goToScreen(startScreen);
    }

    void setCurrentApp(int appNumber) {
        int instanceNumber = visibleInstance(appNumber + page * 6);
        if (instanceNumber == -1)
            return;
        setForegroundInstance(instanceNumber);
        printTitle(nameForInstance(instanceNumber));
        setJustOpened(1);
        switchContextToInstance(instanceNumber);
        goToScreen(screenForInstance(instanceNumber));
        popContext();
    }

    void startScreen() {
        setForegroundInstance(instanceForId(DESKTOP_ID));
        printTitle(PSTR("AquaOS"));
        for (int a = page * 6; a < page * 6 + 6 && a < numberOfVisibleInstances(); a++) {
            int xCoordinate = 60;
            int i = a - page * 6;
            if (i % 3 == 1)
                xCoordinate = 160;
            if (i % 3 == 2)
                xCoordinate = 320 - 60;
            int      yCoordinate = 70 + 90 * (i / 3) + 8;
            int      appIndex = visibleInstance(a);
            uint16_t id = idForInstance(appIndex);
            int      icon = getDescriptorIndex(id);
            int      y = yCoordinate;
            int      suffix = getRepetitionForInstance(appIndex);
            suffix += suffix > 0;  // we start at two
            drawIcon(icon,
                     xCoordinate - iconWidth(icon) / 2,
                     70 + 90 * (i / 3) + 8 - iconHeight(icon) - 30 + iconHeight(icon) / 2);
            if (chunkForInstanceAndHandleExists(appIndex, NAME_HANDLE))
                addLabel(xCoordinate - 45, y, appIndex, NAME_HANDLE, 90, fontHeight(), colorBlack, STYLE_CENTERED,
                         suffix);
            else
                addLabel(xCoordinate - 45, y, nameForType(id), 90, fontHeight(), colorBlack, STYLE_CENTERED, suffix);
        }
        addButton(30, 25, 60, 70, setCurrentApp, 0, 3, 2, 40, 20, -1);
        toolbarAdd(standard, page > 0, MINI_ICON_LEFT, previousPage);
        toolbarAdd(standard, page * 6 + 6 < numberOfVisibleInstances(), MINI_ICON_RIGHT, nextPage);
    }

}  // namespace desktop
