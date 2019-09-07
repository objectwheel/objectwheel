#include <centralwidget.h>
#include <designerview.h>
#include <projectoptionswidget.h>
#include <helpwidget.h>
#include <buildswidget.h>
#include <controlrenderingmanager.h>
#include <delayer.h>
#include <qmlcodeeditorwidget.h>
#include <qmlcodedocument.h>
#include <savemanager.h>
#include <parserutils.h>
#include <controlsavefilter.h>
#include <utilityfunctions.h>
#include <qmlcodeeditor.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <designerscene.h>
#include <designerpane.h>
#include <designercontroller.h>
#include <outputpane.h>
#include <outputcontroller.h>
#include <issueswidget.h>
#include <consolewidget.h>
#include <form.h>

#include <QBoxLayout>
#include <QLabel>

class EditorContainer : public QLabel {
public: explicit EditorContainer(QWidget* parent) : QLabel(parent) {}
public: QSize sizeHint() const override { return {680, 680}; }
};

namespace { EditorContainer* g_editorContainer; }

CentralWidget::CentralWidget(QWidget* parent) : QSplitter(parent)
  , m_splitterIn(new QSplitter)
  , m_outputPane(new OutputPane)
  , m_outputController(new OutputController(m_outputPane, this))
  , m_designerPane(new DesignerPane)
  , m_designerController(new DesignerController(m_designerPane, this))
  , m_qmlCodeEditorWidget(new QmlCodeEditorWidget)
  , m_projectOptionsWidget(new ProjectOptionsWidget)
  , m_buildsWidget(new BuildsWidget)
  , m_helpWidget(new HelpWidget)
{
    setHandleWidth(0);
    setFrameShape(QFrame::NoFrame);
    setOrientation(Qt::Vertical);
    addWidget(m_splitterIn);
    addWidget(m_outputPane);
    setChildrenCollapsible(false);

    g_editorContainer = new EditorContainer(this);
    g_editorContainer->setAlignment(Qt::AlignCenter);
    g_editorContainer->setObjectName("g_editorContainer");
    g_editorContainer->setText(tr("Editor window\nraised"));
    g_editorContainer->setStyleSheet("#g_editorContainer { background: transparent; color: #808080;}");
    g_editorContainer->setLayout(new QHBoxLayout);
    g_editorContainer->layout()->setSpacing(0);
    g_editorContainer->layout()->setContentsMargins(0, 0, 0, 0);
    g_editorContainer->layout()->addWidget(UtilityFunctions::createSeparatorWidget(Qt::Vertical));
    g_editorContainer->layout()->addWidget(m_qmlCodeEditorWidget);

    m_splitterIn->setHandleWidth(0);
    m_splitterIn->setFrameShape(QFrame::NoFrame);
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterIn->addWidget(m_designerPane);
    m_splitterIn->addWidget(g_editorContainer);
    m_splitterIn->addWidget(m_projectOptionsWidget);
    m_splitterIn->addWidget(m_buildsWidget);
    m_splitterIn->addWidget(m_helpWidget);
    m_splitterIn->setChildrenCollapsible(false);

    onModeChange(ModeManager::mode());

    m_qmlCodeEditorWidget->addSaveFilter(new ControlSaveFilter(this)); // Changes made in code editor
    connect(SaveManager::instance(), &SaveManager::propertyChanged,    // Changes made out of code editor
            this, [=] (Control* control, const QString& property, const QString& value) {
        if (!control->gui() && (property == "x" || property == "y"))
            return;
        QmlCodeEditorWidget::DesignsDocument* document = qmlCodeEditorWidget()->getDesigns(control, SaveUtils::controlMainQmlFileName());
        if (document)
            ParserUtils::setProperty(document->document, control->dir(), property, value);
    });
    connect(SaveManager::instance(), &SaveManager::formConnectionsDone,
            this, [=] (const QString& FormJS, const QString& id) {
        qmlCodeEditorWidget()->openAssets(id + ".js");
        qmlCodeEditorWidget()->codeEditor()->gotoLine(4);

        QmlCodeEditorWidget::AssetsDocument* qmldirDoc = qmlCodeEditorWidget()->getAssets("qmldir");
        if (qmldirDoc) {
            bool modified = qmldirDoc->document->isModified();
            QTextCursor cursor(qmldirDoc->document);
            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText("\n" + FormJS + " 1.0 " + id + ".js");
            cursor.endEditBlock();

            if (!modified)
                qmldirDoc->document->setModified(false);
        }

        const QString& qmldirLine = "\n" + FormJS + " 1.0 " + id + ".js";
        const QString& qmldirPath = SaveUtils::toProjectAssetsDir(ProjectManager::dir()) + "/qmldir";

        if (!QFileInfo::exists(qmldirPath))
            qFatal("CentralWidget: qmldir file is gone.");

        QFile file(qmldirPath);
        if (!file.open(QFile::WriteOnly | QFile::Append)) {
            qWarning("CentralWidget: Cannot open qmldir file");
            return;
        }
        file.write(qmldirLine.toUtf8());
    });

    connect(m_outputPane->issuesWidget(), &IssuesWidget::designsFileOpened,
            this, [=] (Control* control, const QString& relativePath, int line, int column) {
        m_qmlCodeEditorWidget->openDesigns(control, relativePath);
        m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
    });
    connect(m_outputPane->issuesWidget(), &IssuesWidget::assetsFileOpened,
            this, [=] (const QString& relativePath, int line, int column) {
        m_qmlCodeEditorWidget->openAssets(relativePath);
        m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
    });
    connect(m_outputPane->consoleWidget(), &ConsoleWidget::designsFileOpened,
            this, [=] (Control* control, const QString& relativePath, int line, int column) {
        m_qmlCodeEditorWidget->openDesigns(control, relativePath);
        m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
    });
    connect(m_outputPane->consoleWidget(), &ConsoleWidget::assetsFileOpened,
            this, [=] (const QString& relativePath, int line, int column) {
        m_qmlCodeEditorWidget->openAssets(relativePath);
        m_qmlCodeEditorWidget->codeEditor()->gotoLine(line, column);
    });

    //   BUG connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
    //            m_qmlCodeEditorWidget, &QmlCodeEditorWidget::onControlRemoval);

    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlRenderingManager::scheduleTerminate);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlRenderingManager::scheduleInit);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged, this, [=] {
        if (const Form* currentForm = m_designerPane->designerView()->scene()->currentForm()) {
            Delayer::delay(3000);
            ControlRenderingManager::scheduleRefresh(currentForm->uid());
        }
    });
    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, &CentralWidget::onModeChange);
}

OutputPane* CentralWidget::outputPane() const
{
    return m_outputPane;
}

OutputController* CentralWidget::outputController() const
{
    return m_outputController;
}

DesignerPane* CentralWidget::designerPane() const
{
    return m_designerPane;
}

DesignerController* CentralWidget::designerController() const
{
    return m_designerController;
}

void CentralWidget::discharge()
{
    m_outputController->discharge();
    m_outputController->discharge();
    m_qmlCodeEditorWidget->discharge();
    m_projectOptionsWidget->discharge();
    m_buildsWidget->discharge();
    m_helpWidget->discharge();
}

void CentralWidget::hideWidgets()
{
    m_outputPane->hide();
    m_designerPane->hide();
    g_editorContainer->hide();
    m_projectOptionsWidget->hide();
    m_buildsWidget->hide();
    m_helpWidget->hide();
}

void CentralWidget::onModeChange(ModeManager::Mode mode)
{
    hideWidgets();

    switch (mode) {
    case ModeManager::Designer:
        m_outputPane->show();
        m_designerPane->show();
        break;

    case ModeManager::Editor:
        m_outputPane->show();
        g_editorContainer->show();
        break;

    case ModeManager::Split:
        m_outputPane->show();
        m_designerPane->show();
        g_editorContainer->show();
        break;

    case ModeManager::Options:
        m_projectOptionsWidget->show();
        break;

    case ModeManager::Builds:
        m_buildsWidget->show();
        break;

    case ModeManager::Documents:
        m_helpWidget->show();
        break;

    default:
        break;
    }
}

QmlCodeEditorWidget* CentralWidget::qmlCodeEditorWidget() const
{
    return m_qmlCodeEditorWidget;
}

