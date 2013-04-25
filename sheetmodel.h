#ifndef SHEETMODEL_H
#define SHEETMODEL_H

#include <QAbstractTableModel>
#include <QtScript/QScriptEngine>

#include "sheet.h"
#include "sheetcell.h"
#include "csv.h"

class SheetModel : public QAbstractTableModel
{

    Q_OBJECT
public:

    explicit SheetModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant computeCell(SheetCell &cell, bool recompute = true, bool cascading = false);

    void readCSV(const std::string &path);

    SheetCell &getCell(int row, int column);

protected:

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:
    
public slots:
    
private:

    QScriptEngine js;

    Sheet cells;

    struct CSVReader : public CSV
    {
        Sheet *table;
        int n_rows = 0;
        Row &onRow(Row &row);
    };

};

#endif // SHEETMODEL_H
