#ifndef SHEETMODEL_H
#define SHEETMODEL_H

#include <QAbstractTableModel>
#include <QAbstractItemView>
#include <QFlags>


#include "sheet.h"
#include "sheetcell.h"
#include "csv.h"

class SheetModel : public QAbstractTableModel
{

    Q_OBJECT
public:

    enum class Direction
    {
        Up,
        Down,
        Left,
        Right,
        None
    };

    enum EraseMode
    {
        EraseValue   = 0x1,
        EraseFormula = 0x2,
        EraseFormat  = 0x4
    };
    Q_DECLARE_FLAGS(EraseModes, EraseMode)

    explicit SheetModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant computeCell(SheetCell &cell, bool recompute = true, bool cascading = false);

    void readCSV(const std::string &path);

    SheetCell &getCell(int row, int column);

    QModelIndex jumpIndex(const QModelIndex &current, Direction dir);

    void erase(const QModelIndex &index, const EraseModes &mode);

public slots:
    void on_this_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

protected:

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:
    
public slots:
    
private:

    Sheet cells;

    struct CSVReader : public CSV
    {
        Sheet *table;
        int n_rows = 0;
        Row &onRow(Row &row);
    };

};

Q_DECLARE_OPERATORS_FOR_FLAGS(SheetModel::EraseModes)

#endif // SHEETMODEL_H
