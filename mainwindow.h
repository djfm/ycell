#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

#include "sheetmodel.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    SheetModel *current_sheet_model = nullptr;

private slots:
    void on_openButton_clicked();

};

#endif // MAINWINDOW_H
