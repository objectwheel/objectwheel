#ifndef QMLCODEEDITOR_H
#define QMLCODEEDITOR_H

#include <qmlhighlighter.h>
#include <QPlainTextEdit>
#include <QThread>
#include <QTimer>

#include <completionhelper.h>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class LineNumberArea;
class QCompleter;

class QmlCodeEditor : public QPlainTextEdit
{
        Q_OBJECT

    public:
        QmlCodeEditor(QWidget* parent = 0);
        ~QmlCodeEditor();

        void lineNumberAreaPaintEvent(QPaintEvent* event);
        int lineNumberAreaWidth();

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* e) override;
        void focusInEvent(QFocusEvent* e) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect& , int);
        void insertCompletion(const QString& completion);
        void handleExtractionResult(const ExtractionResult& result);

    private:
        QString textUnderCursor() const;

    private:
        QWidget* lineNumberArea;
        QmlHighlighter highlighter;
        QCompleter _completer;
        QStandardItemModel _model;
        CompletionHelper _completionHelper;
        QThread _completionThread;
        QTimer _completionTimer;
};


#endif // QMLCODEEDITOR_H
