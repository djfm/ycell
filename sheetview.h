#ifndef SHEETVIEW_H
#define SHEETVIEW_H

#include <QTableView>
#include <QItemEditorFactory>
#include <QKeyEvent>

#include "sheetviewdelegate.h"
#include "topformulaeditor.h"


class SheetView : public QTableView
{
    Q_OBJECT
    
public:

    enum class State
    {
        Normal,
        EditingFormula
    };

    explicit SheetView(QWidget *parent = 0);
    ~SheetView();

    QString getEditorText() const;
    void setEditorText(const QString &str, bool ignoreChange=false);
    void insertEditorText(const QString &str);
    void backspaceEditorText();
    void supprEditorText();
    void setFormulaBarWidget(TopFormulaEditor *ed);
    void formulaBarKeyPressed(QKeyEvent *e);

    State getState() const;

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
    void editorSelectionChanged();
    void editorTextChanged();
    void editorClicked();
    void formulaBarTextChanged();
    void formulaBarClicked();

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);


private:

    void submitCell();

    TopFormulaEditor *formulaBar = nullptr;
    bool ignore_editor_text_changed = false;
    bool expanding_formula = false;
    QModelIndex expand_from;
    SheetViewDelegate  *delegate;
    QItemEditorFactory *editor_factory;

    State state = State::Normal;
    bool editingInTopFormulaBar = false;

    QWidget *current_editor = nullptr;
};

#endif // SHEETVIEW_H
