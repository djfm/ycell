#ifndef SHEET_H
#define SHEET_H

#include "sparsetable.h"

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

private:
    SheetModel *model;
};

#endif // SHEET_H
