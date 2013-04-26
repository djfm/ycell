#include <iostream>
#include <ctime>

#include <QApplication>
#include <QDebug>

#include "refsolver.h"

#include "mainwindow.h"
#include "csv.h"
#include "sparsetable.h"
#include "app.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Q_INIT_RESOURCE(resources);

    App::getInstance();

    RefSolver::test();

    //return 0;

    /*
    QScriptEngine engine;
    QScriptValue v = engine.evaluate("1+1");

    if(v.isNumber())
    {
        qDebug() << v.toNumber();
    }

    return 0;*/

    /*
    CSV::Huffman h;
    h.insert("b");
    h.insert("ba");
    std::cout<<h.validate('b')<<std::endl;
    std::cout<<h.validate('a')<<std::endl;
    h.remove("ba");
    h.reset();
    std::cout<<h.validate('b')<<std::endl;
    std::cout<<h.validate('a')<<std::endl;
    return 0;*/

    /*
    CSV csv;
    //csv.parse("hel\"lo;;worlld!,lol;\"quo\\\"te\"\"d\"\n");
    int t0 = clock();
    csv.read("/home/fram/Downloads/_1.5.3.1.csv");
    std::cout<<"Read file in "<<clock()-t0<<" microsecs."<<std::endl;*/



    MainWindow w;
    w.show();
    
    return a.exec();
}
