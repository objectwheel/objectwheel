#include <centralwidget.h>
#include <outputpane.h>
#include <designerwidget.h>
#include <projectoptionswidget.h>
#include <helpwidget.h>
#include <buildswidget.h>
#include <issuesbox.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>
#include <controlremovingmanager.h>
#include <delayer.h>
#include <qmlcodeeditorwidget.h>
#include <qmlcodedocument.h>
#include <savemanager.h>
#include <parserutils.h>
#include <controlsavefilter.h>
#include <utilityfunctions.h>

#include <QWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QStyle>
#include <QLabel>

class EditorContainer : public QLabel {
public: explicit EditorContainer(QWidget* parent) : QLabel(parent) {}
public: QSize sizeHint() const override { return {680, 680}; }
};

namespace { EditorContainer* g_editorContainer; }

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

    g_editorContainer = new EditorContainer(this);
    g_editorContainer->setAlignment(Qt::AlignCenter);
    g_editorContainer->setText(tr("Editor window\nraised"));
    g_editorContainer->setStyleSheet("QLabel { background: transparent; color: #808080;}");
    g_editorContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    g_editorContainer->setLayout(new QVBoxLayout(g_editorContainer));
    g_editorContainer->layout()->setSpacing(0);
    g_editorContainer->layout()->setContentsMargins(0, 0, 0, 0);
    g_editorContainer->layout()->addWidget(m_qmlCodeEditorWidget);

    m_splitterIn->setStyleSheet("QSplitter, QSplitter::handle { border: none }");
    m_splitterIn->setHandleWidth(0);
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterIn->addWidget(m_designerWidget);
    m_splitterIn->addWidget(g_editorContainer);
    m_splitterIn->addWidget(m_projectOptionsWidget);
    m_splitterIn->addWidget(m_buildsWidget);
    m_splitterIn->addWidget(m_helpWidget);

    connect(m_outputPane->issuesBox(), SIGNAL(entryDoubleClicked(Control*)),
            m_designerWidget, SLOT(onControlDoubleClick(Control*))); // FIXME: onControlDo.. is a private member

    m_qmlCodeEditorWidget->addSaveFilter(new ControlSaveFilter(this)); // Changes made in code editor
    connect(SaveManager::instance(), &SaveManager::propertyChanged,    // Changes made out of code editor
            this, [=] (Control* control, const QString& property, const QString& value) {
        QmlCodeEditorWidget::InternalDocument* document = qmlCodeEditorWidget()->getInternal(control, "main.qml");
        if (document)
            ParserUtils::setProperty(document->document, control->url(), property, value);
    });
    //   BUG connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
    //            m_qmlCodeEditorWidget, &QmlCodeEditorWidget::onControlRemoval);

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
        return g_editorContainer->show();
        break;

    case Page_Help:
        return m_helpWidget->show();
        break;

    case Page_QmlCodeEditor:
        m_outputPane->show();
        return g_editorContainer->show();
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
    g_editorContainer->hide();
    m_projectOptionsWidget->hide();
    m_buildsWidget->hide();
    m_helpWidget->hide();
}

QmlCodeEditorWidget* CentralWidget::qmlCodeEditorWidget() const
{
    return m_qmlCodeEditorWidget;
}

