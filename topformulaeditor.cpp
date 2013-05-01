#include "topformulaeditor.h"

#include <QDebug>
#include <QCoreApplication>
#include <QKeyEvent>

#include "sheetview.h"

TopFormulaEditor::TopFormulaEditor(QWidget *parent) :
    CellEditor(parent)
{

}

void TopFormulaEditor::setSheet(SheetView *s)
{
    sheet = s;
}

QSize TopFormulaEditor::sizeHint()
{
    return {250,30};
}

void TopFormulaEditor::keyPressEvent(QKeyEvent *e)
{
    if(e->key() != Qt::Key_Return || e->modifiers() & Qt::ControlModifier)
    {
        QTextEdit::keyPressEvent(e);
    }
    sheet->formulaBarKeyPressed(e);
}

void TopFormulaEditor::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);
}
