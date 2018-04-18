#ifndef PROJECTTEMPLATESSWIDGET_H
#define PROJECTTEMPLATESSWIDGET_H

#include <QWidget>

class QLabel;
class ButtonSlice;
class QVBoxLayout;
class QListWidget;

class ProjectTemplatesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectTemplatesWidget(QWidget* parent = nullptr);

private slots:
    void onNextButtonClick();
    void onNewProject(const QString& projectName);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void back();
    void newProject(const QString& projectName, int templateNumber);

private:
    QVBoxLayout* m_layout;
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_informativeLabel;
    QLabel* m_templatesLabel;
    QListWidget* m_listWidget;
    ButtonSlice* m_buttons;
    QString m_projectName;
};

#endif // PROJECTTEMPLATESSWIDGET_H