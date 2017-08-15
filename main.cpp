#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>

int initPlatPPS,initVertPPS, initLayer;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    //QImage splashImage("willie3.png");
    //QPixmap pixmap;
    //pixmap.convertFromImage(splashImage);
    //QSplashScreen* splash= new QSplashScreen(pixmap);
    //splash->show();
    //QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    //splash.show();
    //a.processEvents();

    //for(int i=0;i<500000;i++){
    //    qDebug()<<i;
    //}

    EDigitalOut(&w.ID,0, 0, 1, 1);

    w.show();

    //initPlatPPS= w.rpmtopps(10, 1600);
    //1initVertPPS= w.mmstopps(1,1600);
    //initLayer =1;
    //w.talktoarduino("setRamp", "500");
   // w.talktoarduino("platPPS", QString::number(initPlatPPS));
    //w.talktoarduino("vertPPS", QString::number(initVertPPS));
    //w.talktoarduino("setLayer", QString::number(initLayer));

    //splash.finish(&w);

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
