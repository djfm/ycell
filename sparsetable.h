#ifndef SPARSETABLE_H
#define SPARSETABLE_H

#include <map>

template <class Cell>
class SparseTable
{
    typedef std::map<int, Cell> Row;
    typedef std::map<int, Row> Rows;


protected:
    Rows rows;

    int row_count;
    int column_count;

public:
    SparseTable(int rows = 0, int columns = 0)
        :row_count(rows), column_count(columns)
    {

    }

    int rowCount() const
    {
        return row_count;
    }
    int columnCount() const
    {
        return column_count;
    }



    template <typename T>
    void
    setCoords(typename std::enable_if<
                                      std::is_member_function_pointer<decltype(&T::setRow)>::value
                                      && std::is_member_function_pointer<decltype(&T::setColumn)>::value
                                      ,
                                      T>::type &cell,
              int row,
              int column)
    {
        cell.setRow(row);
        cell.setColumn(column);
    }

    template <typename T>
    void
    setCoords(T &cell, int row, int column)
    {

    }

    virtual void init(Cell &cell)
    {

    }

    Cell &getCell(int row, int column)
    {
        if(row >= row_count)row_count = row + 1;
        if(column >= column_count)column_count = column + 1;
        auto r = rows.find(row);
        if(r == rows.end())
        {
            r = rows.insert({row, Row()}).first;
        }
        auto c = r->second.find(column);
        if(c == r->second.end())
        {
            Cell cell;
            setCoords<Cell>(cell, row, column);
            init(cell);
            c = r->second.insert({column, cell}).first;
        }

        return c->second;
    }

    const Cell getCell(int row, int column) const
    {
        auto r = rows.find(row);
        if(r == rows.end())
        {
            return Cell();
        }
        auto c = r->second.find(column);
        if(c == r->second.end())
        {
            return Cell();
        }
        else
        {
            return c->second;
        }
    }
};

#endif // SPARSETABLE_H
