#include <issueswidget.h>
#include <control.h>
#include <pathfinder.h>
#include <utilityfunctions.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <issueslistdelegate.h>
#include <paintutils.h>

#include <QDir>
#include <QLabel>
#include <QTimer>
#include <QLayout>
#include <QPainter>
#include <QToolBar>
#include <QToolButton>
#include <QStyledItemDelegate>

enum Roles {
    ControlErrorsRole = Qt::UserRole + 1,
    QmlErrorIndexRole,
};

QIcon iconForQmlError(const QmlError& error)
{
    static QIcon info(":/images/output/info.svg"),
            warning(":/images/output/warning.svg"),
            critical(":/images/output/issue.svg");
    switch (error.messageType) {
    case QtInfoMsg:
    case QtDebugMsg:
        return info;
    case QtCriticalMsg:
    case QtFatalMsg:
        return critical;
    case QtWarningMsg: // FIXME: Fix this when Qt has a proper fix
        return /*warning*/critical;
    default:
        return info;
    }
}

IssuesWidget::IssuesWidget(QWidget* parent) : QListWidget(parent)
  , m_toolBar(new QToolBar(this))
  , m_iconLabel(new QLabel(this))
  , m_titleLabel(new QLabel(this))
  , m_clearButton(new QToolButton(this))
  , m_fontSizeUpButton(new QToolButton(this))
  , m_fontSizeDownButton(new QToolButton(this))
  , m_minimizeButton(new QToolButton(this))
{
    setItemDelegate(new IssuesListDelegate(this));
    setSelectionMode(NoSelection);
    setTextElideMode(Qt::ElideRight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setObjectName("m_listWidget");
    setStyleSheet("#m_listWidget { border: 1px solid #c4c4c4;"
                  "border-top: none; border-bottom: none;}");
    setFocusPolicy(Qt::NoFocus);

    m_toolBar->layout()->setSpacing(2);
    m_toolBar->layout()->setContentsMargins(1, 1, 1, 1);

    m_toolBar->setFixedHeight(22);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget(QSize(1, 1)));
    m_toolBar->addWidget(m_iconLabel);
    m_toolBar->addWidget(m_titleLabel);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(m_fontSizeUpButton);
    m_toolBar->addWidget(m_fontSizeDownButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_minimizeButton);

    m_iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/output/issue.svg"), QSize(16, 16), this));
    m_iconLabel->setFixedHeight(16);

    m_titleLabel->setText(tr("Issues"));
    m_titleLabel->setFixedSize(50, 20);

    m_clearButton->setFixedSize({20, 20});
    m_clearButton->setIcon(QIcon(":/images/designer/clear.svg"));
    m_clearButton->setToolTip(tr("Clean issues list"));
    m_clearButton->setCursor(Qt::PointingHandCursor);

    m_fontSizeUpButton->setFixedSize({20, 20});
    m_fontSizeUpButton->setIcon(QIcon(":/images/designer/font-increase.svg"));
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);

    m_fontSizeDownButton->setFixedSize({20, 20});
    m_fontSizeDownButton->setIcon(QIcon(":/images/designer/font-decrease.svg"));
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);

    m_minimizeButton->setFixedSize({20, 20});
    m_minimizeButton->setIcon(QIcon(":/images/designer/down.svg"));
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);

    connect(this, &QListWidget::itemDoubleClicked,
            this, &IssuesWidget::onItemDoubleClick);
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &IssuesWidget::minimized);
    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(this, -1);
    });
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(this, 1);
    });
    connect(m_clearButton, &QToolButton::clicked,
            this, &QListWidget::clear);
    connect(m_clearButton, &QToolButton::clicked,
            this, [=] {
        emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
    });
}

void IssuesWidget::clean()
{
    clear();
    qDeleteAll(m_erroneousControls);
    m_erroneousControls.clear();
}

void IssuesWidget::refresh(Control* control)
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

        if (!isVisible())
            emit flash();

        return titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
    }

    if (!controlErrors) {
        controlErrors = new ControlErrors;
        controlErrors->control = control;
        m_erroneousControls.append(controlErrors);
        connect(control, &Control::destroyed, this, &IssuesWidget::onControlDestruction);
    } else {
        controlErrors->errors.clear();
        for (int i = count() - 1; i >= 0; --i) {
            const ControlErrors* error = item(i)->data(ControlErrorsRole).value<const ControlErrors*>();
            if (error->control == control)
                delete takeItem(i);
        }
    }

    Q_ASSERT(control->hasErrors());

    for (const QmlError& error : control->errors()) {
        controlErrors->errors.append(error);
        auto item = new QListWidgetItem;
        item->setData(QmlErrorIndexRole, controlErrors->errors.size() - 1);
        item->setData(ControlErrorsRole, QVariant::fromValue<const ControlErrors*>(controlErrors));
        item->setData(Qt::ToolTipRole, PathFinder::locallyCleansed(error.toString()));
        item->setData(Qt::DisplayRole, PathFinder::locallyCleansed(error.toString()));
        item->setIcon(iconForQmlError(error));
        addItem(item);
    }

    if (!isVisible())
        emit flash();

    emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
}

void IssuesWidget::onItemDoubleClick(QListWidgetItem* item)
{
    const int errorIndex = item->data(QmlErrorIndexRole).toInt();
    const ControlErrors* controlErrors = item->data(ControlErrorsRole).value<const ControlErrors*>();
    const QmlError& error = controlErrors->errors.at(errorIndex);
    const QString& fullPath = error.url.toLocalFile();
    const QString& assetsPath = SaveUtils::toProjectAssetsDir(ProjectManager::dir());

    int line = error.line > 0 ? error.line : 0;
    int column = error.column > 0 ? error.column : 0;

    if (fullPath.contains(assetsPath)) {
        emit assetsFileOpened(QDir(assetsPath).relativeFilePath(fullPath), line, column);
    } else {
        emit designsFileOpened(controlErrors->control,
            QDir(SaveUtils::toControlThisDir(controlErrors->control->dir())).relativeFilePath(fullPath),
                               line, column);
    }
}

void IssuesWidget::onControlDestruction(QObject* controlObject)
{
    Q_ASSERT(controlObject);

    for (int i = count() - 1; i >= 0; --i) {
        const ControlErrors* error = item(i)->data(ControlErrorsRole).value<const ControlErrors*>();
        if (dynamic_cast<QObject*>(error->control) == controlObject)
            delete takeItem(i);
    }

    ControlErrors* controlErrors = nullptr;
    for (int i = 0; i < m_erroneousControls.size(); ++i) {
        if (dynamic_cast<QObject*>(m_erroneousControls.at(i)->control) == controlObject) {
            controlErrors = m_erroneousControls.at(i);
            m_erroneousControls.removeAt(i);
            break;
        }
    }

    if (controlErrors)
        delete controlErrors;

    if (!isVisible())
        emit flash();

    emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
}

void IssuesWidget::updateGeometries()
{
    QListWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setTop(m_toolBar->height());
    setViewportMargins(vm);
    m_toolBar->setGeometry(0, 0, viewport()->width() + 2, m_toolBar->height());
}

QSize IssuesWidget::sizeHint() const
{
    return {0, 100};
}

QSize IssuesWidget::minimumSizeHint() const
{
    return {0, 100};
}
