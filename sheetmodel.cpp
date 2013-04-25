#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QScriptEngine>

#include "app.h"

#include "sheetmodel.h"
#include "refsolver.h"

SheetModel::SheetModel(QObject *parent) :
    QAbstractTableModel(parent),
    cells(this, 56000,256)
{
}

int SheetModel::rowCount(const QModelIndex &parent) const
{
    return cells.rowCount();
}

int SheetModel::columnCount(const QModelIndex &parent) const
{
    return cells.columnCount();
}

Qt::ItemFlags SheetModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant SheetModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return cells.getCell(index.row() + 1, index.column() + 1).getValue();
    }
    else if(role == Qt::EditRole)
    {
        SheetCell cell = cells.getCell(index.row() + 1, index.column() + 1);
        if(cell.hasFormula())return cell.getFormula();
        else return cell.getValue();
    }
    else if(role == Qt::UserRole)
    {
        return cells.getCell(index.row() + 1, index.column() + 1).getDataType();
    }

    return QVariant();
}

bool SheetModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() && role == Qt::EditRole)
    {
        SheetCell &cell = cells.getCell(index.row() + 1,index.column() + 1);
        cell.setValue(value);

        if(cell.hasFormula())computeCell(cell);

        emit dataChanged(index, index);

        for(const SheetCell::Ref &c : cell.children)
        {
            SheetCell &target = c.model->getCell(c.row, c.column);
            computeCell(target, true, true);
        }

        return true;
    }
    return false;
}

QVariant SheetModel::computeCell(SheetCell &cell, bool recompute, bool cascading)
{
    qDebug()<<"Computing cell"<<cell.toString()<<"with val"<<cell.getValue();
    if(!cell.hasFormula() or !recompute)return cell.getValue();

    QString raw_code = cell.getFormula().mid(1);
    QString code;

    if(!cascading)cell.removeFromParentsChildren();

    code = RefSolver::forEachRef(raw_code, [this, &cell, cascading](const RefSolver::Ref &r) -> QString {
        QString v = "#NIY";

        if(r.type == RefSolver::Ref::Type::SingleCell)
        {
            if(!cascading)cell.addParent(r, this);
            SheetCell &cell = cells.getCell(r.topLeft.row, r.topLeft.column);
            v = SheetCell::toScriptValue(computeCell(cell, false).toString());
        }
        else if(r.type == RefSolver::Ref::Type::Range)
        {
            QStringList array;

            auto callback = [&array, this, cascading, &cell](int row, int column){
                if(!cascading)cell.addParent({row,column}, this);
                array.push_back(computeCell(cells.getCell(row, column), false).toString());
            };

            if(r.range_type == RefSolver::Ref::RangeType::Rect)
            {
                r.forEachCell(callback);
            }
            else if(r.range_type == RefSolver::Ref::RangeType::Row)
            {
                r.forEachCell(callback, cells.rowCount());
            }
            else if(r.range_type == RefSolver::Ref::RangeType::Column)
            {
                r.forEachCell(callback, cells.columnCount());
            }

            v = SheetCell::toScriptValue(array);
        }

        return v;
    });

    qDebug()<<"Source code:"<<raw_code<<" Precompiled code:"<<code;

    QScriptValue val = App::getJs().evaluate(code);
    QVariant result;

    if(val.isNumber())
    {
        result = val.toNumber();
    }
    else if(val.isString())
    {
        result = val.toString();
    }
    else
    {
        result = "#OOPS";
    }

    cell.setValue(result);
    QModelIndex idx = index(cell.getRow(), cell.getColumn());
    emit dataChanged(idx, idx);


    for(const SheetCell::Ref &c : cell.children)
    {
        SheetCell &target = c.model->getCell(c.row, c.column);
        computeCell(target, true, true);
    }


    return result;
}

void SheetModel::readCSV(const std::string &path)
{
    CSVReader reader;
    reader.table = &cells;
    reader.read(path);
}

SheetCell &SheetModel::getCell(int row, int column)
{
    return cells.getCell(row, column);
}

QVariant SheetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return RefSolver::numberToColumn(section);
    }
    else return QAbstractTableModel::headerData(section, orientation, role);
}


CSV::Row &SheetModel::CSVReader::onRow(CSV::Row &row)
{
    ++n_rows;
    for(int i = 0; i < row.size(); ++i)
    {
        table->getCell(n_rows, i + 1).setValue(row[i].c_str());
    }
    return row;
}
