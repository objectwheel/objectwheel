#include <centralwidget.h>
#include <designerwidget.h>
#include <projectoptionswidget.h>
#include <helpwidget.h>
#include <buildswidget.h>
#include <issuespane.h>
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
#include <qmlcodeeditor.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <bottombar.h>
#include <transparentstyle.h>
#include <consolepane.h>
#include <controlpropertymanager.h>

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
  , m_bottomBar(new BottomBar)
  , m_consolePane(new ConsolePane)
  , m_issuesPane(new IssuesPane)
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
    m_splitterOut->addWidget(m_consolePane);
    m_splitterOut->addWidget(m_issuesPane);
    m_splitterOut->addWidget(m_bottomBar);
    m_splitterOut->setChildrenCollapsible(false);

    m_splitterOut->handle(3)->setDisabled(true);

    m_bottomBar->setFixedHeight(24);

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
    m_splitterIn->setStyleSheet("QSplitter, QSplitter::handle { border: none; }");
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterIn->addWidget(m_designerWidget);
    m_splitterIn->addWidget(g_editorContainer);
    m_splitterIn->addWidget(m_projectOptionsWidget);
    m_splitterIn->addWidget(m_buildsWidget);
    m_splitterIn->addWidget(m_helpWidget);
    m_splitterIn->setChildrenCollapsible(false);

    connect(ControlPropertyManager::instance(), &ControlPropertyManager::previewChanged,
            this, [=] (Control* control, int codeChanged) {
        if (codeChanged)
            m_issuesPane->refresh(control);
    });
    connect(m_issuesPane, &IssuesPane::titleChanged,
            m_bottomBar->issuesButton(), &QAbstractButton::setText);
    connect(m_issuesPane, &IssuesPane::internalFileOpened,
            m_designerWidget, &DesignerWidget::onInternalFileOpen);
    connect(m_issuesPane, &IssuesPane::globalFileOpened,
            m_designerWidget, &DesignerWidget::onGlobalFileOpen);
    connect(m_consolePane, &ConsolePane::internalFileOpened,
            m_designerWidget, &DesignerWidget::onInternalFileOpen);
    connect(m_consolePane, &ConsolePane::globalFileOpened,
            m_designerWidget, &DesignerWidget::onGlobalFileOpen);
    connect(m_issuesPane, &IssuesPane::flash,
            this, [=] {
        m_bottomBar->flash(m_bottomBar->issuesButton());
    });
    connect(m_consolePane, &ConsolePane::flash,
            this, [=] {
        m_bottomBar->flash(m_bottomBar->consoleButton());
    });
    connect(m_issuesPane, &IssuesPane::minimized,
            this, [=] {
        m_bottomBar->issuesButton()->setChecked(false);
        m_issuesPane->hide();
    });
    connect(m_consolePane, &ConsolePane::minimized,
            this, [=] {
        m_bottomBar->consoleButton()->setChecked(false);
        m_consolePane->hide();
    });
    connect(m_bottomBar, &BottomBar::buttonActivated,
            this, [=] (QAbstractButton* button) {
        if (button == m_bottomBar->consoleButton()) {
            if (button->isChecked())
                m_consolePane->show();
            else
                m_consolePane->hide();
        } else {
            if (button->isChecked())
                m_issuesPane->show();
            else
                m_issuesPane->hide();
        }
    });

    m_qmlCodeEditorWidget->addSaveFilter(new ControlSaveFilter(this)); // Changes made in code editor
    connect(SaveManager::instance(), &SaveManager::propertyChanged,    // Changes made out of code editor
            this, [=] (Control* control, const QString& property, const QString& value) {
        QmlCodeEditorWidget::InternalDocument* document = qmlCodeEditorWidget()->getInternal(control, SaveUtils::mainQmlFileName());
        if (document)
            ParserUtils::setProperty(document->document, control->url(), property, value);
    });
    connect(SaveManager::instance(), &SaveManager::formGlobalConnectionsDone,
            this, [=] (const QString& FormJS, const QString& id) {
        qmlCodeEditorWidget()->openGlobal(id + ".js");
        qmlCodeEditorWidget()->codeEditor()->gotoLine(4);

        QmlCodeEditorWidget::GlobalDocument* qmldirDoc = qmlCodeEditorWidget()->getGlobal("qmldir");
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
        const QString& qmldirPath = SaveUtils::toGlobalDir(ProjectManager::dir()) + '/' + "qmldir";

        if (!QFileInfo::exists(qmldirPath))
            qFatal("CentralWidget: qmldir file is gone.");

        QByteArray qmldirFile = rdfile(qmldirPath);
        qmldirFile.append(qmldirLine);
        wrfile(qmldirPath, qmldirFile);
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

IssuesPane* CentralWidget::issuesPane() const
{
    return m_issuesPane;
}

BottomBar* CentralWidget::bottomBar() const
{
    return m_bottomBar;
}

ConsolePane* CentralWidget::consolePane() const
{
    return m_consolePane;
}

void CentralWidget::discharge()
{
    setCurrentPage(Page_Designer);
    m_consolePane->hide();
    m_issuesPane->hide();
    m_bottomBar->discharge();
    m_consolePane->discharge();
    m_issuesPane->discharge();
    m_qmlCodeEditorWidget->discharge();
    m_designerWidget->discharge();
    m_projectOptionsWidget->discharge();
    m_buildsWidget->discharge();
    m_helpWidget->discharge();
}

void CentralWidget::setCurrentPage(const Pages& page)
{
    hideWidgets();

    switch (page) {
    case Page_Builds:
        return m_buildsWidget->show();

    case Page_Designer:
        m_bottomBar->show();
        if (m_bottomBar->activeButton() == m_bottomBar->consoleButton())
            m_consolePane->show();
        if (m_bottomBar->activeButton() == m_bottomBar->issuesButton())
            m_issuesPane->show();
        return m_designerWidget->show();

    case Page_SplitView:
        m_bottomBar->show();
        if (m_bottomBar->activeButton() == m_bottomBar->consoleButton())
            m_consolePane->show();
        if (m_bottomBar->activeButton() == m_bottomBar->issuesButton())
            m_issuesPane->show();
        m_designerWidget->show();
        return g_editorContainer->show();

    case Page_Help:
        return m_helpWidget->show();

    case Page_QmlCodeEditor:
        m_bottomBar->show();
        if (m_bottomBar->activeButton() == m_bottomBar->consoleButton())
            m_consolePane->show();
        if (m_bottomBar->activeButton() == m_bottomBar->issuesButton())
            m_issuesPane->show();
        return g_editorContainer->show();

    case Page_ProjectOptions:
        return m_projectOptionsWidget->show();
    }
}

void CentralWidget::hideWidgets()
{
    m_bottomBar->hide();
    m_issuesPane->hide();
    m_consolePane->hide();
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

