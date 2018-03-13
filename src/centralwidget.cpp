#include <centralwidget.h>
#include <outputwidget.h>
#include <qmlcodeeditorwidget.h>
#include <designerwidget.h>
#include <projectsettingswidget.h>
#include <documentationswidget.h>
#include <buildswidget.h>

#include <QSplitter>
#include <QVBoxLayout>

CentralWidget::CentralWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_splitterOut(new QSplitter)
  , m_splitterIn(new QSplitter)
  , m_outputWidget(new OutputWidget)
  , m_qmlCodeEditorWidget(new QmlCodeEditorWidget)
  , m_designerWidget(new DesignerWidget(m_qmlCodeEditorWidget))
  , m_projectSettingsWidget(new ProjectSettingsWidget)
  , m_buildsWidget(new BuildsWidget)
  , m_documentationsWidget(new DocumentationsWidget)
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_splitterOut);

    m_splitterOut->addWidget(m_splitterIn);
    m_splitterOut->addWidget(m_outputWidget);

    m_splitterIn->addWidget(m_qmlCodeEditorWidget);
    m_splitterIn->addWidget(m_designerWidget);
    m_splitterIn->addWidget(m_projectSettingsWidget);
    m_splitterIn->addWidget(m_buildsWidget);
    m_splitterIn->addWidget(m_documentationsWidget);
}

DesignerWidget* CentralWidget::designerWidget() const
{
    return m_designerWidget;
}

void CentralWidget::reset()
{
    m_splitterIn->setOrientation(Qt::Horizontal);
    m_splitterOut->setOrientation(Qt::Vertical);

    m_qmlCodeEditorWidget->hide();
    m_projectSettingsWidget->hide();
    m_buildsWidget->hide();
    m_documentationsWidget->hide();

    m_outputWidget->reset();
    m_qmlCodeEditorWidget->reset();
    m_designerWidget->reset();
    m_projectSettingsWidget->reset();
    m_buildsWidget->reset();
    m_documentationsWidget->reset();

//    m_centralWidget->qmlCodeEditorWidget()->clear();
//    m_centralWidget->designerWidget()->designerScene()->clearSelection();
//    m_centralWidget->designerWidget()->designerScene()->clearScene();
}

QmlCodeEditorWidget* CentralWidget::qmlCodeEditorWidget() const
{
    return m_qmlCodeEditorWidget;
}

