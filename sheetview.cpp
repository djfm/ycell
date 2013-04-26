#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>
#include <QKeyEvent>
#include <QRegExp>
#include <QItemSelection>

#include "sheetview.h"
#include "celleditor.h"
#include "refsolver.h"
#include "sheetmodel.h"

SheetView::SheetView(QWidget *parent) :
    QTableView(parent)
{
    delegate = new SheetViewDelegate(this);
    delegate->setObjectName("delegate");

    /*
    editor_factory = new QItemEditorFactory;
    QItemEditorCreatorBase *cellEditor = new QStandardItemEditorCreator<CellEditor>();
    editor_factory->registerEditor(QVariant::String, cellEditor);
    editor_factory->registerEditor(QVariant::, cellEditor);
    delegate->setItemEditorFactory(editor_factory);*/

    setItemDelegate(delegate);
    QMetaObject::connectSlotsByName(this);
}

SheetView::~SheetView()
{
}

QString SheetView::getEditorText() const
{
    return current_editor->metaObject()->userProperty().read(current_editor).toString();
}

void SheetView::setEditorText(const QString &str, bool ignoreChange)
{
    bool last_ignore_editor_text_changed = ignore_editor_text_changed;
    ignore_editor_text_changed = ignoreChange;
    current_editor->metaObject()->userProperty().write(current_editor, str);
    ignore_editor_text_changed = last_ignore_editor_text_changed;
}

void SheetView::insertEditorText(const QString &str)
{
    CellEditor *ed = qobject_cast<CellEditor*>(current_editor);
    QString text = ed->toPlainText();
    int pos = ed->getLastCursorPosition();
    text.insert(pos, str);
    pos += str.length();
    setEditorText(text, true);
    ed->setLastCursorPosition(pos);
}

void SheetView::backspaceEditorText()
{
    CellEditor *ed = qobject_cast<CellEditor*>(current_editor);
    QString text = ed->toPlainText();
    int pos = ed->getLastCursorPosition();
    if(pos == 0 || text.isEmpty())return;
    text.remove(pos - 1, 1);
    pos -= 1;
    setEditorText(text, true);
    ed->setLastCursorPosition(pos);
}

void SheetView::supprEditorText()
{
    CellEditor *ed = qobject_cast<CellEditor*>(current_editor);
    QString text = ed->toPlainText();
    int pos = ed->getLastCursorPosition();
    if(pos == text.length())return;
    text.remove(pos, 1);
    setEditorText(text, true);
    ed->setLastCursorPosition(pos);
}

void SheetView::keyPressEvent(QKeyEvent *event)
{
    if((event->modifiers() & Qt::ControlModifier)
            &&(
                    event->key() == Qt::Key_Right
                ||  event->key() == Qt::Key_Left
                ||  event->key() == Qt::Key_Up
                ||  event->key() == Qt::Key_Down
              )
      )
    {
        QModelIndex current = selectionModel()->currentIndex();
        selectionModel()->setCurrentIndex(qobject_cast<SheetModel*>(model())->jumpIndex(current, event->key()), QItemSelectionModel::ClearAndSelect);
    }
    else if(state == State::EditingFormula)
    {
        if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return)
        {
            state = State::Normal;

            current_editor->disconnect(SIGNAL(textChanged()));
            delegate->commitData(current_editor);
            closeEditor(current_editor, QAbstractItemDelegate::EndEditHint::NoHint);

            event->accept();
        }
        else if(event->key() == Qt::Key_Backspace)
        {
            backspaceEditorText();
            event->accept();
        }
        else if(event->key() == Qt::Key_Delete)
        {
            supprEditorText();
            event->accept();
        }
        else if(!event->text().isEmpty())
        {
            insertEditorText(event->text());
            event->accept();
        }
        else
        {
            QTableView::keyPressEvent(event);
        }
    }
    else if(state == State::Normal && current_editor != nullptr)
    {
        if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return)
        {
            current_editor->disconnect(SIGNAL(textChanged()));
            delegate->commitData(current_editor);
            closeEditor(current_editor, QAbstractItemDelegate::EndEditHint::NoHint);
        }
        else
        {
            QTableView::keyPressEvent(event);
        }
    }
    else
    {
        if(event->key() == Qt::Key_Return)
        {
            QItemSelection selection = selectionModel()->selection();
            if(selection.count() == 1)
            {
                QItemSelectionRange &range                 = selection.back();
                const QPersistentModelIndex &topLeft       = range.topLeft();
                const QPersistentModelIndex &bottomRight   = range.bottomRight();

                if(topLeft == bottomRight)
                {
                    QModelIndex index = model()->index(topLeft.row(), topLeft.column());
                    QTableView::setCurrentIndex(index);
                    QTableView::edit(index);
                }

                /*ref = RefSolver::Ref(topLeft.row()+1, topLeft.column()+1, bottomRight.row()+1, bottomRight.column()+1);
                qDebug()<<"Selected"<<ref.toString();*/
            }
        }
        else if(event->key() == Qt::Key_Equal)
        {
            QItemSelection selection = selectionModel()->selection();
            if(selection.count() == 1)
            {
                QItemSelectionRange &range                 = selection.back();
                const QPersistentModelIndex &topLeft       = range.topLeft();
                const QPersistentModelIndex &bottomRight   = range.bottomRight();

                if(topLeft == bottomRight)
                {
                    QModelIndex index = model()->index(topLeft.row(), topLeft.column());

                    QString formula = index.data(Qt::EditRole).toString();
                    if(!formula.isEmpty() and formula.at(0) == '=')
                    {
                        QTableView::setCurrentIndex(index);
                        QTableView::edit(index);
                    }
                    else
                    {
                        QTableView::keyPressEvent(event);
                    }
                }
            }
        }
        else
        {
            QTableView::keyPressEvent(event);
        }
    }
}

bool SheetView::eventFilter(QObject *object, QEvent *event)
{
    if(state == State::EditingFormula)
    {
        //qDebug()<<"TableViewEvent:"<<event;
    }

    return QTableView::eventFilter(object, event);
}

bool SheetView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    if(state == State::EditingFormula)
    {
        return false;
    }
    else return QTableView::edit(index, trigger, event);
}

void SheetView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected, deselected);

    CellEditor *ed = qobject_cast<CellEditor*>(current_editor);

    RefSolver::Ref ref;

    if(state == State::EditingFormula)
    {
        QItemSelection selection = selectionModel()->selection();
        if(selection.count() == 1)
        {
            QItemSelectionRange &range                 = selection.back();
            const QPersistentModelIndex &topLeft       = range.topLeft();
            const QPersistentModelIndex &bottomRight   = range.bottomRight();

            ref = RefSolver::Ref(topLeft.row()+1, topLeft.column()+1, bottomRight.row()+1, bottomRight.column()+1);
            qDebug()<<"Selected"<<ref.toString();
        }

        if(ref.valid())
        {
            QString cell_name = ref.toString();

            QString formula = getEditorText();
            int pos = ed->getLastCursorPosition();
            int ltrim = 0;
            int rtrim = 0;

            QRegExp fxp = RefSolver::getRegExp();

            int m = 0;
            while((m = fxp.indexIn(formula, m)) != -1)
            {

                if(pos >= m && pos <= m + fxp.matchedLength())
                {
                    ltrim = pos - m;
                    rtrim = m + fxp.matchedLength() - pos;
                    break;
                }

                m += fxp.matchedLength();
            }

            QString left  = formula.left(pos - ltrim);
            QString right = formula.mid(pos + rtrim);

            formula       = left + cell_name + right;

            setEditorText(formula, true);

            ed->setLastCursorPosition(pos - ltrim + cell_name.length());

            pos = ed->getLastCursorPosition();
        }
    }
}

void SheetView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{

}

void SheetView::on_delegate_editorCreated(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(option);

    if(state == State::EditingFormula)
    {
        qDebug()<<"Two editors open?";
    }

    if(!index.data(Qt::EditRole).toString().isEmpty() && index.data(Qt::EditRole).toString().at(0) == '=')
    {
        state = State::EditingFormula;
    }
    current_editor = editor;
    connect(current_editor, SIGNAL(textChanged()), this, SLOT(editorTextChanged()));
    connect(current_editor, SIGNAL(selectionChanged()), this, SLOT(editorSelectionChanged()));
}

void SheetView::editorTextChanged()
{
    if(ignore_editor_text_changed)return;

    QString text = getEditorText();

    CellEditor *ed = qobject_cast<CellEditor*>(current_editor);
    int pos = ed->getCursorPosition();
    if(!text.isEmpty() && pos > 0)ed->setLastCursorPosition(pos);

    if(!text.isEmpty() && text.at(0) == '=' && state != State::EditingFormula)
    {
        state = State::EditingFormula;
    }
}

void SheetView::editorSelectionChanged()
{
}


void SheetView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    bool reallyClose = state != State::EditingFormula;

    if(reallyClose)
    {
        if(state == State::EditingFormula)
        {
            state = State::Normal;
        }
        current_editor = nullptr;
        QTableView::closeEditor(editor, hint);
    }
    else
    {

    }

}
