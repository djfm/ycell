#ifndef CELLEDITOR_H
#define CELLEDITOR_H

#include <QTextEdit>

class CellEditor : public QTextEdit
{
    Q_OBJECT
    Q_PROPERTY(QString plainText READ toPlainText WRITE setPlainText USER true)
public:
    explicit CellEditor(QWidget *parent = 0);

    int getLastCursorPosition() const;
    int getCursorPosition() const;
    void setCursorPosition(int pos);

    void restoreLastCursorPosition();
    void setLastCursorPosition(int pos);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void leaveEvent(QEvent *event);
    virtual void enterEvent(QEvent *event);
    virtual void focusInEvent(QFocusEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);

signals:

    void clicked() const;
    
public slots:

private:

    int last_cursor_position = 0;

    
};

#endif // CELLEDITOR_H
