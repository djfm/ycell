#ifndef SHEETVIEWDELEGATE_H
#define SHEETVIEWDELEGATE_H

#include <QStyledItemDelegate>

class SheetView;

class SheetViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    QModelIndex current_index;

public:
    explicit SheetViewDelegate(SheetView *parent = 0);

    bool eventFilter(QObject *object, QEvent *event);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

signals:
    void editorCreated(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void editorKeyPressed(int key) const;
    void leftEditor() const;

public slots:
    
private:

    SheetView *sheet = nullptr;

public:

};

#endif // SHEETVIEWDELEGATE_H
