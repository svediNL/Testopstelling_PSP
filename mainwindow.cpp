#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

//#include "qtcsv/stringdata.h"
//#include "qtcsv/qtcsv_global.h"
//#include "qtcsv/reader.h"
//#include "qtcsv/writer.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    ui->logDirLineEdit->setText(logDir);

    QWidget::setWindowTitle("Polymer Science Park - 3DPrintHuge Testopstelling");

    trisD=3;
    DigitalIO(&ID, 0, &trisD, trisIO, &stateD, &stateIO,1, &outputD);


    // connect timers to functions for periodic excecution
    connect(this, SIGNAL(atimeout()), this, SLOT(readButton()));
    connect(this,SIGNAL(platformTimeout()), this, SLOT(incrementPlatform()));
    connect(this,SIGNAL(verticalTimeout()), this, SLOT(incrementVertical()));

    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(movePlatform()));
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(moveVertical()));


    // start timers
    timer.start(1, this);
    globalTimer.start(10, this);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{

    // ******************************************** //
    // *********** EMIT TIMEOUT SIGNALS *********** //
    // ******************************************** //

    // random timer
    if (event->timerId()==timer.timerId()){
        emit atimeout();
        //qDebug(dateTime.currentDateTime().toString().toLatin1());
        //qDebug("timerevent");
    }


    //global timer to synchronize functions
    if (event->timerId()==globalTimer.timerId()){
        emit globalTimeout();
        //qDebug(dateTime.currentDateTime().toString().toLatin1());
        //qDebug("globaltimerevent");
    }

    // timer to move platform motor one step
    if(event->timerId()==platformTimer.timerId()){

        emit platformTimeout();

    }

    // timer to move vertical motor one step
    if(event->timerId()==verticalTimer.timerId()){

        emit verticalTimeout();
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
void MainWindow::movePlatform()
{
    // This function calculates the pulsefrequency for a given RPM,
    // then starts a timer accordingly. The timer triggers another slot/function
    // which outputs a pulse on the labjack
    platformTimer.stop();

    int microsteps,stepsPerRotation;
    float mPulsePeriod;
    QString steppingString;

    platRPM=ui->platformSpinBox->value();   //get RPM value set by user

    // calculate steps per rotation
    steppingString= ui->microSteppingComboBox->currentText();   //get microsteping info
    microsteps= steppingString.toLatin1().toInt();  //convert string to integer
    stepsPerRotation= microsteps*200;  //calculate amount of steps for one rotation

    platformPPS = rpmtopps(platRPM, stepsPerRotation);  //convert rpm to pulses per second

    // calculate and set timer period
    PulseOutCalc(&platformPPS, &timeB, &timeC); // calculate appropriate pulse parameters for labjack
    platformPeriod= 1/platformPPS; // timer period related to appropriate frequency [in seconds]
    mPulsePeriod= 1000*platformPeriod;  // convert seconds to milliseconds
    platformTimer.start(mPulsePeriod, Qt::TimerType(0),this);   //start timer

    qDebug("PlatformPPS: %f", platformPPS);
    qDebug("Platform mpulsePeriod %f", mPulsePeriod);

}

void MainWindow::incrementPlatform()
{
    qDebug("Platform increment");

    //PulseOut(&ID, 0, 0, 1, 1, timeB,timeC,timeB,timeC);

    EDigitalOut(&ID,0, 0, 1, 0);
    Sleep(1000*(1/platformPPS));
    EDigitalOut(&ID,0, 0, 1, 1);

    if(platDir){ asPlatPos++;} else if (platDir==false || asPlatPos>0){asPlatPos--;}

    qDebug()<<asPlatPos;

}

void MainWindow::directionPlatform(bool dir)
{
    // set direction of platform motor
    if(dir){ EDigitalOut(&ID,0,0,1,1); platDir=true;}
    else{EDigitalOut(&ID,0,0,1,0); platDir=false;}

}

float MainWindow::rpmtopps(float rpm, int spr)
{
    // this function converts rpm into pulses per second
    // rpm - rotation per minute
    // spr - steps per rotation
    // pps - pulses per second
    // rps - rotations per second

    float pps, rps;
    rps = rpm/60;
    pps = rps*spr;

    return pps;
}


// FUNCTIONS DIRECTLY RELATED TO VERTICAL MOTOR
void MainWindow::moveVertical()
{
    // This function calculates the pulsefrequency for the vertical motor,
    // then starts a timer accordingly. The timer triggers another slot/function
    // which outputs a pulse on the labjack
    verticalTimer.stop();

    float layerHeight, mPulsePeriod;
    QString steppingString;
    int microSteps, stepsPerRotation;

    // calculate steps per rotation
    layerHeight= ui->layerDoubleSpinBox->value();   //get layerheight
    steppingString= ui->microSteppingComboBox_2->currentText(); //get microstepping value
    microSteps= steppingString.toLatin1().toInt();  //convert string to integer
    stepsPerRotation= microSteps*200;   // calculate steps per rotation

    verticalPPS= mmstopps(layerHeight,stepsPerRotation); // calcuate pulses per second related to vertical speed

    //calculate and set timer period
    PulseOutCalc(&verticalPPS,&timeB1,&timeC1); // calculate appropriate pulse parameters for the labjack
    verticalPeriod = 1/verticalPPS; // calculate timer period related to appropriate frequency [in seconds]
    mPulsePeriod= 1000*verticalPeriod; //convert period into milliseconds
    verticalTimer.start(mPulsePeriod, Qt::TimerType(0), this);  //start timer

    qDebug("verticalPPS: %f", verticalPPS);
    qDebug("vertical mPulsePeriod %f", mPulsePeriod);

}

void MainWindow::incrementVertical()
{
    qDebug("Vertical increment");

    PulseOut(&ID, 0, 0, 2, 1, timeB1,timeC1,timeB1,timeC1);

    if(vertDir){ asVertPos++;} else if (vertDir==false || asVertPos>0){asVertPos--;}

}

void MainWindow::directionVertical(bool dir)
{
    // set direction if vertical motor
    if(dir){ EDigitalOut(&ID,0,0,1,1); vertDir= true; }
    else{EDigitalOut(&ID,0,0,1,0); vertDir=false; }

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

// ******************************** //
// *v* standard generated slots *v* //
// *v*v*v*v*v*v*v*vv*v*v*v*v*v*v*v* //

// stop motor timers on button click
void MainWindow::on_stopSpeedButton_clicked()
{
    EDigitalOut(&ID,0, 0, 1, 1);
    platformTimer.stop();
    verticalTimer.stop();
}

// stop motors on tab change
void MainWindow::on_controlTab_currentChanged(int index)
{
    // this slot checks if the control tab has been changed

    //if the speed tab is inactive all speed related timers will be stopped
    if (index != 0 ){

        platformTimer.stop();
        verticalTimer.stop();

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

void MainWindow::on_incButton_clicked()
{

//    EDigitalOut(&ID,0, 0, 1, 0);
//    Sleep(20);
//    EDigitalOut(&ID,0, 0, 1, 1);

    long int a,b;
    float freq  = ui->hertzSpinBox->value();


    PulseOutCalc(&freq, &a, &b); // calculate appropriate pulse parameters for labjack
    PulseOut(&ID, 0, 0, 1, 1, a,b,a,b);


}
