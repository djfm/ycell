#ifndef SHEETCELL_H
#define SHEETCELL_H

#include <QString>
#include <QVariant>
#include <QSet>
#include <QStringList>

#include "refsolver.h"

class SheetModel;
class Sheet;

class SheetCell
{
    friend class SheetModel;

public:

    struct Ref
    {
        SheetModel *model;
        int row;
        int column;

        Ref(SheetModel *m, int r, int c);

        bool operator==(const Ref &other) const;
    };


    enum DataType
    {
        Number,
        String
    };
    
    SheetCell& setValue(const QVariant &val);
    const QVariant& getValue() const;
    DataType getDataType() const;
    bool hasFormula() const;
    const QString& getFormula() const;
    bool isEmpty() const;

    int getRow() const;
    int getColumn() const;
    void setRow(int r);
    void setColumn(int c);
    void setSheet(Sheet *tbl);

    void removeFromParentsChildren();
    void addParent(const RefSolver::Ref &, SheetModel *sheet);

    static QString toScriptValue(const QString &val);
    static QString toScriptValue(const QStringList &list);

    QString toString() const;

    void clearFormula();

    bool hasRange() const;
    void setHasRange(int rows, int columns);
    void setHasNoRange();
    int getRangeRows() const;
    int getRangeColumns() const;

private:

    int row;
    int column;

    bool has_range = false;
    int range_rows = 0;
    int range_columns = 0;


    DataType data_type = String;
    QVariant value = "";
    QString  formula;

    QSet<Ref> parents, children;

    SheetModel *model();
    Sheet *sheet;
    
};

uint qHash(const SheetCell::Ref &r);

#endif // SHEETCELL_H
