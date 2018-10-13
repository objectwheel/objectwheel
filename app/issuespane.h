#ifndef ISSUESPANE_H
#define ISSUESPANE_H

#include <QListWidget>
#include <QQmlError>

class QToolBar;
class QLabel;
class QToolButton;
class Control;

class IssuesPane : public QListWidget
{
    Q_OBJECT

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

#endif // ISSUESPANE_H
