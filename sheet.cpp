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

bool Sheet::isEmpty(int row, int column) const
{
    auto r = rows.find(row);
    if(r == rows.end())
    {
        return true;
    }
    auto c = r->second.find(column);
    if(c == r->second.end())
    {
        return true;
    }
    else
    {
        return c->second.getValue().toString().isEmpty();
    }
}
