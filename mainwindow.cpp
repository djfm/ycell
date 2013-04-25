#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sheetmodel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SheetModel *model = new SheetModel(this);
    //model->readCSV("/home/fram/Downloads/_1.5.3.1.csv");
    ui->sheet->setModel(model);

}

MainWindow::~MainWindow()
{
    delete ui;
}
