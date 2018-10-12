#ifndef ISSUESPANE_H
#define ISSUESPANE_H

#include <QListWidget>
#include <QList>
#include <QQmlError>

class QVBoxLayout;
class QToolBar;
class QLabel;
class QToolButton;
class Control;

class IssuesPane : public QListWidget
{
    Q_OBJECT

    friend class IssuesListDelegate;
    template <typename T> friend struct QMetaTypeId;

    struct ControlErrors {
        Control* control;
        QList<QQmlError> errors;
    };

public:
    explicit IssuesPane(QWidget* parent = nullptr);

public slots:
    void sweep();
    void update(Control* control);

private slots:
    void onItemDoubleClick(QListWidgetItem* item);
    void onControlDestruction(QObject* controlObject);

protected:
    void updateGeometries() override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void flash();
    void minimized();
    void controlDoubleClicked(Control*);
    void titleChanged(const QString& title);

private:
    QToolBar* m_toolBar;
    QLabel* m_titleLabel;
    QToolButton* m_clearButton;
    QToolButton* m_fontSizeUpButton;
    QToolButton* m_fontSizeDownButton;
    QToolButton* m_minimizeButton;
    QList<ControlErrors*> m_erroneousControls;
};

Q_DECLARE_METATYPE(const IssuesPane::ControlErrors*)

inline bool operator==(const QQmlError& e1, const QQmlError& e2)
{
    return e1.column() == e2.column()
            && e1.description() == e2.description()
            && e1.line() == e2.line()
            && e1.messageType() == e2.messageType()
            && e1.object() == e2.object()
            && e1.url() == e2.url();
}

#endif // ISSUESPANE_H
