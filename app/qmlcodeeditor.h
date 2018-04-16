#ifndef QMLCODEEDITOR_H
#define QMLCODEEDITOR_H

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

    void addErrorLine(int line);
    void clearErrorLines();

public slots:
    void reset();
    void updateCompletion();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    QSize sizeHint() const override;

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
    QCompleter _completer;
    QStandardItemModel _model;
    CompletionHelper _completionHelper;
    QThread _completionThread;
    QTimer _completionTimer;
    QList<int> _errorLines;
};


#endif // QMLCODEEDITOR_H
