#ifndef SHEETVIEW_H
#define SHEETVIEW_H

#include <QTableView>
#include <QItemEditorFactory>

#include "sheetviewdelegate.h"


class SheetView : public QTableView
{
    Q_OBJECT
    
public:
    explicit SheetView(QWidget *parent = 0);
    ~SheetView();

    QString getEditorText() const;
    void setEditorText(const QString &str, bool ignoreChange=false);
    void insertEditorText(const QString &str);
    void backspaceEditorText();
    void supprEditorText();

protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event) const;

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void on_delegate_editorCreated(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index);
    void editorTextChanged();
    void editorSelectionChanged();


protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);


private:

    bool ignore_editor_text_changed = false;
    bool expanding_formula = false;
    QModelIndex expand_from;
    SheetViewDelegate  *delegate;
    QItemEditorFactory *editor_factory;

    enum class State
    {
        Normal,
        EditingFormula
    };

    State state = State::Normal;

    QWidget *current_editor = nullptr;
};

#endif // SHEETVIEW_H
