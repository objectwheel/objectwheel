#ifndef CONSOLEBOX_H
#define CONSOLEBOX_H

#include <QWidget>

class PlainTextEdit;
class QVBoxLayout;
class QToolButton;
class QToolBar;
class QLabel;

class ConsoleBox : public QWidget
{
    Q_OBJECT

public:
    explicit ConsoleBox(QWidget* widget = nullptr);

public:
    bool isClean() const;
    void print(const QString& text);
    void printError(const QString& text);
    void printFormatted(const QString& text, const QColor& color, QFont::Weight weight);

public slots:
    void fade();
    void sweep();
    void scrollToEnd();

private slots:
    void onLinkClick(const QString& link);
    void onStandardError(const QString& output);
    void onStandardOutput(const QString& output);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void flash();
    void minimized();

private:
    QVBoxLayout* m_layout;
    PlainTextEdit* m_plainTextEdit;
    QToolBar* m_toolBar;
    QLabel* m_titleLabel;
    QToolButton* m_clearButton;
    QToolButton* m_fontSizeUpButton;
    QToolButton* m_fontSizeDownButton;
    QToolButton* m_minimizeButton;
};

#endif // CONSOLEBOX_H