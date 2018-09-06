#include <qmlcodeeditorwidget.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <qmlcodeeditortoolbar.h>
#include <fileexplorer.h>
#include <qmlcodedocument.h>

#include <QToolBar>
#include <QToolButton>
#include <QSplitter>
#include <QLayout>

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget *parent) : QWidget(parent)
  , m_splitter(new QSplitter(this))
  , m_codeEditor(new QmlCodeEditor(this))
  , m_fileExplorer(new FileExplorer(0))
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_splitter);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter->addWidget(m_codeEditor);
    m_splitter->addWidget(m_fileExplorer);
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
    m_splitter->setHandleWidth(0);

    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::saved,
            this, &QmlCodeEditorWidget::save);
    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::closed,
            this, &QmlCodeEditorWidget::close);
    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::showed,
            this, &QmlCodeEditorWidget::setExplorerVisible);
    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::pinned,
            this, &QmlCodeEditorWidget::pinned);
}

void QmlCodeEditorWidget::sweep()
{
    m_splitter->setStretchFactor(0, 30);
    m_splitter->setStretchFactor(1, 9);

    m_codeEditor->sweep();
    m_fileExplorer->sweep();

    m_fileExplorer->setRootPath("/Users/omergoktas/Desktop");
}

void QmlCodeEditorWidget::save()
{

}

void QmlCodeEditorWidget::close()
{

}

void QmlCodeEditorWidget::setExplorerVisible(bool visible)
{
    m_splitter->handle(1)->setDisabled(!visible);
    m_fileExplorer->setHidden(!visible);
}
