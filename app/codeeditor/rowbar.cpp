#include <rowbar.h>
#include <bracketband.h>
#include <breakpointband.h>
#include <linenumberband.h>
#include <markband.h>
#include <qmlcodedocument.h>
#include <qmlcodeeditor.h>
#include <texteditor/texteditorconstants.h>
#include <codeeditorsettings.h>
#include <fontcolorssettings.h>

#include <QPainter>
#include <QHBoxLayout>

RowBar::RowBar(QmlCodeEditor* editor, QWidget* parent) : QWidget(parent)
  , m_qmlCodeEditor(editor)
  , m_layout(new QHBoxLayout(this))
  , m_breakpointBand(new BreakpointBand(editor))
  , m_markBand(new MarkBand(editor))
  , m_linenumberBand(new LineNumberBand(editor))
  , m_bracketBand(new BracketBand(editor))
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_breakpointBand);
    m_layout->addWidget(m_linenumberBand);
    m_layout->addWidget(m_markBand);
    m_layout->addWidget(m_bracketBand);

    connect(m_qmlCodeEditor, SIGNAL(updateRequest(QRect,int)), this, SLOT(update()));
    connect(CodeEditorSettings::instance(), &CodeEditorSettings::fontColorsSettingsChanged,
            this, qOverload<>(&RowBar::update));
}

int RowBar::calculatedWidth() const
{
    return m_breakpointBand->calculatedWidth() + m_markBand->calculatedWidth()
            + m_linenumberBand->calculatedWidth() + m_bracketBand->calculatedWidth();
}

QSize RowBar::sizeHint() const
{
    return QSize(calculatedWidth(), 0);
}

void RowBar::mouseReleaseEvent(QMouseEvent* e)
{
    auto cursor = m_qmlCodeEditor->cursorForPosition({0, e->pos().y()});
    m_qmlCodeEditor->setTextCursor(cursor);
}

BreakpointBand* RowBar::breakpointBand() const
{
    return m_breakpointBand;
}

void RowBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(),
               CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_LINE_NUMBER).background());
}

MarkBand* RowBar::markBand() const
{
    return m_markBand;
}

LineNumberBand* RowBar::linenumberBand() const
{
    return m_linenumberBand;
}

BracketBand* RowBar::bracketBand() const
{
    return m_bracketBand;
}
