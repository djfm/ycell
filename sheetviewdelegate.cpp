#include <QPainter>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

#include "sheetview.h"
#include "sheetviewdelegate.h"
#include "sheetcell.h"
#include "celleditor.h"

SheetViewDelegate::SheetViewDelegate(SheetView *parent) :
    QStyledItemDelegate(parent),
    sheet(parent)
{

}

bool SheetViewDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::KeyPress && object == this)
    {
        QKeyEvent* e = static_cast<QKeyEvent*>(event);
        emit editorKeyPressed(e->key());
    }
    else if(event->type() == QEvent::Leave)
    {
        emit leftEditor();
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

QWidget *SheetViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *editor = new CellEditor(parent); //QStyledItemDelegate::createEditor(parent, option, index);

    emit editorCreated(editor, option, index);

    return editor;
}

void SheetViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    switch(index.data(Qt::UserRole).toInt())
    {
        case SheetCell::Number:
        opt.displayAlignment = Qt::AlignRight | Qt::AlignCenter;
        break;
    }
    painter->save();
    QStyledItemDelegate::paint(painter, opt, index);
    painter->restore();
}

void SheetViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void SheetViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(sheet->getState() != SheetView::State::EditingFormula)
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
