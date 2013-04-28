#ifndef SHEET_H
#define SHEET_H

#include "sparsetable.h"

#include <QModelIndex>

#include "sheetcell.h"

class SheetModel;

class Sheet : public SparseTable<SheetCell>
{

public:
    Sheet(SheetModel *m, int r, int c);

    void setModel(SheetModel *m);
    SheetModel *getModel();

    void init(SheetCell &c);

    bool isEmpty(int row, int column) const;
    bool isEmpty(const QModelIndex &index) const;
    bool isSet(const QModelIndex &index) const;
    bool isEmpty(const QModelIndex &tl, const QModelIndex &br, const QModelIndexList & except_maybe = {}) const;

    using SparseTable<SheetCell>::getCell;
    SheetCell &getCell(const QModelIndex &index);

private:
    SheetModel *model;
};

#endif // SHEET_H
