#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSplashScreen* splash= new QSplashScreen;
    splash->setPixmap(QPixmap("‪C:/Qt/projects/untitled/willie3.png"));
    splash->show();


    MainWindow w;

    QTimer::singleShot(2500,splash,SLOT(close()));
    QTimer::singleShot(2500,&w,SLOT(show()));

    EDigitalOut(&w.ID,0, 0, 1, 1);

    w.show();

//    if (!w.initialized){
//        qDebug("initializing");
//        long switchState=0;
//        bool switchInit=false;
//        bool b= true;
//        bool mvdUp=false;

//        //open serial connection
//        w.serial.setPortName("COM3");
//        w.serial.open(QIODevice::ReadWrite);


//        while(switchInit==false){

//            EDigitalIn(&w.ID, 0 ,1 ,0, &switchState);
//            while (b){
//            w.talktoarduino("platDir", "0");
//            QThread::msleep(100);
//            w.talktoarduino("runspd","0");
//            b=false;
//            }

//            qDebug("dropping motor");

//            if (switchState>0){
//                w.talktoarduino("stopAll", "0");
//                QThread::msleep(100);
//                w.talktoarduino("platDir", "1");

//                for (int i=0; i<10; i++){
//                    QThread::msleep(100);
//                    w.talktoarduino("incPlat", "0");
//                    qDebug("move up");
//                }
//                mvdUp=true;
//            }

//            if (mvdUp==true && switchState>0){ switchInit = true;}


//      }

//        w.initialized=true;
//    }

    return a.exec();



}
