#include "sheet.h"

Sheet::Sheet(SheetModel *m, int r, int c)
    :SparseTable<SheetCell>(r,c)
{
    setModel(m);
}

void Sheet::setModel(SheetModel *m)
{
    model = m;
}

SheetModel *Sheet::getModel()
{
    return model;
}

void Sheet::init(SheetCell &c)
{
    c.setSheet(this);
}
