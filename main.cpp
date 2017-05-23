#include "mainwindow.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    EDigitalOut(&w.ID,0, 0, 1, 1);


    w.show();

    if (!w.initialized){
        qDebug("initializing");
        long switchState=0;
        bool switchInit=false;
        bool mvdUp=false;

        while(switchInit==false){
            EDigitalIn(&w.ID, 0 ,1 ,0, &switchState);



            qDebug("dropping motor");

            if (switchState>0){
                for (int i=0; i<100; i++){
                    qDebug("move up");

                }
                mvdUp=true;
            }

            if (mvdUp==true && switchState>0){ switchInit = true;}


        }


        w.initialized=true;
    }

    return a.exec();
}
