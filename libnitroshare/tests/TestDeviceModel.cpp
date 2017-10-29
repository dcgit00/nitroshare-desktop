/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nathan Osman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <QSignalSpy>
#include <QTest>

#include <nitroshare/device.h>
#include <nitroshare/deviceenumerator.h>
#include <nitroshare/devicemodel.h>

const QString TestUuid = "1234";
const QString TestName = "Test";
const QString TestAddress1 = "127.0.0.1";
const QString TestAddress2 = "::1";

class TestDeviceModel : public QObject
{
    Q_OBJECT

private slots:

    void testAddDevice();
    void testUpdateDevice();
    void testRemoveDevice();
    void testRemoveEnumerator();

private:

    void addDevice();

    DeviceEnumerator enumerator;
};

void TestDeviceModel::testAddDevice()
{
    DeviceModel model;
    model.addDeviceEnumerator(&enumerator);

    // Ensure that a single row is added
    QSignalSpy rowsInsertedSpy(&model, &DeviceModel::rowsInserted);
    addDevice();
    QCOMPARE(rowsInsertedSpy.count(), 1);

    // Ensure the device contains the expected values
    Device *device = model.find(TestUuid);
    QVERIFY(device);
    QCOMPARE(device->uuid(), TestUuid);
    QCOMPARE(device->name(), TestName);
    QCOMPARE(device->addresses().count(), 2);
    QVERIFY(device->addresses().contains(TestAddress1));
    QVERIFY(device->addresses().contains(TestAddress2));
}

void TestDeviceModel::testUpdateDevice()
{
    DeviceModel model;
    model.addDeviceEnumerator(&enumerator);
    addDevice();

    // Update the device with identical data and no signal should be emitted
    QSignalSpy dataChangedSpy(&model, &DeviceModel::dataChanged);
    addDevice();
    QCOMPARE(dataChangedSpy.count(), 0);

    // Make a change - remove an addresses - and ensure the signal is emitted
    emit enumerator.deviceUpdated(TestUuid, {
        { "addresses", QStringList{ TestAddress1 } }
    });
    QCOMPARE(dataChangedSpy.count(), 1);
}

void TestDeviceModel::testRemoveDevice()
{
    DeviceModel model;
    model.addDeviceEnumerator(&enumerator);
    addDevice();

    // Watch for removal of the device
    QSignalSpy spy(&model, &DeviceModel::rowsRemoved);
    emit enumerator.deviceRemoved(TestUuid);

    // Ensure one row was removed
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).toInt(), 0);
    QCOMPARE(spy.at(0).at(2).toInt(), 0);

    // Ensure there is nothing left in the model
    QCOMPARE(model.rowCount(QModelIndex()), 0);
}

void TestDeviceModel::testRemoveEnumerator()
{
    DeviceModel model;
    model.addDeviceEnumerator(&enumerator);
    addDevice();

    // Watch for removal of the device once the enumerator is removed
    QSignalSpy spy(&model, &DeviceModel::rowsRemoved);
    model.removeDeviceEnumerator(&enumerator);

    // Ensure a row was removed
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.rowCount(QModelIndex()), 0);
}

void TestDeviceModel::addDevice()
{
    // Emit a signal for a new device that has been discovered
    emit enumerator.deviceUpdated(TestUuid, {
        { "name", TestName },
        { "addresses", QStringList({ TestAddress1, TestAddress2 }) }
    });
}

QTEST_MAIN(TestDeviceModel)
#include "TestDeviceModel.moc"
