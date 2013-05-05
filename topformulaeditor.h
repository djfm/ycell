#ifndef TOPFORMULAEDITOR_H
#define TOPFORMULAEDITOR_H

#include "celleditor.h"

class SheetView;

class TopFormulaEditor : public CellEditor
{
    Q_OBJECT
public:
    explicit TopFormulaEditor(QWidget *parent = 0);
    
    void setSheet(SheetView *s);

    QSize sizeHint();

protected:

    void keyPressEvent(QKeyEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void mousePressEvent(QMouseEvent *e);

signals:
    
public slots:

private:
    SheetView *sheet = nullptr;
    
};

#endif // TOPFORMULAEDITOR_H
