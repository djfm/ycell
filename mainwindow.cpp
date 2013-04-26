#include "mainwindow.h"

#include <QFileDialog>


#include "app.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    Ui::MainWindow()
{
    setupUi(this);

    current_sheet_model = new SheetModel(this);
    //current_sheet_model->readCSV("/home/fram/Downloads/_1.5.3.1.csv");
    sheet->setModel(current_sheet_model);
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_openButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Please choose which file to open...");
    if(!path.isEmpty())
    {
        SheetModel *model = App::getInstance()->open(path);
        if(model != nullptr)
        {
            if(current_sheet_model != nullptr)delete current_sheet_model;
            current_sheet_model = model;
            sheet->setModel(current_sheet_model);
        }
    }
}
