#include "sheet.h"

#include "sheetmodel.h"


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
        return c->second.isEmpty();
    }
}

bool Sheet::isEmpty(const QModelIndex &index) const
{
    return isEmpty(index.row()+1, index.column()+1);
}

bool Sheet::isEmpty(const QModelIndex &tl, const QModelIndex &br, const QModelIndexList &except_maybe) const
{
    for(int row = tl.row(); row <= br.row(); ++row)
    {
        for(int column = tl.column(); column <= br.column(); ++column)
        {
            QModelIndex i = model->index(row, column);
            if(except_maybe.contains(i))continue;
            else
            {
                if(!isEmpty(i))return false;
            }
        }
    }
    return true;
}

SheetCell &Sheet::getCell(const QModelIndex &index)
{
    return getCell(index.row()+1, index.column()+1);
}
