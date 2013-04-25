#ifndef APP_H
#define APP_H

#include <QtScript/QScriptEngine>

class App
{
    QScriptEngine js;

public:

    static App *getInstance();
    static QScriptEngine &getJs();

private:
    App();
    static App *instance;
};

#endif // APP_H
