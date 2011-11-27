/*
 Oximetry GUI Headers
 Copyright (c)2011 Mark Watkins <jedimark@users.sourceforge.net>
 License: GPL
*/

#ifndef OXIMETRY_H
#define OXIMETRY_H

#include <QWidget>
#include <QGLContext>
#include <QHBoxLayout>
#include <QSplitter>
#include <qextserialport/qextserialport.h>

#include "SleepLib/profiles.h"
#include "SleepLib/day.h"
#include "SleepLib/session.h"

#include "Graphs/gLineChart.h"
#include "Graphs/gFooBar.h"

class SerialOximeter:public QObject
{
    Q_OBJECT
public:
    explicit SerialOximeter(QObject * parent,QString oxiname, QString portname="",BaudRateType baud=BAUD19200, FlowType flow=FLOW_OFF, ParityType parity=PAR_ODD, DataBitsType databits=DATA_8, StopBitsType stopbits=STOP_1);
    virtual ~SerialOximeter();

    virtual bool Open(QextSerialPort::QueryMode mode=QextSerialPort::EventDriven);
    virtual void Close();

    virtual bool startImport()=0;
    virtual void stopImport() {} // abort, default do nothing.

    virtual bool startLive();
    virtual void stopLive();

    qint64 lastTime() { return lasttime; }
    Machine * getMachine() { return machine; }

    Session *createSession();
    Session * getSession() { return session; }

    void compactToWaveform(EventList *el);
    void compactToEvent(EventList *el);
    void compactAll();

    void setPortName(QString portname);
    void setBaudRate(BaudRateType baud);
    void setFlowControl(FlowType flow);
    void setParity(ParityType parity);
    void setDataBits(DataBitsType databits);
    void setStopBits(StopBitsType stopbits);

    QString portName() { return m_portname; }
    BaudRateType baudRate() { return m_baud; }
    FlowType flowControl() { return m_flow; }
    ParityType parity() { return m_parity; }
    DataBitsType dataBits() { return m_databits; }
    StopBitsType stopBits() { return m_stopbits; }

signals:
    void sessionCreated(Session *);
    void dataChanged();
    void importProcess();
    void importComplete(Session *);
    void importAborted();
    void updateProgress(float f); // between 0 and 1.
    void liveStopped(Session *);

    void updatePulse(float p);
    void updateSpO2(float p);

protected slots:
    virtual void onReadyRead();
    virtual void on_import_process()=0;


protected:
    //virtual void addEvents(EventDataType pr, EventDataType o2, EventDataType pleth=-1000000);

    virtual void addPulse(qint64 time, EventDataType pr);
    virtual void addSpO2(qint64 time, EventDataType o2);
    virtual void addPlethy(qint64 time, EventDataType pleth);


    Session * session;

    EventList * pulse;
    EventList * spo2;
    EventList * plethy;
    QextSerialPort *m_port;

    bool m_opened;
    QString m_oxiname;
    QString m_portname;
    BaudRateType m_baud;
    FlowType m_flow;
    ParityType m_parity;
    DataBitsType m_databits;
    StopBitsType m_stopbits;
    QextSerialPort::QueryMode m_mode;
    Machine *machine;

    qint64 lasttime;
    bool import_mode;
};

class CMS50Serial:public SerialOximeter
{
public:
    explicit CMS50Serial(QObject * parent,QString portname);
    virtual ~CMS50Serial();
    virtual bool startImport();

protected:
    virtual void on_import_process();

    virtual void onReadyRead();
    bool waitf6;
    short cntf6;
    short failcnt;

    QByteArray data;
    QVector<QDateTime> f2time;
    int datasize;

    int received_bytes;
};

namespace Ui {
    class Oximetry;
}

enum PORTMODE { PM_LIVE, PM_RECORDING };
const int max_data_points=1000000;

class Oximetry : public QWidget
{
    Q_OBJECT

public:
    explicit Oximetry(QWidget *parent, gGraphView * shared=NULL);
    ~Oximetry();

    void RedrawGraphs();

    gGraphView *graphView() { return GraphView; }

private slots:
    void on_RefreshPortsButton_clicked();
    void on_RunButton_toggled(bool checked);

    void on_SerialPortsCombo_activated(const QString &arg1);
    //void onReadyRead();
    //void onDsrChanged(bool status);

    void on_ImportButton_clicked();

    void onDataChanged();
    void onPulseChanged(float p);
    void onSpO2Changed(float o2);

    void on_saveButton_clicked();
    void on_updateProgress(float f);
    void on_import_aborted();
    void on_import_complete(Session *session);

private:
    void import_finished();
    Ui::Oximetry *ui;

    gGraphView *GraphView;
    MyScrollBar *scrollbar;
    QHBoxLayout *layout;

    gLineChart *pulse,*spo2,*plethy;
    gGraph *PULSE,*SPO2,*PLETHY,*CONTROL;

    QVector<gLineChart *> Data;

    QextSerialPort *port;
    QString portname;
    PORTMODE portmode;
    double lasttime,starttime;
    int lastpulse, lastspo2;

    Day * day;
    //Session * session;
    //EventList * ev_pulse;
    //EventList * ev_spo2;
    //EventList * ev_plethy;
    Layer * foobar;
    gGraphView * m_shared;

    SerialOximeter *oximeter;
    bool firstSPO2Update;
    bool firstPulseUpdate;
    bool secondPulseUpdate;
    bool secondSPO2Update;

};

#endif // OXIMETRY_H
