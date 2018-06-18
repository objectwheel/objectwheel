#ifndef CONSOLEBOX_H
#define CONSOLEBOX_H

#include <QWidget>

class QTextBrowser;
class QVBoxLayout;
class OutputPane;

class ConsoleBox : public QWidget
{
    Q_OBJECT

public:
    explicit ConsoleBox(OutputPane* outputPane);

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

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QVBoxLayout* m_layout;
    QTextBrowser* m_textBrowser;
    OutputPane* m_outputPane;
};

#endif // CONSOLEBOX_H