#include <centralwidget.h>
#include <outputpane.h>
#include <qmlcodeeditorwidget.h>
#include <designerwidget.h>
#include <projectoptionswidget.h>
#include <helpwidget.h>
#include <buildswidget.h>
#include <issuesbox.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>
#include <controlremovingmanager.h>
#include <delayer.h>

#include <QSplitter>
#include <QVBoxLayout>

CentralWidget::CentralWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_splitterOut(new QSplitter)
  , m_splitterIn(new QSplitter)
  , m_outputPane(new OutputPane)
  , m_qmlCodeEditorWidget(new QmlCodeEditorWidget)
  , m_designerWidget(new DesignerWidget(m_qmlCodeEditorWidget))
  , m_projectOptionsWidget(new ProjectOptionsWidget)
  , m_buildsWidget(new BuildsWidget)
  , m_helpWidget(new HelpWidget)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_splitterOut);

    m_splitterOut->setStyleSheet("QSplitter, QSplitter::handle { border: none }");
    m_splitterOut->setHandleWidth(0);
    m_splitterOut->setOrientation(Qt::Vertical);
    m_splitterOut->addWidget(m_splitterIn);
    m_splitterOut->addWidget(m_outputPane);

    m_splitterIn->setStyleSheet("QSplitter, QSplitter::handle { border: none }");
    m_splitterIn->setHandleWidth(0);
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterIn->addWidget(m_designerWidget);
    m_splitterIn->addWidget(m_qmlCodeEditorWidget);
    m_splitterIn->addWidget(m_projectOptionsWidget);
    m_splitterIn->addWidget(m_buildsWidget);
    m_splitterIn->addWidget(m_helpWidget);

    connect(m_outputPane->issuesBox(), SIGNAL(entryDoubleClicked(Control*)),
            m_designerWidget, SLOT(handleControlDoubleClick(Control*))); // FIXME: onControlDo.. is a private member
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            m_qmlCodeEditorWidget, &QmlCodeEditorWidget::onControlRemoval);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlPreviewingManager::scheduleTerminate);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlPreviewingManager::scheduleInit);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged, this, [=] {
        Delayer::delay(3000);
        m_designerWidget->refresh();
    });
}

DesignerWidget* CentralWidget::designerWidget() const
{
    return m_designerWidget;
}

OutputPane* CentralWidget::outputPane() const
{
    return m_outputPane;
}

void CentralWidget::sweep()
{
    setCurrentPage(Page_Designer);

    m_outputPane->sweep();
    m_qmlCodeEditorWidget->sweep();
    m_designerWidget->sweep();
    m_projectOptionsWidget->sweep();
    m_buildsWidget->sweep();
    m_helpWidget->sweep();
}

void CentralWidget::setCurrentPage(const Pages& page)
{
    hideWidgets();

    switch (page) {
        case Page_Builds:
            return m_buildsWidget->show();
            break;

        case Page_Designer:
            m_outputPane->show();
            return m_designerWidget->show();
            break;

        case Page_SplitView:
            m_outputPane->show();
            m_designerWidget->show();
            return m_qmlCodeEditorWidget->show();
            break;

        case Page_Help:
            return m_helpWidget->show();
            break;

        case Page_QmlCodeEditor:
            m_outputPane->show();
            return m_qmlCodeEditorWidget->show();
            break;

        case Page_ProjectOptions:
            return m_projectOptionsWidget->show();
            break;
    }
}

void CentralWidget::hideWidgets()
{
    m_outputPane->hide();
    m_designerWidget->hide();
    m_qmlCodeEditorWidget->hide();
    m_projectOptionsWidget->hide();
    m_buildsWidget->hide();
    m_helpWidget->hide();
}

QmlCodeEditorWidget* CentralWidget::qmlCodeEditorWidget() const
{
    return m_qmlCodeEditorWidget;
}

