#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QSplitter>

class QComboBox;
class LineEdit;
class QToolButton;
class QLabel;
namespace Help { namespace Internal { class TextBrowserHelpViewer; }}

class HelpWidget final : public QSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(HelpWidget)

public:
    enum Mode {
        Invalid,
        ContentList,
        IndexList
    };

public:
    explicit HelpWidget(QWidget* parent = nullptr);

    Mode mode() const;
    void setMode(Mode mode);

public slots:
    void discharge();

protected:
    QSize sizeHint() const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onHelpModeComboBoxActivation();
    void onHomeButtonClick();
    void onTitleChange();
    void onIndexFilterTextEdit(const QString& filterText);
    void onUrlChange(const QUrl& url);
    void onUrlChange(const QUrl& link, const QString&);
    void onUrlChange(const QMap<QString, QUrl> &links, const QString &keyword);

private:
    Mode m_mode;
    QComboBox* m_helpModeComboBox;
    LineEdit* m_indexListFilterEdit;
    QLabel* m_titleLabel;
    QToolButton* m_homeButton;
    QToolButton* m_backButton;
    QToolButton* m_forthButton;
    Help::Internal::TextBrowserHelpViewer* m_helpViewer;
};

#endif // HELPWIDGET_H