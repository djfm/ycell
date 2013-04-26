#include "celleditor.h"

#include <QKeyEvent>
#include <QDebug>

CellEditor::CellEditor(QWidget *parent) :
    QTextEdit(parent)
{
}

int CellEditor::getLastCursorPosition() const
{
    return last_cursor_position;
}

int CellEditor::getCursorPosition() const
{
    return textCursor().position();
}

void CellEditor::setCursorPosition(int pos)
{
     QTextCursor cursor = textCursor();
     cursor.setPosition(pos);
     setTextCursor(cursor);
}

void CellEditor::restoreLastCursorPosition()
{
    int length = toPlainText().length();
    if(length > 0)
    {
        if(last_cursor_position == 0)last_cursor_position = length;
        setCursorPosition(last_cursor_position);
    }
}

void CellEditor::setLastCursorPosition(int pos)
{
    last_cursor_position = pos;
}

void CellEditor::keyPressEvent(QKeyEvent *e)
{
    if(      e->key() == Qt::Key_Return
          || e->key() == Qt::Key_Left
          || e->key() == Qt::Key_Right
          || e->key() == Qt::Key_Up
          || e->key() == Qt::Key_Down)
    {
        e->ignore();
    }
    else
    {
        QTextEdit::keyPressEvent(e);
    }
}

void CellEditor::leaveEvent(QEvent *event)
{
    setLastCursorPosition(getCursorPosition());
    QTextEdit::leaveEvent(event);
}

void CellEditor::enterEvent(QEvent *event)
{
    restoreLastCursorPosition();
    QTextEdit::enterEvent(event);
}

void CellEditor::focusInEvent(QFocusEvent *e)
{
    restoreLastCursorPosition();
    QTextEdit::focusInEvent(e);
}
