#include "app.h"

#include <QFile>
#include <QDebug>

App *App::instance = nullptr;

App *App::getInstance()
{
    if(instance == nullptr)instance = new App();
    return instance;
}

QScriptEngine &App::getJs()
{
    return getInstance()->js;
}

App::App()
{
    //TODO: make finding the JS lib location (more)generic!
    QFile file("../ycell/common.js");
    if(file.open(QFile::ReadOnly))
    {
        QString code = file.readAll();
        js.evaluate(code);
        file.close();
    }
    else
    {
        qDebug()<<"Could not find common scripts file!";
    }
}
