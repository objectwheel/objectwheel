#ifndef CONSOLEBOX_H
#define CONSOLEBOX_H

#include <QWidget>

class QTextEdit;
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
        void clear();
        void scrollToEnd();

    private slots:
        void onStandardError(const QString& output);
        void onStandardOutput(const QString& output);

    private:
        QVBoxLayout* _layout;
        QTextEdit* _textEdit;
        OutputPane* _outputPane;
};

#endif // CONSOLEBOX_H