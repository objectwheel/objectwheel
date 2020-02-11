#include <centralwidget.h>
#include <designerview.h>
#include <projectoptionswidget.h>
#include <helpwidget.h>
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
#include <buildspane.h>
#include <buildscontroller.h>
#include <issueswidget.h>
#include <consolewidget.h>
#include <form.h>
#include <themechooserwidget.h>

#include <QBoxLayout>
#include <QLabel>

static QString methodName(const QString& signal)
{
    QString method(signal);
    method.replace(0, 1, method[0].toUpper());
    return method.prepend("on");
}

static bool warnIfFileDoesNotExist(const QString& filePath)
{
    if (!QFileInfo::exists(filePath)) {
        return UtilityFunctions::showMessage(
                    nullptr, QObject::tr("Oops"),
                    QObject::tr("File %1 does not exist.").arg(QFileInfo(filePath).fileName()));
    }
    return false;
}

class EditorContainer final : public QLabel {
public:
    explicit EditorContainer(QWidget* parent = nullptr) : QLabel(parent) {}
    QSize sizeHint() const override { return {670, 640}; }
};

CentralWidget::CentralWidget(QWidget* parent) : QSplitter(parent)
  , m_splitterIn(new QSplitter)
  , m_outputPane(new OutputPane)
  , m_outputController(new OutputController(m_outputPane, this))
  , m_designerPane(new DesignerPane)
  , m_designerController(new DesignerController(m_designerPane, this))
  , m_buildsPane(new BuildsPane)
  , m_buildsController(new BuildsController(m_buildsPane, this))
  , m_codeEditorContainer(new EditorContainer)
  , m_qmlCodeEditorWidget(new QmlCodeEditorWidget)
  , m_projectOptionsWidget(new ProjectOptionsWidget)
  , m_helpWidget(new HelpWidget)
{
    setFrameShape(QFrame::NoFrame);
    setOrientation(Qt::Vertical);
    addWidget(m_splitterIn);
    addWidget(m_outputPane);
    setChildrenCollapsible(false);

    auto containerLayout = new QHBoxLayout(m_codeEditorContainer);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(m_qmlCodeEditorWidget);

    QFont smallFont = font();
    smallFont.setPixelSize(smallFont.pixelSize() - 1);
    m_codeEditorContainer->setFont(smallFont);
    m_codeEditorContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_codeEditorContainer->setAlignment(Qt::AlignCenter);
    m_codeEditorContainer->setText(QStringLiteral("<span style='color:#777777'>%1</span>")
                                   .arg(tr("Editor window<br>raised")));

    m_splitterIn->setFrameShape(QFrame::NoFrame);
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterIn->addWidget(m_designerPane);
    m_splitterIn->addWidget(m_codeEditorContainer);
    m_splitterIn->addWidget(m_projectOptionsWidget);
    m_splitterIn->addWidget(m_buildsPane);
    m_splitterIn->addWidget(m_helpWidget);
    m_splitterIn->setChildrenCollapsible(false);
    m_splitterIn->setHandleWidth(1);

    UtilityFunctions::setFocusRing(m_designerPane, true);
    UtilityFunctions::setFocusRing(m_qmlCodeEditorWidget->codeEditor(), true);

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

    connect(m_designerController, &DesignerController::viewSourceCodeTriggered,
            this, [=] (Control* control) {
        m_qmlCodeEditorWidget->openDesigns(control, SaveUtils::controlMainQmlFileName());
    });
    connect(m_designerController, &DesignerController::goToSlotTriggered,
            this, [=] (Control* control, const QString& signal) {
        // 1. Control name; 2. Method name
        #define METHOD_DECL "%1_%2"
        #define FORM_DECL "%1_onCompleted"
        #define METHOD_BODY \
            "\n\n"\
            "function %1() {\n"\
            "    \n"\
            "}"

        DesignerScene* scene = designerPane()->designerView()->scene();
        const QString& methodSign = QString::fromUtf8(METHOD_DECL)
                .arg(control->id())
                .arg(methodName(signal));
        const QString& methodBody = QString::fromUtf8(METHOD_BODY).arg(methodSign);
        const QString& formSign = scene->currentForm()->id();
        const QString& formJS = formSign + ".js";
        const QString& fullPath = SaveUtils::toProjectAssetsDir(ProjectManager::dir()) + '/' + formJS;

        if (warnIfFileDoesNotExist(fullPath))
            return;

        qmlCodeEditorWidget()->openAssets(formJS);

        QmlCodeEditorWidget::AssetsDocument* document = qmlCodeEditorWidget()->getAssets(formJS);
        Q_ASSERT(document);

        int pos = ParserUtils::methodLine(document->document, fullPath, methodSign);
        if (pos < 0) {
            const QString& connection =
                    control->id() + "." + signal + ".connect(" + methodSign + ")";
            const QString& loaderSign = QString::fromUtf8(FORM_DECL).arg(formSign);
            ParserUtils::addConnection(document->document, fullPath, loaderSign, connection);
            pos = ParserUtils::addMethod(document->document, fullPath, methodBody);
            pos += 3;
        }

        qmlCodeEditorWidget()->codeEditor()->gotoLine(pos);
    });
    connect(m_designerController, &DesignerController::projectThemeActivated,
            m_projectOptionsWidget->themeChooserWidget(), &ThemeChooserWidget::setCurrentStyle);
    connect(m_designerController, &DesignerController::projectThemeActivated1,
            m_projectOptionsWidget->themeChooserWidget1(), &ThemeChooserWidget::setCurrentStyle);

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
            ControlRenderingManager::terminate);
    connect(m_projectOptionsWidget, &ProjectOptionsWidget::themeChanged,
            ControlRenderingManager::start);

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

BuildsPane* CentralWidget::buildsPane() const
{
    return m_buildsPane;
}

BuildsController* CentralWidget::buildsController() const
{
    return m_buildsController;
}

DesignerPane* CentralWidget::designerPane() const
{
    return m_designerPane;
}

DesignerController* CentralWidget::designerController() const
{
    return m_designerController;
}

void CentralWidget::charge()
{
    m_designerController->charge();
    m_projectOptionsWidget->charge();
    m_buildsPane->charge();
}

void CentralWidget::discharge()
{
    m_outputController->discharge();
    m_designerController->discharge();
    m_qmlCodeEditorWidget->discharge();
    m_buildsPane->discharge();
    m_helpWidget->discharge();
}

void CentralWidget::hideWidgets()
{
    m_outputPane->hide();
    m_designerPane->hide();
    m_codeEditorContainer->hide();
    m_projectOptionsWidget->hide();
    m_buildsPane->hide();
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
        m_codeEditorContainer->show();
        break;

    case ModeManager::Split:
        m_outputPane->show();
        m_designerPane->show();
        m_codeEditorContainer->show();
        break;

    case ModeManager::Options:
        m_projectOptionsWidget->show();
        break;

    case ModeManager::Builds:
        m_buildsPane->show();
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

