#ifndef QMLCODEEDITOR_H
#define QMLCODEEDITOR_H

#include <qmlhighlighter.h>
#include <QPlainTextEdit>
#include <QObject>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class LineNumberArea;

class QmlCodeEditor : public QPlainTextEdit
{
        Q_OBJECT

    public:
        QmlCodeEditor(QWidget *parent = 0);

        void lineNumberAreaPaintEvent(QPaintEvent *event);
        int lineNumberAreaWidth();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &, int);

    private:
        QWidget *lineNumberArea;
        QmlHighlighter highlighter;
};


#endif // QMLCODEEDITOR_H
