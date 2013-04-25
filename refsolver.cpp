#include "refsolver.h"

#include <cassert>

#include <QDebug>
#include <QStringList>

RefSolver::Ref::Ref()
{
}

RefSolver::Ref::Ref(int tlRow, int tlColumn)
{
    topLeft.row = tlRow;
    topLeft.column = tlColumn;

    if(tlRow < 0)
    {
        type = Type::Range;
        range_type = RangeType::Column;
    }
    else if(tlColumn < 0)
    {
        type = Type::Range;
        range_type = RangeType::Row;
    }
    else
    {
        type = Type::SingleCell;
        range_type = RangeType::NotARange;
    }

    bottomRight = topLeft;
}

RefSolver::Ref::Ref(int tlRow, int tlColumn, int brRow, int brColumn)
{
    topLeft.row = tlRow;
    topLeft.column = tlColumn;
    bottomRight.row = brRow;
    bottomRight.column = brColumn;

    if(tlRow == brRow && tlColumn == brColumn)
    {
        type = Type::SingleCell;
        range_type = RangeType::NotARange;
    }
    else
    {
        type = Type::Range;
        range_type = RangeType::Rect;
    }
}

bool RefSolver::Ref::forEachCell(std::function<void(int, int)> callback, int row_or_col_size) const
{
    if(!valid())return false;

    if(type == Type::SingleCell)
    {
        callback(topLeft.row, topLeft.column);
    }
    else if(type == Type::Range)
    {
        if(range_type == RangeType::Rect)
        {
            for(int r = topLeft.row; r <= bottomRight.row; ++r)
            {
                for(int c = topLeft.column; c <= bottomRight.column; ++c)
                {
                    callback(r,c);
                }
            }
        }
        else if(range_type == RangeType::Row)
        {
            for(int c = 1; c <= row_or_col_size; ++c)
            {
                callback(topLeft.row, c);
            }
        }
        else if(range_type == RangeType::Column)
        {
            for(int r = 1; r <= row_or_col_size; ++r)
            {
                callback(r, topLeft.column);
            }
        }
    }

    return true;
}



bool RefSolver::Ref::valid() const
{
    return type != Type::NotARef;
}

QString RefSolver::Ref::toString() const
{
    if(type == Type::SingleCell)
        return (topLeft.columnFixed ? "$" : "") + RefSolver::numberToColumn(topLeft.column - 1) + (topLeft.rowFixed ? "$" : "") + QString::number(topLeft.row);
    else if(type == Type::Range && range_type == RangeType::Rect)
        return (topLeft.columnFixed ? "$" : "") + RefSolver::numberToColumn(topLeft.column - 1) + (topLeft.rowFixed ? "$" : "") + QString::number(topLeft.row)
                + ":" +
               (bottomRight.columnFixed ? "$" : "") + RefSolver::numberToColumn(bottomRight.column - 1) + (bottomRight.rowFixed ? "$" : "") + QString::number(bottomRight.row);
    else if(type == Type::Range && range_type == RangeType::Row)
    {
        QString part = (topLeft.rowFixed ? "$" : "") + QString::number(topLeft.row);
        return part + ":" + part;
    }
    else if(type == Type::Range && range_type == RangeType::Column)
    {
        QString part = (topLeft.columnFixed ? "$" : "") + RefSolver::numberToColumn(topLeft.column - 1);
        return part + ":" + part;
    }

    return "#REF";
}

QRegExp RefSolver::getRegExp()
{
    /*QString cxp = "(?:(R(\\d*|\\(-?\\d+\\))C(\\d*|\\(-?\\d+\\)))|(?:((\\$?[A-Z]+)(\\$?\\d+))))";
    QRegExp fxp("(?:([A-Z]+):\\1)|"+cxp+"|("+cxp+":"+cxp+")|R(\\d*|\\(-?\\d+\\))|C(\\d*|\\(-?\\d+\\))");*/
    QString cxp = "((\\$?)([A-Z]+)(\\$?)(\\d+))";
    QRegExp fxp("(?:" + cxp + "(?::" + cxp + ")?)|(?:((\\$?)([A-Z]+)):\\11)|(?:((\\$?)(\\d+)):\\14)");
    return fxp;
}

RefSolver::Ref RefSolver::resolve(QString str, int *pos, int *length)
{
    Ref ref;
    ref.type = Ref::Type::NotARef;

    QRegExp exp = getRegExp();

    if(exp.indexIn(str, pos == nullptr ? 0 : *pos) >= 0 && (pos != nullptr || exp.matchedLength() == str.length()))
    {
        if(pos != nullptr)
        {
            *pos = exp.pos();
        }
        if(length != nullptr)
        {
            *length = exp.matchedLength();
        }

        if(!exp.cap(1).isEmpty())
        {
            ref.topLeft.row     = exp.cap(5).toInt();
            ref.topLeft.column  = RefSolver::columnToNumber(exp.cap(3)) + 1;

            ref.topLeft.rowFixed    = !exp.cap(4).isEmpty();
            ref.topLeft.columnFixed = !exp.cap(2).isEmpty();

            if(exp.cap(6).isEmpty())
            {
                ref.type = Ref::Type::SingleCell;
                ref.bottomRight = ref.topLeft;
            }
            else
            {
                ref.type = Ref::Type::Range;
                ref.range_type = Ref::RangeType::Rect;

                ref.bottomRight.row     = exp.cap(10).toInt();
                ref.bottomRight.column  = RefSolver::columnToNumber(exp.cap(8)) + 1;

                ref.bottomRight.rowFixed    = !exp.cap(9).isEmpty();
                ref.bottomRight.columnFixed = !exp.cap(7).isEmpty();
            }
        }
        else if(!exp.cap(11).isEmpty())
        {
            ref.type = Ref::Type::Range;
            ref.range_type = Ref::RangeType::Column;

            ref.topLeft.row     = -1;
            ref.topLeft.column  = RefSolver::columnToNumber(exp.cap(13)) + 1;

            ref.topLeft.rowFixed    = false;
            ref.topLeft.columnFixed = !exp.cap(12).isEmpty();

            ref.bottomRight     = ref.topLeft;
        }
        else if(!exp.cap(14).isEmpty())
        {
            ref.type = Ref::Type::Range;
            ref.range_type = Ref::RangeType::Row;

            ref.topLeft.row     = exp.cap(16).toInt();
            ref.topLeft.column  = -1;

            ref.topLeft.rowFixed    = !exp.cap(15).isEmpty();
            ref.topLeft.columnFixed = false;

            ref.bottomRight     = ref.topLeft;
        }
    }
    else
    {
        if(length != nullptr)
        {
            *length = 0;
        }
    }

    return ref;
}

QString RefSolver::forEachRef(const QString &str, RefSolver::ExpCallback callback)
{
    QString res;
    QRegExp exp = getRegExp();
    int pos = 0, last_pos = 0, length = 0;
    Ref ref;
    while((ref = resolve(str, &pos, &length)).valid())
    {
        res += str.mid(last_pos, pos - last_pos) + callback(ref);
        pos += length;
        last_pos = pos;
    }
    res += str.mid(pos);
    return res;
}


QString RefSolver::numberToColumn(int n)
{
    static QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString res;
    div_t rem;
    rem.quot = n;
    do
    {
        rem = div(rem.quot, alphabet.size());
        res = alphabet.at(rem.rem) + res;
    }while(rem.quot > 0);

    return res;
}

int RefSolver::columnToNumber(QString column)
{
    int base = 'Z' - 'A' + 1;
    int pow  = 1;
    int res  = 0;
    for(int i = 0; i < column.size(); ++i)
    {
        res += pow * (column.at(column.size() - 1 - i).toLatin1() - 'A');
        pow *= base;
    }
    return res;
}

bool RefSolver::test()
{
    RefSolver rs;
    RefSolver::Ref r;

    r = rs.resolve("A1");
    assert(r.type == RefSolver::Ref::Type::SingleCell);
    assert(r.range_type == RefSolver::Ref::RangeType::NotARange);
    assert(r.topLeft.row == 1 && r.topLeft.column == 1);
    assert(r.topLeft.rowFixed == false && r.topLeft.columnFixed == false);

    r = rs.resolve("B$4");
    assert(r.type == RefSolver::Ref::Type::SingleCell);
    assert(r.range_type == RefSolver::Ref::RangeType::NotARange);
    assert(r.topLeft.row == 4 && r.topLeft.column == 2);
    assert(r.topLeft.rowFixed == true && r.topLeft.columnFixed == false);

    r = rs.resolve("$C3:D$8");
    assert(r.type == RefSolver::Ref::Type::Range);
    assert(r.range_type == RefSolver::Ref::RangeType::Rect);
    assert(r.topLeft.row == 3 && r.topLeft.column == 3);
    assert(r.topLeft.rowFixed == false && r.topLeft.columnFixed == true);
    assert(r.bottomRight.row == 8 && r.bottomRight.column == 4);
    assert(r.bottomRight.rowFixed == true && r.bottomRight.columnFixed == false);

    r = rs.resolve("C:C");
    assert(r.type == RefSolver::Ref::Type::Range);
    assert(r.range_type == RefSolver::Ref::RangeType::Column);
    assert(r.topLeft.row == -1 && r.topLeft.column == 3);
    assert(r.topLeft.rowFixed == false && r.topLeft.columnFixed == false);
    assert(r.bottomRight.row == -1 && r.bottomRight.column == 3);
    assert(r.bottomRight.rowFixed == false && r.bottomRight.columnFixed == false);

    r = rs.resolve("$C:$C");
    assert(r.type == RefSolver::Ref::Type::Range);
    assert(r.range_type == RefSolver::Ref::RangeType::Column);
    assert(r.topLeft.row == -1 && r.topLeft.column == 3);
    assert(r.topLeft.rowFixed == false && r.topLeft.columnFixed == true);
    assert(r.bottomRight.row == -1 && r.bottomRight.column == 3);
    assert(r.bottomRight.rowFixed == false && r.bottomRight.columnFixed == true);

    r = rs.resolve("5:5");
    assert(r.type == RefSolver::Ref::Type::Range);
    assert(r.range_type == RefSolver::Ref::RangeType::Row);
    assert(r.topLeft.row == 5 && r.topLeft.column == -1);
    assert(r.topLeft.rowFixed == false && r.topLeft.columnFixed == false);
    assert(r.bottomRight.row == 5 && r.bottomRight.column == -1);
    assert(r.bottomRight.rowFixed == false && r.bottomRight.columnFixed == false);

    r = rs.resolve("$5:$5");
    assert(r.type == RefSolver::Ref::Type::Range);
    assert(r.range_type == RefSolver::Ref::RangeType::Row);
    assert(r.topLeft.row == 5 && r.topLeft.column == -1);
    assert(r.topLeft.rowFixed == true && r.topLeft.columnFixed == false);
    assert(r.bottomRight.row == 5 && r.bottomRight.column == -1);
    assert(r.bottomRight.rowFixed == true && r.bottomRight.columnFixed == false);

    assert(rs.resolve("A1").toString() == "A1");
    assert(rs.resolve("A$1").toString() == "A$1");
    assert(rs.resolve("XY22").toString() == "XY22");
    assert(rs.resolve("3:3").toString() == "3:3");
    assert(rs.resolve("$3:$3").toString() == "$3:$3");
    assert(rs.resolve("A:A").toString() == "A:A");
    assert(rs.resolve("$A:$A").toString() == "$A:$A");
    assert(rs.resolve("$A15:ZC$128").toString() == "$A15:ZC$128");

    return true;
}



