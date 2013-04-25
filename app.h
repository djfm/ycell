#ifndef APP_H
#define APP_H

#include <QtScript/QScriptEngine>
#include <QString>

#include "sheetmodel.h"

class App
{
    QScriptEngine js;

public:

    static App *getInstance();
    static QScriptEngine &getJs();

    SheetModel *open(const QString &path);

private:
    App();
    static App *instance;
};

#endif // APP_H
