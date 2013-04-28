#include "sheetcell.h"

#include <QDebug>

#include "sheetmodel.h"
#include "refsolver.h"

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
    //qDebug()<<"Setting value for "<<toString()<<":"<<val;

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

bool SheetCell::isEmpty() const
{
    return !hasFormula() && value.toString().isEmpty();
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

QString SheetCell::toScriptValue(const QList<QStringList> &list)
{
    QString val;

    bool first = true;
    for(const QStringList &l : list)
    {
        if(first)first=false;
        else val += ", ";

        val.push_back(toScriptValue(l));
    }

    return "[" + val + "]";
}

QString SheetCell::toString() const
{
    return RefSolver::Ref(row,column).toString();
}

void SheetCell::clearFormula()
{
    formula.clear();
}

bool SheetCell::hasRange() const
{
    return has_range;
}

void SheetCell::setHasRange(int rows, int columns)
{
    has_range = true;
    range_rows = rows;
    range_columns = columns;
}

void SheetCell::setHasNoRange()
{
    has_range = false;
    range_rows = 0;
    range_columns = 0;
}

void SheetCell::setIsInRange()
{
    is_in_range = true;
}

int SheetCell::getRangeRows() const
{
    return range_rows;
}

int SheetCell::getRangeColumns() const
{
    return range_columns;
}

QString SheetCell::translatedFormula(int d_rows, int d_columns) const
{
    return RefSolver::forEachRef(formula, [d_rows, d_columns](const RefSolver::Ref &r) -> QString{
        return r.translate(d_rows, d_columns).toString();
    });
}

SheetModel *SheetCell::model()
{
    return sheet->getModel();
}


uint qHash(const SheetCell::Ref &r)
{
    return (long)r.model * ((((uint)r.row) << 16) | (((uint)r.column) & 0xFFFF));
}
