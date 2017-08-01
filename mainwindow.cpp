#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    ui->logDirLineEdit->setText(logDir);

    serial.setPortName("COM3");
    serial.open(QIODevice::ReadWrite);
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    QWidget::setWindowTitle("Polymer Science Park - 3DPrintHuge Testopstelling");

    trisD=3;
    DigitalIO(&ID, 0, &trisD, trisIO, &stateD, &stateIO,1, &outputD);

    // update stepper motor periods
    connect(ui->platformSpinBox, SIGNAL(editingFinished()), this, SLOT(setPlatformPeriod()));
    connect(ui->platformSpinBox, SIGNAL(editingFinished()), this, SLOT(setVerticalPeriod()));
    connect(ui->layerDoubleSpinBox, SIGNAL(editingFinished()), this, SLOT(setPlatformPeriod()));
    connect(ui->layerDoubleSpinBox, SIGNAL(editingFinished()), this, SLOT(setVerticalPeriod()));
    //connect(ui->microSteppingComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPlatformPeriod()));
    //connect(ui->microSteppingComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setVerticalPeriod()));
    //connect(ui->microSteppingComboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(setPlatformPeriod()));
    //connect(ui->microSteppingComboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(setVerticalPeriod()));
    connect(ui->initializeButton, SIGNAL(clicked(bool)), this, SLOT(homing()));

    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(runSpeed()));
//    connect(this, SIGNAL(MainWindow::closeEvent()), this, SLOT(closeCom()));

    connect(this, SIGNAL(sensor_timeout()), this, SLOT(readPressure()));


    // start timers

    sensorTimer.start(100, this);
    //globalTimer.start(10, this);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{

//    // ******************************************** //
//    // *********** EMIT TIMEOUT SIGNALS *********** //
//    // ******************************************** //

    //qDebug("EVENT");

    // Sensor read timer
    if (event->timerId()==sensorTimer.timerId()){
        emit sensor_timeout();
        //qDebug(dateTime.currentDateTime().toString().toLatin1());
        //qDebug("sensorTimer");
    }

}

void MainWindow::writeDataTxt(QString fileName, QString writeData)
{

    // this function prints one line into a .txt file,
    // each line is seperated by a comma and an endline.
    // QString fileName - desired name of the log file, date (YYYYMMDD) is appended automatically
    // QString writeData - the line that is printed in the file

    QDateTime cDateTime= QDateTime::currentDateTime();
    QDate cDate = cDateTime.date();
    QString cDay = QString::number(cDate.day());
    QString cMonth = QString::number(cDate.month());
    QString cYear = QString::number(cDate.year());

    // add a 0 if a date is smaller than 10 to stick to YYYYMMDD format
    if (cDate.day()<10) { cDay.prepend('0'); }
    if (cDate.month()<10) { cMonth.prepend('0'); }

    //generate entire file path and file name
    QString filePath= logDir;
    filePath.append(fileName);
    filePath.append(cYear);
    filePath.append(cMonth);
    filePath.append(cDay);
    filePath.append(".txt");

    // open/create file based on desired path
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){

        //connect stream to file
        QTextStream stream(&file);

        // get time data and tranform QTime into QString
        QTime cTime = QDateTime::currentDateTime().time();
        QString cHour = QString::number(cTime.hour());
        QString cMinute = QString::number(cTime.minute());
        QString cSecond = QString::number(cTime.second());

        //check if time data<10 to stickto basic format HH:MM:SS
        if(cTime.hour()<10){cHour.prepend("0");}
        if(cTime.minute()<10){cMinute.prepend("0");}
        if(cTime.second()<10){cSecond.prepend("0");}

        //build time string
        QString prepTime = cHour;
        prepTime.append(":");
        prepTime.append(cMinute);
        prepTime.append(":");
        prepTime.append(cSecond);


        //place data in stream
        stream << prepTime << ";" << writeData << ";" << endl;

        //flush stream and close file
        stream.flush();
        file.close();
    }

}


// FUNCTIONS DIRECTLY RELATED TO PLATFORM MOTOR
void MainWindow::setPlatformPeriod()
{

    // This function calculates the pulsefrequency for a given RPM,
    // then starts a timer accordingly. The timer triggers another slot/function
    // which outputs a pulse on the labjack

    int microsteps,stepsPerRotation;
    float mPulsePeriod;
    QString steppingString;

    platRPM=ui->platformSpinBox->value();   //get RPM value set by user

    // calculate steps per rotation
    steppingString= ui->microSteppingComboBox->currentText();   //get microsteping info
    microsteps= steppingString.toLatin1().toInt();  //convert string to integer
    stepsPerRotation= ui->microSteppingComboBox->currentText().toLatin1().toInt();  //calculate amount of steps for one rotation

    platformPPS = rpmtopps(platRPM, stepsPerRotation);  //convert rpm to pulses per second

    // calculate and set timer period
    platformPeriod= 1/platformPPS; // timer period related to appropriate frequency [in seconds]
    mPulsePeriod= 1000000*platformPeriod;  // convert seconds to microseconds

    //set pulsePeriod for the platform on arduino
    talktoarduino("platPPS", QString::number(platformPPS));

    qDebug("PlatformPPS: %f", platformPPS);
    qDebug("Platform mpulsePeriod %f", mPulsePeriod);

}

float MainWindow::rpmtopps(float rpm, int spr)
{
    // this function converts rpm into pulses per second
    // rpm - rotation per minute
    // spr - steps per rotation
    // pps - pulses per second
    // rps - rotations per second

    float pps, rps, i;
    i = ui->platReductionBox->value();
    rps = rpm/60;
    pps = rps*spr*i;

    return pps;
}


// FUNCTIONS DIRECTLY RELATED TO VERTICAL MOTOR
void MainWindow::setVerticalPeriod()
{

    // This function calculates the pulsefrequency for the vertical motor,
    // then starts a timer accordingly. The timer triggers another slot/function
    // which outputs a pulse on the labjack
    float layerHeight, mPulsePeriod;
    QString steppingString;
    int microSteps, stepsPerRotation;

    // calculate steps per rotation
    layerHeight= ui->layerDoubleSpinBox->value();   //get layerheight
    steppingString= ui->microSteppingComboBox_2->currentText(); //get microstepping value
    microSteps= steppingString.toLatin1().toInt();  //convert string to integer
    stepsPerRotation= ui->microSteppingComboBox_2->currentText().toLatin1().toInt();   // calculate steps per rotation

    verticalPPS= mmstopps(layerHeight,stepsPerRotation); // calcuate pulses per second related to vertical speed

    //calculate and set timer period
    verticalPeriod = 1/verticalPPS; // calculate timer period related to appropriate frequency [in seconds]
    mPulsePeriod= 1000000*verticalPeriod; //convert period into milliseconds

    //set pulsePeriod for the vertical axis on arduino
    talktoarduino("vertPPS", QString::number(verticalPPS));

    qDebug("verticalPPS: %f", verticalPPS);
    qDebug("vertical mPulsePeriod %f", mPulsePeriod);

}

float MainWindow::mmstopps(float mmr, int spr)
{
    //This function calculates pulses per second needed for a layerheight
    // related to the platform speed
    //mmr - millimeters per rotation of the platform (layerheight)
    //spr - steps per revolution of the motor
    //rps - rotations per second
    //spd - vertical speed
    //pps - pulses per second

    float rps, spd, pitch, pps;

    pitch= ui->pitchDoubleSpinBox->value(); // get pitch value
    spd= (platRPM/60)*mmr;   //vertical speed,from layerheight related to rps
    rps= spd/pitch; //rotations per second
    pps= rps*spr;   //pulses per second

    return pps;

}


// ARDUINO FUNCTIONS
void MainWindow::talktoarduino(QString command, QString value)
{
    QString datatosend;
    datatosend.append(command);
    datatosend.append(";");
    datatosend.append(value);
    datatosend.append("~");
    qint64 datal = datatosend.length();

    qDebug()<< "datatosend" << datatosend.toUtf8();

    serial.setBaudRate(2000000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);


    serial.write(datatosend.toLatin1(), datal);
    serial.flush();

    Sleep(100);
    datatosend.clear();


}

int MainWindow::getDataArduino(QString command)
{
    QString datatosend;
    int a;
    datatosend.append(command);
    datatosend.append(";");
    datatosend.append("0");
    datatosend.append("~");
    qint64 datal = datatosend.length();

    qDebug()<< "datatosend" << datatosend.toUtf8();

    serial.setBaudRate(2000000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.write(datatosend.toLatin1(), datal);
    serial.flush();

    a= serial.readAll().toInt();

    return a;


}

void MainWindow::delay(int ms)
{
    QTime ct, st;
    ct= QTime::currentTime();
    st= QTime::currentTime().addMSecs(ms);

    while(ct<st){

    }

    return;
}

void MainWindow::homing()
{
    talktoarduino("init","0");
}


void MainWindow::closeCom()
{
   serial.close();

}

void MainWindow::runSpeed()
{

    talktoarduino("runspd",QString::number(0));

}


// TEST FUNCTIONS

void MainWindow::readButton()
{
    long state;

    EDigitalIn(&ID, 0 ,1 ,0, &state);

    if(state>0){
        ui->buttonLabel->setText("button pressed");
        endSwitch=true;
    }else {
        ui->buttonLabel->setText("button not pressed");
        endSwitch=false;
    }

}

void MainWindow::readPressure()
{
    float voltage;
    long int overV;
    EAnalogIn(&ID,0,0,0,&overV,&voltage);

    ui->pressureLabel->setText(QString::number(voltage));


}

// ******************************** //
// *v* standard generated slots *v* //
// *v*v*v*v*v*v*v*vv*v*v*v*v*v*v*v* //

// stop motor timers on button click
void MainWindow::on_stopSpeedButton_clicked()
{
    talktoarduino("slowAll", QString::number(0));

}

// stop motors on tab change
void MainWindow::on_controlTab_currentChanged(int index)
{
    // this slot checks if the control tab has been changed

    //if the speed tab is inactive all speed related timers will be stopped
    if (index != 0 ){
        talktoarduino("stopAll","0");

    }

}

// read potmeterand print data in log file
void MainWindow::on_getButton_clicked()
{
    // **************************** //
    // *** get potmeter voltage *** //
    // **************************** //

    float voltage;
    long int overVolts;
    int commaPos;

    QString fileName, writeData;
    fileName= "potmeter";

    EAnalogIn(&ID,0,0,0,&overVolts, &voltage);  //get data from AI0

    writeData= QString::number(voltage, 'f', 6); //convert voltage interger to a string

    //replace '.' with ',' to avoid inerpretation error from excel
    commaPos = writeData.indexOf(".");
    writeData.replace(commaPos,1,",");

    qDebug()<<"WriteDate: "<< writeData;

    writeDataTxt(fileName, writeData);

    qDebug("input val: %f",voltage);
}

//turn LED on/off
void MainWindow::on_ledBox_toggled(bool checked)
{
    qDebug("checked %i", checked);

    EDigitalOut(&ID,0,0,0,checked);

}

// change log file directory
void MainWindow::on_logDirButton_clicked()
{
    logDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                               "C://", QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    ui->logDirLineEdit->setText(logDir);
}

void MainWindow::on_testPPS_button_clicked()
{
    talktoarduino("platPeriod", QString::number(200));
}

void MainWindow::on_testrunspd_button_clicked()
{
    talktoarduino("runspd", QString::number(0));
}

void MainWindow::on_testStopAll_button_clicked()
{
    talktoarduino("stopAll", QString::number(0));
}

void MainWindow::on_rightButton_clicked()
{
    talktoarduino("incPlat", QString::number(0));
}

void MainWindow::on_rampDoubleSpinBox_valueChanged(double arg1)
{
    int rampTime= arg1*1000;

    talktoarduino("setRamp", QString::number(rampTime));

}

void MainWindow::on_dial_sliderReleased()
{
    double cpos= 0;
    double platPos = ui->dial->value();
    double microsteps = ui->microSteppingComboBox->currentText().toLatin1().toInt();
    double PPR = microsteps;
    double gotopos =(platPos/100)*PPR;

    if (cpos<gotopos){
        talktoarduino("platDir", "0");

    }
    else{


    }
    talktoarduino("movePlat", QString::number(gotopos));
    cpos=platPos;

}

void MainWindow::on_sendCommandButton_clicked()
{
    talktoarduino(ui->commandComboBox->currentText(), ui->valueLineEdit->text());

    ui->valueLineEdit->clear();
}

void MainWindow::on_pushButton_5_clicked()
{
    int a;
    QString A;

    a = getDataArduino("getPlatPos");
    qDebug()<<a;
}

void MainWindow::on_enablePlatBox_toggled(bool checked)
{
    if (checked){
        talktoarduino("enPlat", "0");qDebug("enPlat");
    } else { talktoarduino("disPlat", "0");qDebug("disPlat");}
}

void MainWindow::on_enableVertBox_toggled(bool checked)
{
    if (checked){
        talktoarduino("enVert", "0");qDebug("enVert");
        on_motorMmsBox_editingFinished();
    } else { talktoarduino("disVert", "0");qDebug("disVert");}

}

void MainWindow::on_motorMmsBox_editingFinished()
{
    int val;
    int ppr;

    ppr= ui->microSteppingComboBox_2->currentText().toLatin1().toInt();   // calculate steps per rotation

    val = (ppr*(ui->motorMmsBox->value()))/(ui->pitchDoubleSpinBox->value());



    talktoarduino("vertPPS",QString::number(val));
}

void MainWindow::on_motorRPMBox_editingFinished()
{
    int val;
    int ppr;

    ppr= ui->microSteppingComboBox->currentText().toLatin1().toInt();   // calculate steps per rotation


    val = rpmtopps(ui->motorRPMBox->value(), ppr);



    talktoarduino("platPPS",QString::number(val));

}

void MainWindow::on_motorRunButton_clicked()
{
    if ( !(ui->enablePlatBox->isChecked()) && !(ui->enableVertBox->isChecked()) ){

    }else{

        talktoarduino("runspd", "0");
    }
}

void MainWindow::on_motorStopButton_clicked()
{
    talktoarduino("slowAll", "0");
}
