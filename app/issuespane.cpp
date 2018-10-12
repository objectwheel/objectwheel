#include <issuespane.h>
#include <control.h>
#include <utilsicons.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>
#include <controlpropertymanager.h>

#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTimer>
#include <QQmlError>

namespace {

QIcon iconForError(const QQmlError& error, const QWidget* widget)
{
    QStyleOption opt;
    opt.initFrom(widget);
    switch (error.messageType()) {
    case QtInfoMsg:
    case QtDebugMsg:
        return widget->style()->standardIcon(QStyle::SP_MessageBoxInformation, &opt, widget);
    case QtCriticalMsg:
    case QtFatalMsg:
        return widget->style()->standardIcon(QStyle::SP_MessageBoxCritical, &opt, widget);
    case QtWarningMsg:
        return widget->style()->standardIcon(QStyle::SP_MessageBoxWarning, &opt, widget);
    }
}
}
enum Roles {
    ControlErrorsRole = Qt::UserRole + 1,
    QmlErrorIndexRole
};

class IssuesListDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    IssuesListDelegate(QListWidget* parent) : QStyledItemDelegate(parent)
      , m_listWidget(parent)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        const QAbstractItemModel* model = index.model();
        Q_ASSERT(model);
        painter->setRenderHint(QPainter::Antialiasing);

        const int errorIndex = index.data(QmlErrorIndexRole).toInt();
        const IssuesPane::ControlErrors* controlErrors
                = index.data(ControlErrorsRole).value<const IssuesPane::ControlErrors*>();
        const QQmlError& error = controlErrors->errors.at(errorIndex);
        const int lspace = 3 - QString::number(error.line()).size();
        const int cspace = 3 - QString::number(error.column()).size();

        QString laddition;
        if (lspace > 0) {
            for (int i = 0; i < lspace; ++i)
                laddition.append(" ");
        }

        QString caddition;
        if (cspace > 0) {
            for (int i = 0; i < cspace; ++i)
                caddition.append(" ");
        }

        QString text("%1 %2 %3");
        QString desc(error.description());
        QString path(error.url().toString());
        QString lineCol(("\t" + tr("Line") + ":%1%2, " + tr("Col") + ":%3%4")
                        .arg(laddition).arg(error.line()).arg(caddition).arg(error.column()));

        QStyleOptionViewItem copy(option);
        copy.text = text.arg(path).arg(desc).arg(lineCol);
        QStyledItemDelegate::paint(painter, copy, index);
    }

private:
    QListWidget* m_listWidget;
};

IssuesPane::IssuesPane(QWidget* parent) : QListWidget(parent)
  , m_toolBar(new QToolBar(this))
  , m_titleLabel(new QLabel(this))
  , m_clearButton(new QToolButton(this))
  , m_fontSizeUpButton(new QToolButton(this))
  , m_fontSizeDownButton(new QToolButton(this))
  , m_minimizeButton(new QToolButton(this))
{

    setTextElideMode(Qt::ElideMiddle);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setObjectName("m_listWidget");
    setStyleSheet("#m_listWidget { border: 1px solid #c4c4c4;"
                  "border-top: none; border-bottom: none;}");
    setIconSize({16, 16});
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegate(new IssuesListDelegate(this));
    connect(this, &QListWidget::itemDoubleClicked,
            this, &IssuesPane::onItemDoubleClick);

    m_titleLabel->setText("   " + tr("Issues") + "   ");
    m_titleLabel->setFixedHeight(22);

    m_toolBar->addWidget(m_titleLabel);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(m_fontSizeUpButton);
    m_toolBar->addWidget(m_fontSizeDownButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_minimizeButton);
    m_toolBar->setFixedHeight(24);

    m_clearButton->setFixedHeight(22);
    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_clearButton->setToolTip(tr("Clean issues list"));
    m_clearButton->setCursor(Qt::PointingHandCursor);
    connect(m_clearButton, &QToolButton::clicked,
            this, &QListWidget::clear);

    m_fontSizeUpButton->setFixedHeight(22);
    m_fontSizeUpButton->setIcon(Utils::Icons::PLUS_TOOLBAR.icon());
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(this, 1);
    });

    m_fontSizeDownButton->setFixedHeight(22);
    m_fontSizeDownButton->setIcon(Utils::Icons::MINUS.icon());
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(this, -1);
    });

    m_minimizeButton->setFixedHeight(22);
    m_minimizeButton->setIcon(Utils::Icons::CLOSE_SPLIT_BOTTOM.icon());
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &IssuesPane::minimized);

    connect(ControlPropertyManager::instance(), &ControlPropertyManager::previewChanged,
            this, [=] (Control* control, int codeChanged) {
        if (codeChanged)
            update(control);
    });

    TransparentStyle::attach(m_toolBar);
    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        m_toolBar->setContentsMargins(0, 0, 0, 0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        m_toolBar->layout()->setSpacing(0);
    });
}

void IssuesPane::sweep()
{
    clear();
    qDeleteAll(m_erroneousControls);
    m_erroneousControls.clear();
}

void IssuesPane::update(Control* control)
{
    ControlErrors* controlErrors = nullptr;
    for (ControlErrors* error : m_erroneousControls) {
        if (error->control == control)
            controlErrors = error;
    }

    if (!controlErrors && !control->hasErrors())
        return;

    if (controlErrors && !control->hasErrors()) {
        for (int i = count() - 1; i >= 0; --i) {
            const ControlErrors* error = item(i)->data(ControlErrorsRole).value<const ControlErrors*>();
            if (error->control == control)
                delete takeItem(i);
        }

        for (int i = 0; i < m_erroneousControls.size(); ++i) {
            if (m_erroneousControls.at(i)->control == control) {
                m_erroneousControls.removeAt(i);
                break;
            }
        }

        control->disconnect(this);

        delete controlErrors;

        if (isHidden())
            emit flash();

        return titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
    }

    if (!controlErrors) {
        controlErrors = new ControlErrors;
        controlErrors->control = control;
        m_erroneousControls.append(controlErrors);
        connect(control, &Control::destroyed, this, &IssuesPane::onControlDestruction);
    } else {
        controlErrors->errors.clear();
        for (int i = count() - 1; i >= 0; --i) {
            const ControlErrors* error = item(i)->data(ControlErrorsRole).value<const ControlErrors*>();
            if (error->control == control)
                delete takeItem(i);
        }
    }

    Q_ASSERT(control->hasErrors());

    for (const QQmlError& error : control->errors()) {
        controlErrors->errors.append(error);
        auto item = new QListWidgetItem;
        item->setData(ControlErrorsRole, QVariant::fromValue<const ControlErrors*>(controlErrors));
        item->setData(QmlErrorIndexRole, controlErrors->errors.size() - 1);
        item->setData(Qt::ToolTipRole, error.toString());
        item->setIcon(iconForError(error, this));
        addItem(item);
    }

    if (isHidden())
        emit flash();

    emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
}

void IssuesPane::onItemDoubleClick(QListWidgetItem* item)
{
    const ControlErrors* controlErrors = item->data(ControlErrorsRole).value<const ControlErrors*>();
    emit controlDoubleClicked(controlErrors->control);
}

void IssuesPane::onControlDestruction(QObject* controlObject)
{
    Control* control = qobject_cast<Control*>(controlObject);
    Q_ASSERT(control);

    for (int i = count() - 1; i >= 0; --i) {
        const ControlErrors* error = item(i)->data(ControlErrorsRole).value<const ControlErrors*>();
        if (error->control == control)
            delete takeItem(i);
    }

    ControlErrors* controlErrors = nullptr;
    for (int i = 0; i < m_erroneousControls.size(); ++i) {
        if (m_erroneousControls.at(i)->control == control) {
            controlErrors = m_erroneousControls.at(i);
            m_erroneousControls.removeAt(i);
            break;
        }
    }

    Q_ASSERT(controlErrors);

    delete controlErrors;

    if (isHidden())
        emit flash();

    emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
}

void IssuesPane::updateGeometries()
{
    QListWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setTop(m_toolBar->height());
    setViewportMargins(vm);
    QRect tg(0, 0, width(), m_toolBar->height());
    m_toolBar->setGeometry(tg);
}

QSize IssuesPane::sizeHint() const
{
    return {100, 100};
}

QSize IssuesPane::minimumSizeHint() const
{
    return {100, 100};
}

#include "issuespane.moc"
