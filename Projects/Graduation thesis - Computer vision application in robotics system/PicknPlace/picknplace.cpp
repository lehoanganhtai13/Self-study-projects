#include "picknplace.h"

PicknPlace::PicknPlace(QObject *parent) : QObject(parent)
{
    main = new main_dialog;
}

void PicknPlace::implement(){
    qDebug() << "PicknPlace plugin is loaded succesfully";
    main->show();
}
