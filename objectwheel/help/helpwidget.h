#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>
#include <QFutureWatcher>

class QSplitter;
class QComboBox;
class QHelpEngine;
class QVBoxLayout;
class LineEdit;
class QToolBar;
class QToolButton;
class QLabel;

namespace Help { namespace Internal { class TextBrowserHelpViewer; }}

class HelpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelpWidget(QWidget *parent = nullptr);

public slots:
    void discharge();

protected:
    QSize sizeHint() const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onHomeButtonClick();
    void onTypeChange();
    void onTitleChange();
    void onIndexFilterTextEdit(const QString& filterText);
    void onUrlChange(const QUrl& url);
    void onUrlChange(const QUrl& link, const QString&);
    void onUrlChange(const QMap<QString, QUrl> &links, const QString &keyword);

private:
    QVBoxLayout* m_layout;
    QToolBar* m_toolBar;
    QComboBox* m_typeCombo;
    QToolButton* m_homeButton;
    QToolButton* m_backButton;
    QToolButton* m_forthButton;
    QLabel* m_titleLabel;
    QAction* m_copyAction;
    QSplitter* m_splitter;
    Help::Internal::TextBrowserHelpViewer* m_helpViewer;
    QWidget* m_contentsWidget;
    QVBoxLayout* m_contentsLayout;
    QWidget* m_indexWidget;
    QVBoxLayout* m_indexLayout;
    LineEdit* m_indexFilterEdit;
};

#endif // HELPWIDGET_H