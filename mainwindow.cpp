#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    int initPlatPPS,initVertPPS, initLayer;

    ui->setupUi(this);
    ui->logDirLineEdit->setText(logDir);

    serial.setPortName("COM3");
    serial.open(QIODevice::ReadWrite);
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    initPlatPPS= rpmtopps(10, 1600);
    initVertPPS= mmstopps(1,1600);
    initLayer =1;

    talktoarduino("setRamp", "500");
    talktoarduino("platPPS", QString::number(initPlatPPS));
    //talktoarduino("vertPPS", QString::number(initVertPPS));
    talktoarduino("setLayer", QString::number(initLayer));


    QWidget::setWindowTitle("Polymer Science Park - 3DPrintHuge Testopstelling");

    trisD=3;
    DigitalIO(&ID, 0, &trisD, trisIO, &stateD, &stateIO,1, &outputD);

    printMsg.setText("Prepare the extruder");
    printMsg.setInformativeText("Move the extruder to the desired radius click 'ready' when extruder is in position \n\r \n\r");
    printMsg.setIcon(QMessageBox::Information);

    warningMsgVert.setText("WARNING");
    warningMsgVert.setInformativeText("Make sure the platform is moving in the right direction,\r\n +5V= down \r\n +0V=up");
    warningMsgVert.setIcon(QMessageBox::Warning);



    // =-=-=-=-=-=-=-=-= CONNECT SLOTS  &   SIGNALS =-=-=-=-=-=-=-=-= //
    connect(ui->initializeButton, SIGNAL(clicked(bool)), this, SLOT(homing()));

    connect(ui->motorRPMBox, SIGNAL(valueChanged(double)), ui->rpmPrintBox, SLOT(setValue(double)));
    connect(ui->rpmPrintBox, SIGNAL(valueChanged(double)), ui->motorRPMBox, SLOT(setValue(double)));

    connect(ui->motorMmsBox, SIGNAL(valueChanged(double)), ui->layerPrintBox, SLOT(setValue(double)));
    connect(ui->layerPrintBox, SIGNAL(valueChanged(double)), ui->motorMmsBox, SLOT(setValue(double)));

    //connect(this, SIGNAL(sensor_timeout()), this, SLOT(readPressure()));


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
        readSetSpeed();
        readPressure();
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

void MainWindow::readSetSpeed()
{
    float voltage;
    long int overV, setRPM;
    EAnalogIn(&ID,0,0,0,&overV,&voltage);

    if(voltage<=0.01){voltage=0;}
    if(voltage>9.95){voltage=10;}

    setRPM= 150*voltage;

    ui->labelSetRPM->setText(QString::number(setRPM));

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

    platRPM=ui->motorRPMBox->value();   //get RPM value set by user

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
    layerHeight= ui->motorMmsBox->value();   //get layerheight
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

//void MainWindow::readButton()
//{
//    long state;

//    EDigitalIn(&ID, 0 ,1 ,0, &state);

//    if(state>0){
//        ui->buttonLabel->setText("button pressed");
//        endSwitch=true;
//    }else {
//        ui->buttonLabel->setText("button not pressed");
//        endSwitch=false;
//    }

//}

void MainWindow::readPressure()
{
    float voltage;
    long int overV, pressure;
    EAnalogIn(&ID,0,9,0,&overV,&voltage);
    pressure= voltage*99;


    ui->labelReadPressure->setText(QString::number(pressure));


}

// ******************************** //
// *v* standard generated slots *v* //
// *v*v*v*v*v*v*v*vv*v*v*v*v*v*v*v* //

// change log file directory
void MainWindow::on_logDirButton_clicked()
{
    logDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                               "C://", QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    ui->logDirLineEdit->setText(logDir);
}

void MainWindow::on_rampDoubleSpinBox_valueChanged(double arg1)
{
    int rampTime= arg1*1000;

    talktoarduino("setRamp", QString::number(rampTime));

}

//void MainWindow::on_dial_sliderReleased()
//{
//    double cpos= 0;
//    double platPos = ui->dial->value();
//    double microsteps = ui->microSteppingComboBox->currentText().toLatin1().toInt();
//    double PPR = microsteps;
//    double gotopos =(platPos/100)*PPR;

//    if (cpos<gotopos){
//        talktoarduino("platDir", "0");

//    }
//    else{


//    }
//    talktoarduino("movePlat", QString::number(gotopos));
//    cpos=platPos;

//}

void MainWindow::on_sendCommandButton_clicked()
{
    talktoarduino(ui->commandComboBox->currentText(), ui->valueLineEdit->text());

    ui->valueLineEdit->clear();
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
        on_motorMmsBox_valueChanged(ui->motorMmsBox->value());
    } else { talktoarduino("disVert", "0");qDebug("disVert");}

}


//_-_-_-_-_-_-_- SLOTS FROM MOTOR TAB _-_-_-_-_-_-_-//

void MainWindow::on_motorMmsBox_valueChanged(double arg1)
{
    int val;
    int ppr;

    ppr= ui->microSteppingComboBox_2->currentText().toLatin1().toInt();   // calculate steps per rotation
    val = (ppr*arg1) / ( ui->pitchDoubleSpinBox->value() );

    talktoarduino("vertPPS",QString::number(val));

}

void MainWindow::on_motorRPMBox_valueChanged(double arg1)
{
    int val;
    int ppr;

    ppr= ui->microSteppingComboBox->currentText().toLatin1().toInt();   // calculate steps per rotation
    val = rpmtopps(arg1, ppr);

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

//_-_-_-_-_-_-_- SLOTS FROM PRINT TAB _-_-_-_-_-_-_-//

void MainWindow::on_rpmPrintBox_valueChanged(double arg1)
{
    int val;
    int ppr;

    ppr= ui->microSteppingComboBox->currentText().toLatin1().toInt();   // calculate steps per rotation
    val = rpmtopps(arg1, ppr);

    talktoarduino("platPPS",QString::number(val));
}

void MainWindow::on_layerPrintBox_valueChanged(double arg1)
{
    int val;
    int ppr;

    ppr= ui->microSteppingComboBox_2->currentText().toLatin1().toInt();   // calculate steps per rotation
    val = (ppr*(ui->motorMmsBox->value()))/(ui->pitchDoubleSpinBox->value());

    talktoarduino("vertPPS",QString::number(val));
    talktoarduino("setLayer", QString::number(arg1) );

}

void MainWindow::on_startPrintButton_clicked()
{
    platBool=true;
    talktoarduino("disVert","0");
    talktoarduino("disPlat","0");

    warningMsgVert.exec();

    talktoarduino("enPlat", "0");

    if (ui->printContinuousButton->isChecked()){talktoarduino("runspd", "0");}
    else if(ui->printStepsButton->isChecked()) {talktoarduino("runprint","0");}

    if(warningMsgVert.clickedButton()==msgOkButton){ printMsg.exec(); }
    else{talktoarduino("slowAll","0");}

    if(printMsg.clickedButton()== msgReadyButton ){talktoarduino("enVert", "0");}
    else if(printMsg.clickedButton() == msgCancelButton){talktoarduino("slowAll", "0");};

}

void MainWindow::on_readyPrintButton_clicked()
{
    if (platBool){
        talktoarduino("enVert", "0");
        platBool= false;
    }
}

void MainWindow::on_stopPrintButton_clicked()
{
    talktoarduino("slowAll", "0");
}

void MainWindow::on_initializeButton_clicked()
{
    QMessageBox::information(this, tr("WARNING"),tr("Make sure the platform is moving in the right direction,\r\n +5V= down \r\n ground=up"));
}

void MainWindow::on_moveUpRadio_clicked()
{
    talktoarduino("vertDir","0");
}

void MainWindow::on_moveDownRadio_clicked()
{
    talktoarduino("vertDir","1");
}

void MainWindow::on_cwRadio_clicked()
{
    talktoarduino("platDir","1");
}

void MainWindow::on_ccwRadio_clicked()
{
    talktoarduino("platDir","0");
}
