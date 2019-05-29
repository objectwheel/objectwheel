#include <issuespane.h>
#include <control.h>
#include <utilsicons.h>
#include <pathfinder.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>
#include <saveutils.h>
#include <projectmanager.h>

#include <QDir>
#include <QLabel>
#include <QTimer>
#include <QLayout>
#include <QPainter>
#include <QToolBar>
#include <QToolButton>
#include <QStyledItemDelegate>
#include <QQmlError>

enum Roles {
    ControlErrorsRole = Qt::UserRole + 1,
    QmlErrorIndexRole,
};

class IssuesListDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    IssuesListDelegate(QListWidget* parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
    {
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen("#c4c4c4");
        p->setBrush(Qt::NoBrush);
        p->fillRect(opt.rect, "#f0f0f0");
        p->drawLine(opt.rect.bottomLeft() + QPointF(0.5, 0.5),
                    opt.rect.bottomRight() + QPointF(0.5, 0.5));
        QStyledItemDelegate::paint(p, opt, index);
    }
};

IssuesPane::IssuesPane(QWidget* parent) : QListWidget(parent)
  , m_toolBar(new QToolBar(this))
  , m_titleLabel(new QLabel(this))
  , m_clearButton(new QToolButton(this))
  , m_fontSizeUpButton(new QToolButton(this))
  , m_fontSizeDownButton(new QToolButton(this))
  , m_minimizeButton(new QToolButton(this))
{
    setTextElideMode(Qt::ElideRight);
    setItemDelegate(new IssuesListDelegate(this));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setObjectName("m_listWidget");
    setStyleSheet("#m_listWidget { border: 1px solid #c4c4c4;"
                  "border-top: none; border-bottom: none;}");
    setIconSize({16, 16});
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, &QListWidget::itemDoubleClicked,
            this, &IssuesPane::onItemDoubleClick);

    m_titleLabel->setText("   " + tr("Issues") + "   ");
    m_titleLabel->setFixedHeight(20);

    m_toolBar->addWidget(m_titleLabel);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({4, 4}));
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(m_fontSizeUpButton);
    m_toolBar->addWidget(m_fontSizeDownButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_minimizeButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));
    m_toolBar->setIconSize({14, 14});
    TransparentStyle::attach(m_toolBar);

    m_clearButton->setFixedSize({18, 18});
    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_clearButton->setToolTip(tr("Clean issues list"));
    m_clearButton->setCursor(Qt::PointingHandCursor);
    connect(m_clearButton, &QToolButton::clicked,
            this, &QListWidget::clear);
    connect(m_clearButton, &QToolButton::clicked,
            this, [=] {
        emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
    });

    m_fontSizeUpButton->setFixedSize({18, 18});
    m_fontSizeUpButton->setIcon(Utils::Icons::PLUS_TOOLBAR.icon());
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(this, 1);
    });

    m_fontSizeDownButton->setFixedSize({18, 18});
    m_fontSizeDownButton->setIcon(Utils::Icons::MINUS.icon());
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(this, -1);
    });

    m_minimizeButton->setFixedSize({18, 18});
    m_minimizeButton->setIcon(Utils::Icons::CLOSE_SPLIT_BOTTOM.icon());
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &IssuesPane::minimized);

    QTimer::singleShot(200, [=] { // FIXME: Workaround for QToolBarLayout's obsolote serMargin function usage
        m_toolBar->setContentsMargins(0, 0, 0, 0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        m_toolBar->layout()->setSpacing(0);
        m_toolBar->setFixedHeight(22);
    });
}

void IssuesPane::discharge()
{
    clear();
    qDeleteAll(m_erroneousControls);
    m_erroneousControls.clear();
}

void IssuesPane::refresh(Control* control)
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
        item->setData(QmlErrorIndexRole, controlErrors->errors.size() - 1);
        item->setData(ControlErrorsRole, QVariant::fromValue<const ControlErrors*>(controlErrors));
        item->setData(Qt::ToolTipRole, PathFinder::locallyCleansed(error.toString()));
        item->setData(Qt::DisplayRole, PathFinder::locallyCleansed(error.toString()));
        item->setIcon(UtilityFunctions::iconForQmlError(error, this));
        addItem(item);
    }

    if (isHidden())
        emit flash();

    emit titleChanged(tr("Issues") + QString::fromUtf8(" [%1]").arg(count()));
}

void IssuesPane::onItemDoubleClick(QListWidgetItem* item)
{
    const int errorIndex = item->data(QmlErrorIndexRole).toInt();
    const ControlErrors* controlErrors = item->data(ControlErrorsRole).value<const ControlErrors*>();
    const QQmlError& error = controlErrors->errors.at(errorIndex);
    const QString& fullPath = error.url().toLocalFile();
    const QString& assetsPath = SaveUtils::toProjectAssetsDir(ProjectManager::dir());

    if (fullPath.contains(assetsPath)) {
        emit assetsFileOpened(QDir(assetsPath).relativeFilePath(fullPath), error.line(),
                              error.column());
    } else {
        emit designsFileOpened(controlErrors->control,
            QDir(SaveUtils::toControlThisDir(controlErrors->control->dir())).relativeFilePath(fullPath),
            error.line(), error.column());
    }
}

void IssuesPane::onControlDestruction(QObject* controlObject)
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
    m_toolBar->setGeometry(0, 0, viewport()->width() + 2, m_toolBar->height());
}

QSize IssuesPane::sizeHint() const
{
    return {0, 100};
}

QSize IssuesPane::minimumSizeHint() const
{
    return {0, 100};
}

#include "issuespane.moc"
