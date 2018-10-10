#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>
#include <QFutureWatcher>

class QSplitter;
class QComboBox;
class QHelpEngine;
class QVBoxLayout;
class FocuslessLineEdit;
class WebEngineHelpViewer;
class QToolBar;
class QToolButton;
class QLabel;

class HelpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelpWidget(QWidget *parent = nullptr);

public slots:
    void sweep();

protected:
    QSize sizeHint() const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onHomeButtonClick();
    void onTypeChange();
    void onTitleChange();
    void onIndexFilterTextChange(const QString& filterText);
    void onUrlChange(const QUrl& url);
    void onUrlChange(const QUrl& link, const QString&);
    void onUrlChange(const QMap<QString, QUrl> &links, const QString &keyword);

private:
    QHelpEngine* m_helpEngine;
    QVBoxLayout* m_layout;
    QToolBar* m_toolBar;
    QComboBox* m_typeCombo;
    QToolButton* m_homeButton;
    QToolButton* m_backButton;
    QToolButton* m_forthButton;
    QLabel* m_titleLabel;
    QAction* m_copyAction;
    QSplitter* m_splitter;
    WebEngineHelpViewer* m_helpViewer;
    QWidget* m_contentsWidget;
    QVBoxLayout* m_contentsLayout;
    QWidget* m_indexWidget;
    QVBoxLayout* m_indexLayout;
    FocuslessLineEdit* m_indexFilterEdit;
};

#endif // HELPWIDGET_H