#ifndef CONSOLEPANE_H
#define CONSOLEPANE_H

#include <QPlainTextEdit>

class QToolButton;
class QToolBar;
class QLabel;
class Control;

class ConsolePane : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ConsolePane(QWidget* widget = nullptr);

public:
    bool isClean() const;
    void print(const QString& text);
    void printError(const QString& text);
    void printFormatted(const QString& text, const QColor& color, QFont::Weight weight);

public slots:
    void fade();
    void sweep();

private slots:
    void onLinkClick(const QString& link);
    void onStandardOutput(const QString& output);
    void onStandardErrorOutput(const QString& output);

protected:
    void updateViewportMargins();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    bool eventFilter(QObject*, QEvent*) override;
    void resizeEvent(QResizeEvent* e) override;

signals:
    void flash();
    void minimized();
    void globalFileOpened(const QString& relativePath, int line, int column);
    void internalFileOpened(Control* control, const QString& relativePath, int line, int column);

private:
    QToolBar* m_toolBar;
    QLabel* m_titleLabel;
    QToolButton* m_clearButton;
    QToolButton* m_fontSizeUpButton;
    QToolButton* m_fontSizeDownButton;
    QToolButton* m_minimizeButton;
};

#endif // CONSOLEPANE_H