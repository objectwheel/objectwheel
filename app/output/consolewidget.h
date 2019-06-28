#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QApplication>
#include <QPlainTextEdit>
#include <pathfinder.h>

class QToolButton;
class QToolBar;
class QLabel;
class Control;

class ConsoleWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ConsoleWidget(QWidget* widget = nullptr);

public slots:
    void fade();
    void press(const QString& text, const QBrush& brush = QApplication::palette().text(),
               QFont::Weight weight = QFont::Normal);

private slots:
    void onLinkClick(const PathFinder::Result& result);

protected:
    bool eventFilter(QObject*, QEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    void updateViewportMargins();

signals:
    void flash();
    void minimized();
    void assetsFileOpened(const QString& relativePath, int line, int column);
    void designsFileOpened(Control* control, const QString& relativePath, int line, int column);

private:
    QToolBar* m_toolBar;
    QLabel* m_titleLabel;
    QToolButton* m_clearButton;
    QToolButton* m_fontSizeUpButton;
    QToolButton* m_fontSizeDownButton;
    QToolButton* m_minimizeButton;
};

#endif // CONSOLEWIDGET_H