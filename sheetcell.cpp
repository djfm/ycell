#include "sheetcell.h"

#include <QDebug>

#include "sheetmodel.h"

SheetCell::Ref::Ref(SheetModel *m, int r, int c)
    :model(m), row(r), column(c)
{
}

bool SheetCell::Ref::operator ==(const SheetCell::Ref &other) const
{
    return other.model == model && other.row == row && other.column == column;
}

SheetCell &SheetCell::setValue(const QVariant &val)
{
    qDebug()<<"Setting value for "<<toString()<<":"<<val;

    if(!val.toString().isEmpty() && val.toString().at(0) == '=')
    {
        formula     = val.toString();
        value       = "(uncomputed cell)";
        data_type   = String;
        return *this;
    }

    bool isFloat;
    float f = val.toFloat(&isFloat);

    if(isFloat)
    {
        data_type = Number;
        value = f;
    }
    else
    {
        data_type = String;
        value = val;
    }

    return *this;
}

const QVariant &SheetCell::getValue() const
{
    return value;
}

SheetCell::DataType SheetCell::getDataType() const
{
    return data_type;
}

bool SheetCell::hasFormula() const
{
    return !formula.isEmpty();
}

const QString &SheetCell::getFormula() const
{
    return formula;
}

int SheetCell::getRow() const
{
    return row;
}

int SheetCell::getColumn() const
{
    return column;
}

void SheetCell::setRow(int r)
{
    row = r;
}

void SheetCell::setColumn(int c)
{
    column = c;
}

void SheetCell::setSheet(Sheet *tbl)
{
    sheet = tbl;
}

void SheetCell::removeFromParentsChildren()
{
    for(const Ref &r : parents)
    {
        r.model->getCell(r.row, r.column).children.remove(Ref(model(), row, column));
    }
    parents.clear();
}

void SheetCell::addParent(const RefSolver::Ref &r, SheetModel *sheet)
{
    Ref ref(sheet, r.topLeft.row, r.topLeft.column);
    parents.insert(ref);
    sheet->getCell(ref.row, ref.column).children.insert(Ref(model(), row, column));
}

QString SheetCell::toScriptValue(const QString &val)
{
    if(val.isEmpty())return "0";
    else
    {
        bool number;
        val.toFloat(&number);
        if(!number)
        {
            QString v = val;
            v = "\"" + v.replace("\"","\\\"") + "\"";
            return v;
        }
        else return val;
    }
}

QString SheetCell::toScriptValue(const QStringList &list)
{
    QString val;

    bool first = true;
    for(const QString &str : list)
    {
        if(first)first=false;
        else val += ", ";

        val.push_back(toScriptValue(str));
    }

    return "[" + val + "]";
}

QString SheetCell::toString() const
{
    return RefSolver::Ref(row,column).toString();
}

SheetModel *SheetCell::model()
{
    return sheet->getModel();
}


uint qHash(const SheetCell::Ref &r)
{
    return (long)r.model * ((((uint)r.row) << 16) | (((uint)r.column) & 0xFFFF));
}
