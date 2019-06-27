#include <centralwidget.h>
#include <designerwidget.h>
#include <projectoptionswidget.h>
#include <helpwidget.h>
#include <buildswidget.h>
#include <designerscene.h>
#include <controlrenderingmanager.h>
#include <controlremovingmanager.h>
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
#include <controlpropertymanager.h>
#include <outputpane.h>
#include <outputcontroller.h>

#include <QWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QStyle>
#include <QLabel>
#include <QAbstractButton>

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
  , m_outputController(new OutputController(m_outputPane, this))
  , m_qmlCodeEditorWidget(new QmlCodeEditorWidget)
  , m_designerWidget(new DesignerWidget(m_qmlCodeEditorWidget))
  , m_projectOptionsWidget(new ProjectOptionsWidget)
  , m_buildsWidget(new BuildsWidget)
  , m_helpWidget(new HelpWidget)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_splitterOut);

    m_splitterOut->setHandleWidth(0);
    m_splitterOut->setFrameShape(QFrame::NoFrame);
    m_splitterOut->setOrientation(Qt::Vertical);
    m_splitterOut->addWidget(m_splitterIn);
    m_splitterOut->addWidget(m_outputPane);
    m_splitterOut->setChildrenCollapsible(false);
    m_splitterOut->handle(3)->setDisabled(true);

    g_editorContainer = new EditorContainer(this);
    g_editorContainer->setAlignment(Qt::AlignCenter);
    g_editorContainer->setObjectName("g_editorContainer");
    g_editorContainer->setText(tr("Editor window\nraised"));
    g_editorContainer->setStyleSheet("#g_editorContainer { background: transparent; color: #808080;}");
    g_editorContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    g_editorContainer->setLayout(new QHBoxLayout);
    g_editorContainer->layout()->setSpacing(0);
    g_editorContainer->layout()->setContentsMargins(0, 0, 0, 0);
    g_editorContainer->layout()->addWidget(UtilityFunctions::createSeparatorWidget(Qt::Vertical));
    g_editorContainer->layout()->addWidget(m_qmlCodeEditorWidget);

    m_splitterIn->setHandleWidth(0);
    m_splitterIn->setFrameShape(QFrame::NoFrame);
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterIn->addWidget(m_designerWidget);
    m_splitterIn->addWidget(g_editorContainer);
    m_splitterIn->addWidget(m_projectOptionsWidget);
    m_splitterIn->addWidget(m_buildsWidget);
    m_splitterIn->addWidget(m_helpWidget);
    m_splitterIn->setChildrenCollapsible(false);

    onModeChange(ModeManager::mode());

    connect(ControlPropertyManager::instance(), &ControlPropertyManager::imageChanged,
            this, [=] (Control* control, int codeChanged) {
        if (codeChanged)
            m_issuesWidget->refresh(control);
    });
    connect(m_issuesWidget, &IssuesWidget::titleChanged,
            m_outputBar->issuesButton(), &QAbstractButton::setText);
    connect(m_issuesWidget, &IssuesWidget::designsFileOpened,
            m_designerWidget, &DesignerWidget::onDesignsFileOpen);
    connect(m_issuesWidget, &IssuesWidget::assetsFileOpened,
            m_designerWidget, &DesignerWidget::onAssetsFileOpen);
    connect(m_consoleWidget, &ConsoleWidget::designsFileOpened,
            m_designerWidget, &DesignerWidget::onDesignsFileOpen);
    connect(m_consoleWidget, &ConsoleWidget::assetsFileOpened,
            m_designerWidget, &DesignerWidget::onAssetsFileOpen);
    connect(m_issuesWidget, &IssuesWidget::flash,
            this, [=] {
        // FIXME       void OutputBar::flash(QAbstractButton* button)
        //        {
        //            if (button == m_consoleButton)
        //                m_consoleFlasher->flash(400, 3);
        //            else if (button == m_issuesButton)
        //                m_issuesFlasher->flash(400, 3);

        //            const InterfaceSettings* settings = GeneralSettings::interfaceSettings();
        //            if (settings->bottomPanesPop && !button->isChecked())
        //                button->click();
        //        }
        m_outputBar->flash(m_outputBar->issuesButton());
    });
    connect(m_consoleWidget, &ConsoleWidget::flash,
            this, [=] {
        m_outputBar->flash(m_outputBar->consoleButton());
    });
    connect(m_issuesWidget, &IssuesWidget::minimized,
            this, [=] {
        m_outputBar->issuesButton()->setChecked(false);
        m_issuesWidget->hide();
        emit bottomPaneTriggered(false);
    });
    connect(m_consoleWidget, &ConsoleWidget::minimized,
            this, [=] {
        m_outputBar->consoleButton()->setChecked(false);
        m_consoleWidget->hide();
        emit bottomPaneTriggered(false);
    });
    connect(m_outputBar, &OutputBar::buttonActivated,
            this, [=] (QAbstractButton* button) {
        if (button == m_outputBar->consoleButton()) {
            if (button->isChecked())
                m_consoleWidget->show();
            else
                m_consoleWidget->hide();
        } else {
            if (button->isChecked())
                m_issuesWidget->show();
            else
                m_issuesWidget->hide();
        }
        emit bottomPaneTriggered(m_issuesWidget->isVisible() || m_consoleWidget->isVisible());
    });

    m_qmlCodeEditorWidget->addSaveFilter(new ControlSaveFilter(this)); // Changes made in code editor
    connect(SaveManager::instance(), &SaveManager::propertyChanged,    // Changes made out of code editor
            this, [=] (Control* control, const QString& property, const QString& value) {
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

    //   BUG connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
    //            m_qmlCodeEditorWidget, &QmlCodeEditorWidget::onControlRemoval);

    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlRenderingManager::scheduleTerminate);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlRenderingManager::scheduleInit);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged, this, [=] {
        Delayer::delay(3000);
        m_designerWidget->refresh();
    });
    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, &CentralWidget::onModeChange);
}

DesignerWidget* CentralWidget::designerWidget() const
{
    return m_designerWidget;
}

OutputPane* CentralWidget::outputPane() const
{
    return m_outputPane;
}

void CentralWidget::discharge()
{
    m_consoleWidget->hide();
    m_issuesWidget->hide();
    // FIXME   m_outputBar->discharge();
    m_consoleWidget->discharge();
    m_issuesWidget->discharge();
    m_qmlCodeEditorWidget->discharge();
    m_designerWidget->discharge();
    m_projectOptionsWidget->discharge();
    m_buildsWidget->discharge();
    m_helpWidget->discharge();
}

void CentralWidget::setBottomPaneVisible(bool visible)
{
    if (m_outputBar->activeButton() && visible)
        return;
    if (!m_outputBar->activeButton() && !visible)
        return;
    if (m_outputBar->activeButton()) {
        m_outputBar->activeButton()->setChecked(false);
        if (m_outputBar->activeButton() == m_outputBar->issuesButton())
            m_issuesWidget->hide();
        else
            m_consoleWidget->hide();
    } else {
        m_outputBar->consoleButton()->setChecked(true);
        m_consoleWidget->show();
    }
}

void CentralWidget::hideWidgets()
{
    m_outputBar->hide();
    m_issuesWidget->hide();
    m_consoleWidget->hide();
    m_designerWidget->hide();
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
        m_outputBar->show();
        if (m_outputBar->activeButton() == m_outputBar->consoleButton())
            m_consoleWidget->show();
        if (m_outputBar->activeButton() == m_outputBar->issuesButton())
            m_issuesWidget->show();
        m_designerWidget->show();
        break;

    case ModeManager::Editor:
        m_outputBar->show();
        if (m_outputBar->activeButton() == m_outputBar->consoleButton())
            m_consoleWidget->show();
        if (m_outputBar->activeButton() == m_outputBar->issuesButton())
            m_issuesWidget->show();
        g_editorContainer->show();
        break;

    case ModeManager::Split:
        m_outputBar->show();
        if (m_outputBar->activeButton() == m_outputBar->consoleButton())
            m_consoleWidget->show();
        if (m_outputBar->activeButton() == m_outputBar->issuesButton())
            m_issuesWidget->show();
        m_designerWidget->show();
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

