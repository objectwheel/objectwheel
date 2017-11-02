#include <issuesbox.h>
#include <fit.h>
#include <css.h>
#include <control.h>
#include <outputwidget.h>
#include <designmanager.h>
#include <controlwatcher.h>

#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>

#define INTERVAL_ERROR_CHECK (1000)

using namespace Fit;

//!
//! *********************** [IssuesListDelegate] ***********************
//!

class IssuesListDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        IssuesListDelegate(QWidget* parent);
        void paint(QPainter* painter, const QStyleOptionViewItem &option,
          const QModelIndex &index) const override;
};

IssuesListDelegate::IssuesListDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

void IssuesListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    QStyledItemDelegate::paint(painter, option, index);

    auto f = option.font;
    auto r = QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5);
    Error error = model->data(index, Qt::UserRole).value<Error>();
    painter->setPen("#a0a4a7");
    painter->drawLine(r.bottomLeft(), r.bottomRight());
    painter->setPen(option.palette.Text);
    f.setWeight(QFont::Medium);
    painter->setFont(f);
    painter->drawText(r.adjusted(fit(26), 0, 0, 0),
      error.id + ":", Qt::AlignVCenter | Qt::AlignLeft);
    QFontMetrics fm(f);
    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->drawText(r.adjusted(fit(26) + fm.width(error.id) + fit(8),0,0,0),
      error.description, Qt::AlignVCenter | Qt::AlignLeft);
    painter->drawText(r, QString("Line: %1, Col: %2 ").
      arg(error.line).arg(error.column), Qt::AlignVCenter | Qt::AlignRight);
}

//!
//! ************************* [IssuesBox] *************************
//!

IssuesBox::IssuesBox(OutputWidget* outputWidget)
    : QWidget(outputWidget)
    , _layout(this)
    , _outputWidget(outputWidget)
{
    _layout.setContentsMargins(0, 0, 0, 0);
    _layout.setSpacing(0);
    _layout.addWidget(&_toolbar);
    _layout.addWidget(&_listWidget);

    QPalette p1(_listWidget.palette());
    p1.setColor(QPalette::Base, QColor("#F3F7FA"));
    p1.setColor(QPalette::Highlight, QColor("#d0d4d7"));
    p1.setColor(QPalette::Text, QColor("#202427"));
    _listWidget.setPalette(p1);

    _title.setText(" Issues");
    _toolbar.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _toolbar.setFixedHeight(fit(21));
    _toolbar.setAutoFillBackground(true);
    _toolbar.setStyleSheet(CSS::DesignerToolbar);
    _toolbar.addWidget(&_title);

    _listWidget.setIconSize(fit({16, 16}));
    _listWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _listWidget.setFocusPolicy(Qt::NoFocus);
    _listWidget.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _listWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget.setItemDelegate(new IssuesListDelegate(&_listWidget));
    connect(ControlWatcher::instance(), SIGNAL(errorOccurred(Control*)),
      SLOT(handleErrors(Control*)));

    _checkTimer.setInterval(INTERVAL_ERROR_CHECK);
    connect(&_checkTimer, SIGNAL(timeout()), SLOT(checkErrors()));

    connect(&_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
      SLOT(handleDoubleClick(QListWidgetItem*)));
}

void IssuesBox::handleErrors(Control* control)
{
    checkErrors();
    if (control->hasErrors()) {
        for (const auto& error : control->errors()) {
            Error err;
            err.id = control->id();
            err.uid = control->uid();
            err.description = error.description();
            err.line = error.line();
            err.column = error.column();
            err.mode = control->mode();
            if (_buggyControls.contains(err))
                continue;
            auto item = new QListWidgetItem;
            item->setData(Qt::UserRole, QVariant::fromValue<Error>(err));
            item->setIcon(QIcon(":/resources/images/error.png"));
            _listWidget.addItem(item);
            item->setHidden(_currentMode != err.mode);
            _buggyControls[err] = control;
        }
    }
    _checkTimer.start();
}

void IssuesBox::handleDoubleClick(QListWidgetItem* item)
{
    const auto& error = item->data(Qt::UserRole).value<Error>();
    const auto& c = _buggyControls.value(error);

    if (!c)
        return;

    DesignManager::qmlEditorView()->addControl(c);
    if (DesignManager::qmlEditorView()->pinned())
        DesignManager::setMode(CodeEdit);
    DesignManager::qmlEditorView()->setMode(QmlEditorView::CodeEditor);
    DesignManager::qmlEditorView()->openControl(c);
    DesignManager::qmlEditorView()->raiseContainer();
}

void IssuesBox::setCurrentMode(const DesignMode& currentMode)
{
    if (currentMode == FormGui || currentMode == ControlGui)
        _currentMode = currentMode;
}

void IssuesBox::checkErrors()
{
    for (const auto& err : _buggyControls.keys()) {
        auto control = _buggyControls.value(err);
        if (control.isNull() || !control->hasErrors()) {
            for (int i = 0; i < _listWidget.count(); i++) {
                auto item = _listWidget.item(i);
                if (item->data(Qt::UserRole).value<Error>() == err)
                    delete _listWidget.takeItem(i);
            }
            _buggyControls.remove(err);
        } else {
            QList<Error> es;
            for (const auto& error : control->errors()) {
                Error e;
                e.id = control->id();
                e.uid = control->uid();
                e.description = error.description();
                e.line = error.line();
                e.column = error.column();
                es << e;
            }
            if (!es.contains(err)) {
                for (int i = 0; i < _listWidget.count(); i++) {
                    auto item = _listWidget.item(i);
                    if (item->data(Qt::UserRole).value<Error>() == err)
                        delete _listWidget.takeItem(i);
                }
                _buggyControls.remove(err);
            }
        }
    }

    int visibleItemCount = 0;
    for (int i = 0; i < _listWidget.count(); i++) {
        auto item = _listWidget.item(i);
        auto err = item->data(Qt::UserRole).value<Error>();
        bool hidden = err.mode != _currentMode;
        item->setHidden(hidden);
        if (!hidden)
            visibleItemCount++;
    }

    _outputWidget->button(Issues)->setText
      (QString("Issues [%1]").arg(visibleItemCount));

    if (_buggyControls.isEmpty())
        _checkTimer.stop();
}

bool operator<(const Error& e1, const Error& e2)
{
    return e1.uid < e2.uid;
}

#include "issuesbox.moc"
