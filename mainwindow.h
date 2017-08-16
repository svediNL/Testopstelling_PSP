#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <math.h>
#include <windows.h>
#include <fstream>
#include <iostream>

#include <QMainWindow>
#include <QString>
#include <QDebug>


#include <QLabel>

#include <QTimer>
#include <QDateTime>
#include <QTimerEvent>

#include <QTextStream>
#include <QFile>
#include <QList>
#include <QStringList>
#include <QDir>
#include <QThread>
#include <QMessageBox>

#include <QFileDialog>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "qextserialport-master/src/qextserialport.h"



extern "C"{
#include "ljackuw.h"
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool initialized=false;
    bool endSwitch=false;
    bool platBool= false;

    //public parameters for controlling IO on CB25 extrension module
    long int ID=0;
    long int trisD=0;
    long int trisIO=0;
    long int stateD=0;
    long int stateIO=0;
    long int outputD=0;

    //public parameters for platform movement

    float platformPPS;
    float platformPeriod;
    float platRPM;
    int asPlatPos=0;    //assumed platform position
    bool platDir= true;
    bool platEnable = true;

    //public parameters for vertical movement
    float verticalPPS;
    float verticalPeriod;
    int asVertPos=0;
    bool vertDir= true;
    bool vertEnable= true;

    QBasicTimer sensorTimer;

    QString logDir = "C:/Qt/logs/";

    QSerialPort serial;

    QMessageBox printMsg;
    QMessageBox warningMsgVert;

    QPushButton *msgReadyButton= printMsg.addButton(tr("Ready"), QMessageBox::AcceptRole);
    QPushButton *msgCancelButton= printMsg.addButton(QMessageBox::Cancel);
    QPushButton *msgOkButton= warningMsgVert.addButton(tr("Ok"), QMessageBox::AcceptRole);

    // public functions
    float rpmtopps(float rpm, int spr);
    float mmstopps(float mmr, int spr);
    void talktoarduino(QString command, QString value);
    int getDataArduino(QString command);
    void delay(int ms);


private slots:
    void homing();

    void closeCom();
    void runSpeed();

    void setPlatformPeriod();
    void setVerticalPeriod();

    void writeDataTxt(QString fileName, QString writeData);

    void readSetSpeed();

    //void readButton();
    void readPressure();

    //standard slots
    void on_logDirButton_clicked();

    void on_rampDoubleSpinBox_valueChanged(double arg1);

    //void on_dial_sliderReleased();
    void on_sendCommandButton_clicked();


    void on_enablePlatBox_toggled(bool checked);

    void on_enableVertBox_toggled(bool checked);

    void on_motorRunButton_clicked();

    void on_motorStopButton_clicked();

    void on_startPrintButton_clicked();

    void on_readyPrintButton_clicked();

    void on_stopPrintButton_clicked();

    void on_initializeButton_clicked();

    void on_rpmPrintBox_valueChanged(double arg1);

    void on_layerPrintBox_valueChanged(double arg1);

    void on_motorRPMBox_valueChanged(double arg1);

    void on_motorMmsBox_valueChanged(double arg1);

    void on_moveUpRadio_clicked();

    void on_moveDownRadio_clicked();

    void on_cwRadio_clicked();

    void on_ccwRadio_clicked();

signals:
    sensor_timeout();

private:
    Ui::MainWindow *ui;


    QDateTime dateTime;

    QLabel buttonLabel;

protected:
    void timerEvent(QTimerEvent *event);


};

#endif // MAINWINDOW_H
