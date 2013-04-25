#ifndef REFSOLVER_H
#define REFSOLVER_H

#include <functional>

#include <QRegExp>

class RefSolver
{
public:

    struct Ref
    {
        enum class Type
        {
            SingleCell,
            Range,
            NotARef
        };

        enum class RangeType
        {
            Row,
            Column,
            Rect,
            NotARange
        };

        struct Cell
        {
            int row;
            int column;
            bool rowFixed = false;
            bool columnFixed = false;
        };

        Ref();
        Ref(int tlRow, int tlColumn);
        Ref(int tlRow, int tlColumn, int brRow, int brColumn);

        bool forEachCell(std::function<void(int, int)> callback, int row_or_col_size=0) const;

        Type type = Type::NotARef;
        RangeType range_type = RangeType::NotARange;

        Cell topLeft,bottomRight;

        bool valid() const;
        QString toString() const;

    };

    typedef std::function<QString(const RefSolver::Ref &)> ExpCallback;

    static QRegExp getRegExp();

    static Ref resolve(QString str, int *pos = nullptr, int *length = nullptr);
    static QString forEachRef(const QString &str, ExpCallback callback);

    static QString numberToColumn(int n);
    static int columnToNumber(QString column);

    static bool test();
};

#endif // REFSOLVER_H
