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
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(on_this_dataChanged(QModelIndex,QModelIndex,QVector<int>)));
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
        if(cells.isEmpty(index))
        {
            return "";
        }
        return cells.getCell(index.row() + 1, index.column() + 1).getValue();
    }
    else if(role == Qt::EditRole)
    {
        SheetCell cell = cells.getCell(index.row() + 1, index.column() + 1);
        if(cell.hasFormula())return cell.getFormula();
        else return cell.getValue();
    }
    else if(role == Qt::BackgroundRole)
    {
        if(cells.isSet(index))
        {
            if(cells.getCell(index.row() + 1, index.column() + 1).is_in_range)
            {
                return QColor(255,255,204);
            }
        }
    }
    else if(role == Qt::UserRole)
    {
        if(cells.isEmpty(index))
        {
            return SheetCell::DataType::String;
        }
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
        else
        {
            cell.removeFromParentsChildren();
            cell.parents.clear();
        }

        emit dataChanged(index, index);

        return true;
    }
    return false;
}

QVariant SheetModel::computeCell(SheetCell &cell, bool recompute, bool cascading)
{
    qDebug()<<"Computing cell"<<cell.toString()<<(recompute ? "" : "(Using Cache)");//<<"with val"<<cell.getValue();
    if(!cell.hasFormula() or !recompute)return cell.getValue();

    QString raw_code = cell.getFormula().mid(1);
    QString code;

    if(!cascading)cell.removeFromParentsChildren();

    bool error = false;
    QString error_string = "";

    code = RefSolver::forEachRef(raw_code, [this, &cell, &error, &error_string, cascading](const RefSolver::Ref &r) -> QString {
        QString v = "#NIY";

        if(r.type == RefSolver::Ref::Type::SingleCell)
        {
            if(!cascading)
            {
                if(!cell.addParent(r, this))
                {
                    error = true;
                    error_string = "#CRCREF";
                }
            }
            if(!error)
            {
                SheetCell &cell = cells.getCell(r.topLeft.row, r.topLeft.column);
                v = SheetCell::toScriptValue(computeCell(cell, false).toString());
            }
        }
        else if(r.type == RefSolver::Ref::Type::Range)
        {
            QStringList array;
            QList<QStringList> rows;

            auto callback = [&array, this, cascading, &cell, &error, &error_string](int row, int column){
                if(!cascading)
                {
                    if(!cell.addParent({row,column}, this))
                    {
                        error = true;
                        error_string = "#CRCREF";
                    }
                }
                array.push_back(computeCell(cells.getCell(row, column), false).toString());
            };

            if(r.range_type == RefSolver::Ref::RangeType::Rect)
            {
                for(int row = r.topLeft.row; row <= r.bottomRight.row; ++row)
                {
                    QStringList list;
                    for(int column = r.topLeft.column; column <= r.bottomRight.column; ++column)
                    {
                        RefSolver::Ref ref(row, column);
                        if(!cascading)
                        {
                            if(!cell.addParent(ref, this))
                            {
                                error = true;
                                error_string = "#CRCREF";
                                row = r.bottomRight.row + 1; //break outer loop
                                break;
                            }
                        }
                        //qDebug()<<"Registering dep from"<<ref.toString()<<"to"<<RefSolver::Ref(cell.row, cell.column).toString();
                        list.push_back(computeCell(cells.getCell(row, column), false).toString());
                    }
                    rows.push_back(list);
                }

                if(!error)
                {
                    v = SheetCell::toScriptValue(rows);
                }

            }
            else if(r.range_type == RefSolver::Ref::RangeType::Row)
            {
                r.forEachCell(callback, cells.rowCount());
                if(!error)
                {
                    v = SheetCell::toScriptValue(array);
                }
            }
            else if(r.range_type == RefSolver::Ref::RangeType::Column)
            {
                r.forEachCell(callback, cells.columnCount());
                if(!error)
                {
                    v = SheetCell::toScriptValue(array);
                }
            }


        }

        return v;
    });

    //qDebug()<<"Source code:"<<raw_code<<" Precompiled code:"<<code;

    QScriptValue val = App::getJs().evaluate(code);
    QVariant result;

    if(!error)
    {
        if(val.isNumber())
        {
            result = val.toNumber();
            cell.setHasNoRange();
        }
        else if(val.isString())
        {
            result = val.toString();
            cell.setHasNoRange();
        }
        else if(val.isArray())
        {
            SparseTable<QString> table;
            int length = val.property("length").toInteger();
            for(int i = 0; i < length; ++i)
            {
                if(val.property(i).isArray())
                {
                    int l = val.property(i).property("length").toInteger();
                    for(int j = 0; j < l; ++j)
                    {
                        table.getCell(i,j) = val.property(i).property(j).toString();
                    }
                }
                else
                {
                    QString v = val.property(i).toString();
                    table.getCell(i,0) = v;
                }
            }

            if(cell.hasRange())
            {
                eraseRange(cell);
            }

            QModelIndex tl = index(cell.getRow()-1, cell.getColumn()-1);
            QModelIndex br = index(cell.getRow()-1 + table.rowCount()-1, cell.getColumn()-1 + table.columnCount()-1);

            QSet<SheetCell::Ref> ancestors = cell.getAncestors();
            QList<SheetCell*> modified;

            if(cells.isEmpty(tl,br,{tl}))
            {
                for(int i = 0; i < table.rowCount(); ++i)
                {
                    for(int j = 0; j < table.columnCount(); ++j)
                    {
                        int r = cell.getRow() + i;
                        int c = cell.getColumn() + j;

                        SheetCell::Ref target = {this, r, c};

                        if(ancestors.contains(target))
                        {
                            //qDebug()<<target.toRef().toString()<<" is in the ancestors of "<<cell.toString();

                            error = true;
                            error_string = "#CRCREF";
                            i = table.rowCount() + 1; //break outer loop
                            break;
                        }
                        else
                        {
                            SheetCell &myCell = cells.getCell(r,c);
                            myCell.setValue(table.getCell(i,j));
                            myCell.setIsInRange();
                            modified.push_back(&myCell);
                        }
                    }
                }
                if(!error)
                {
                    cell.setHasRange(table.rowCount(), table.columnCount());
                    cell.setIsInRange();
                    result = table.getCell(0,0);
                    emit dataChanged(tl, br);
                }
                else
                {
                    for(SheetCell *c : modified)
                    {
                        c->setValue("");
                        c->setHasNoRange();
                    }
                    cell.setHasNoRange();
                    result = error_string;
                }
            }
            else
            {
                cell.setHasNoRange();
                result = "#NDMRSPC(" + QString::number(table.rowCount()) + "x" + QString::number(table.columnCount()) + ")";
            }
        }
        else
        {
            cell.setHasNoRange();
            result = "#OOPS";
        }
    }
    else
    {
        result = error_string;
    }

    cell.setValue(result);
    QModelIndex idx = index(cell.getRow()-1, cell.getColumn()-1);
    emit dataChanged(idx, idx);

    return result;
}

void SheetModel::on_this_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    for(int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        for(int column = topLeft.column(); column <= bottomRight.column(); ++column)
        {
            SheetCell &cell = cells.getCell(row+1, column+1);
            for(const SheetCell::Ref &c : cell.children)
            {
                SheetCell &target = c.model->getCell(c.row, c.column);
                computeCell(target, true, true);
            }
        }
    }
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

QModelIndex SheetModel::jumpIndex(const QModelIndex &current, Direction dir)
{
    int lateral, vertical;
    if(dir == Direction::Right)
    {
        lateral  = 1;
        vertical = 0;
    }
    else if(dir == Direction::Left)
    {
        lateral  = -1;
        vertical =  0;
    }
    else if(dir == Direction::Up)
    {
        lateral  =  0;
        vertical = -1;
    }
    else if(dir == Direction::Down)
    {
        lateral  =  0;
        vertical =  1;
    }
    else
    {
        return current;
    }

    //qDebug()<<"Jumping "<<lateral<<vertical;

    int row_count = rowCount({});
    int column_count = columnCount({});

    bool start_empty = cells.isEmpty(current.row()+1, current.column()+1);

    int row = current.row() + 1 + vertical;
    int column = current.column() + 1 + lateral;

    if(!start_empty && row <= row_count && row >= 1 && column <= column_count && column >= 1 && cells.isEmpty(row, column))
    {
        start_empty = true;
        row    += vertical;
        column += lateral;
    }

    for(; row <= row_count && row >= 1 && column <= column_count && column >= 1; row += vertical, column += lateral)
    {
        bool empty = cells.isEmpty(row, column);
        if(start_empty && !empty)
        {
            break;
        }
        else if(!start_empty && empty)
        {
            row    -= vertical;
            column -= lateral;
            break;
        }
    }

    if(row == 0)++row;
    else if(row == row_count + 1)--row;
    if(column == 0)++column;
    else if(column == column_count + 1)--column;

    qDebug()<<"Got"<<row<<column;

    return index(row - 1, column - 1);

}

void SheetModel::erase(const QModelIndex &index, const EraseModes &mode)
{
    if(index.isValid())
    {
        if(cells.isSet(index))
        {
            SheetCell &cell = cells.getCell(index);
            if(mode & EraseValue)
            {
                cell.setValue({});
            }
            if(mode & EraseFormula)
            {
                cell.clearFormula();
                eraseRange(cell);
                cell.removeFromParentsChildren();
                cell.parents.clear();
            }
            if(mode & EraseInRange)
            {
                cell.is_in_range = false;
            }
            emit dataChanged(index, index);
        }
    }
}

void SheetModel::eraseRange(SheetCell &cell)
{
    if(cell.hasRange())
    {
        for(int i = 0; i < cell.getRangeRows(); ++i)
        {
            for(int j = 0; j < cell.getRangeColumns(); ++j)
            {
                erase(index(cell.getRow()-1+i, cell.getColumn()-1+j), EraseValue | EraseInRange);
            }
        }
    }
}

void SheetModel::paste(const QModelIndex &index, const QItemSelection &selection)
{
    SheetCell &from = cells.getCell(index);

    for(const QItemSelectionRange &range : selection)
    {
        for(int row = range.topLeft().row(); row <= range.bottomRight().row(); ++row)
        {
            for(int column = range.topLeft().column(); column <= range.bottomRight().column(); ++column)
            {
                if(row != index.row() || column != index.column())
                {
                    SheetCell &to = cells.getCell(row+1, column+1);

                    if(from.hasFormula())
                    {
                        qDebug()<<"Pasted formula";
                        to.setValue(from.translatedFormula(row - index.row(), column - index.column()));
                        computeCell(to);
                    }
                    else
                    {
                        to.setValue(from.getValue());
                    }
                }
            }
        }
        emit dataChanged(range.topLeft(), range.bottomRight());
    }

}

Sheet &SheetModel::getSheet()
{
    return cells;
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
