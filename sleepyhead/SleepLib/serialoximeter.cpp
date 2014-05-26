/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 *
 * SleepLib Machine Loader Class Implementation
 *
 * Copyright (c) 2011-2014 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#include <QtSerialPort/QSerialPortInfo>

#include "serialoximeter.h"

// Possibly need to replan this to include oximetry

QList<SerialOximeter *> GetOxiLoaders()
{
    QList<SerialOximeter *> oxiloaders;

    QList<MachineLoader *> loaders = GetLoaders(MT_OXIMETER);

    Q_FOREACH(MachineLoader * loader, loaders) {
        SerialOximeter * oxi = qobject_cast<SerialOximeter *>(loader);
        oxiloaders.push_back(oxi);
    }

    return oxiloaders;
}
bool SerialOximeter::scanDevice(QString keyword,quint16 vendor_id, quint16 product_id)
{
    QStringList ports;

    //qDebug() << "Scanning for USB Serial devices";
    QList<QSerialPortInfo> list=QSerialPortInfo::availablePorts();

    // How does the mac detect this as a SPO2 device?
    for (int i=0;i<list.size();i++) {
        const QSerialPortInfo * info=&list.at(i);
        QString name=info->portName();
        QString desc=info->description();

        if ((!keyword.isEmpty() && desc.contains(keyword)) ||
            ((info->hasVendorIdentifier() && (info->vendorIdentifier()==vendor_id))
                && (info->hasProductIdentifier() && (info->productIdentifier()==product_id))))
        {
            ports.push_back(name);
            QString dbg=QString("Found Serial Port: %1 %2 %3 %4").arg(name).arg(desc).arg(info->manufacturer()).arg(info->systemLocation());

            if (info->hasProductIdentifier()) //60000
                dbg+=QString(" PID: %1").arg(info->productIdentifier());
            if (info->hasVendorIdentifier()) // 4292
                dbg+=QString(" VID: %1").arg(info->vendorIdentifier());

            qDebug() << dbg.toLocal8Bit().data();
            break;
        }
    }
    if (ports.isEmpty()) {
        return false;
    }
    if (ports.size()>1) {
        qDebug() << "More than one serial device matching these parameters was found, choosing the first by default";
    }
    port=ports.at(0);
    return true;
}

void SerialOximeter::closeDevice()
{
    killTimers();
    disconnect(&serial,SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    serial.close();
    m_streaming = false;
    qDebug() << "Port" << port << "closed";
}

bool SerialOximeter::openDevice()
{
    if (port.isEmpty()) {
        if (!scanDevice("",m_vendorID, m_productID))
            return false;
    }
    serial.setPortName(port);
    if (!serial.open(QSerialPort::ReadWrite))
        return false;

    // forward this stuff

    // Set up serial port attributes
    serial.setBaudRate(QSerialPort::Baud19200);
    serial.setParity(QSerialPort::OddParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setDataBits(QSerialPort::Data8);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    m_streaming = true;
    m_abort = false;
    m_importing = false;

    // connect relevant signals
    connect(&serial,SIGNAL(readyRead()), this, SLOT(dataAvailable()));

    return true;
}

void SerialOximeter::dataAvailable()
{
    QByteArray bytes;

    int available = serial.bytesAvailable();
    bytes.resize(available);

    int bytesread = serial.read(bytes.data(), available);
    if (bytesread == 0)
        return;

    if (m_abort) {
        closeDevice();
        return;
    }

    processBytes(bytes);
}

void SerialOximeter::stopRecording()
{
    closeDevice();
    m_status = NEUTRAL;
    emit importComplete(this);
}