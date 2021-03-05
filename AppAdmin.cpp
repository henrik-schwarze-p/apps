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

extern long loadCumulativeDuration;
extern long loadTotalDuration;
namespace admin {

    char _alarmPort;

    void startScreen();

    int cardinality() {
        return APP_CARDINALITY_ALWAYS_ONE;
    }

    void registerVars() {
        pregister(&_alarmPort);
    }

    void whenCreated() {
        _alarmPort = 0;
    }

    // ADD

    char alarmPort() {
        return _alarmPort;
    }

    int canAdd(int type) {
        if (cardinalityForType(type) == APP_CARDINALITY_UNDEFINED)
            return 1;
        if (cardinalityForType(type) == APP_CARDINALITY_ALWAYS_ONE)
            return 0;
        for (int i = 0; i < numberOfInstances(); i++)
            if (idForInstance(i) == type)
                return 0;
        return 1;
    }

    int getAddableIdIndex(int a) {
        for (int i = 0; i < NUMBER_OF_DESCRIPTORS; i++)
            if (canAdd(getDescriptor(i)))
                if (a-- == 0)
                    return i;
        return -1;
    }

    int numberOfIdsThatCanBeAdded() {
        int i = 0;
        while (getAddableIdIndex(i) >= 0)
            i++;
        return i;
    }

    void screenAdded() {
        int iconNumber = getDescriptorIndex(lastInstance());
        drawIcon(iconNumber, 160 - iconWidth(iconNumber) / 2, menuBarHeight + margin * 4);
        setColor(colorBlack);
        setPrintY(120);
        int w = stringWidth(PSTR("The app "));
        w += stringWidth(nameForInstance(lastInstance()));
        w += stringWidth(PSTR(" was added."));
        setPrintX(160 - w / 2);
        print(PSTR("The app "));
        setBoldStyle();
        print(nameForInstance(lastInstance()));
        setNormalStyle();
        print(PSTR(" was added."));
        toolbarAdd(PSTR("<<"), startScreen);
    }

    const char* addLabels(int i) {
        int d = getAddableIdIndex(i);
        if (d == -1)
            return 0;
        return nameForType(getDescriptor(d));
    }

    void addCallback(int selected, int button) {
        if (selected >= 0 && selected < numberOfIdsThatCanBeAdded() && button >= 0) {
            launchDescriptor(getAddableIdIndex(selected));
            goToScreen(screenAdded);
        } else
            goToScreen(startScreen);
    }

    // REMOVING

    int removableInstance(int instance) {
        for (int i = 0; i < numberOfInstances(); i++)
            if (cardinalityForInstance(i) != APP_CARDINALITY_ALWAYS_ONE)
                if (--instance == -1)
                    return i;
        fatalError(1000, instance);
        return 0;
    }

    int numberOfRemoveables() {
        int n = 0;
        for (int i = 0; i < numberOfInstances(); i++)
            if (cardinalityForInstance(i) != APP_CARDINALITY_ALWAYS_ONE)
                n++;
        return n;
    }

    const char* removeLabels(int instance) {
        for (int i = 0; i < numberOfInstances(); i++)
            if (cardinalityForInstance(i) != APP_CARDINALITY_ALWAYS_ONE)
                if (--instance == -1)
                    return nameForInstance(i);
        return 0;
    }

    void removeCallback(int instance, int button) {
        if (instance >= 0 && button >= 0) {
            if (rulesUsingInstance(instance))
                message(PSTR("There are other instances that have rules that reference "
                             "conditions / actions defined in this "
                             "instance."),
                        startScreen);
            else {
                removeInstanceAt(removableInstance(instance));
                message(PSTR("The app was removed."), startScreen);
            }
        } else
            goToScreen(startScreen);
    }

    // MEM INFO SCREEN

    void screenMem() {
        println(PSTR("Memory & CPU information:\n"));
        printAlignedRight(PSTR("Available EEPROM: "), 180);
        setPrintX(180);
        print(availablePersistantHeap());
        println(PSTR(" bytes"));
        printAlignedRight(PSTR("Available SRAM: "), 180);
        setPrintX(180);
        print(availableHeap());
        println(PSTR(" bytes"));
        printAlignedRight(PSTR("Current load: "), 180);
        setPrintX(180);
        print(currentLoad());
        println(PSTR("%"));
        toolbarAdd(PSTR("<<"), startScreen);
    }

    //
    // MAIN ADMIN SCREEN
    //

    void showAdminAddApp(int param) {
        showSelectDialog(PSTR("Add"), 2, addLabels, addCallback, 0,
                         PSTR("Select the kind of App that you want to instantiate. Then "
                              "press 'Add'."));
    }

    void showAdminRemoveMessage(int param) {
        showSelectDialog(PSTR("Remove app"), 2, removeLabels, removeCallback, 0,
                         PSTR("Select an App to be removed and press 'Remove'."));
    }

    const char* logLabels(int i) {
        if (i < numberOfInstances())
            return instanceName(i);
        return 0;
    }

    void logCallback(int selected, int button) {
        switchContextToInstance(selected);
        if (button == 0) {
            if (!chunkForHandleExists(LOG_HANDLE))
                createLogChunk(200);
            else
                resizeChunk(LOG_HANDLE, chunkSize(LOG_HANDLE) + 200);
        } else if (chunkForHandleExists(LOG_HANDLE)) {
            resizeChunk(LOG_HANDLE, chunkSize(LOG_HANDLE) - 200);
            if (logChunkIsEmpty())
                deallocChunk(LOG_HANDLE);
        }
        popContext();
    }

    void screenLog(int param) {
        showSelectDialog(PSTR(" + "), PSTR(" - "), 2, logLabels, logCallback, 0,
                         PSTR("Select an instance to increase or decrease the log "
                              "size in chunks of 200 bytes."));
    }

    void exceptionPortSelected(int what) {
        if (what)
            _alarmPort = what;
        goToScreen(startScreen);
    }

    void exceptionHandling(int param) {
        showDigitalPortDialog(_alarmPort, exceptionPortSelected);
    }

    void screenMore() {
        printDefinition(PSTR("Remove"));
        println(PSTR("Removes an instance of an app and its configuration."));
        printDefinition(PSTR("E. Handling"));
        println(PSTR("Configures a Port to be set in case of a Fatal Exception. Good for Alarms."));
        printDefinition(PSTR("Log"));
        println(PSTR("Turn Log or on Off for different apps."));
        toolbarAdd(standard, numberOfRemoveables() > 0, PSTR("Remove"), showAdminRemoveMessage);
        toolbarAdd(PSTR("E. Handling"), exceptionHandling);
        toolbarAdd(PSTR(" Log "), screenLog);
        toolbarAdd(PSTR(" <<"), startScreen);
    }

    void startScreen() {
        setBoldStyle();
        drawCenteredString(PSTR("AquaOS Version 0.2"));
        setNormalStyle();
        println();
        println();
        printDefinition(PSTR("Add/Remove"));
        println(PSTR("Add an instance of an app."));
        printDefinition(PSTR("Mem"));
        println(PSTR("Show memory stats and CPU load."));
        printDefinition(PSTR("Log"));
        println(PSTR("Turn Log or on Off for different apps."));

        println();
        endDefinitions();
        print(PSTR("Alarm Port "));
        if (_alarmPort) {
            println(_alarmPort);
        } else {
            println(PSTR("not set"));
        }

        toolbarAdd(PSTR(" Add "), showAdminAddApp);
        toolbarAdd(PSTR("Mem"), screenMem);
        toolbarAdd(PSTR(" ... "), screenMore);
        toolbarAddHome();
    }

    const char* getLabelForPort(int port, int digital) {
        return digital && port == _alarmPort ? PSTR("Alarm") : 0;
    }
}  // namespace admin
