#ifndef PROJECTDETAILSWIDGET_H
#define PROJECTDETAILSWIDGET_H

#include <QWidget>

class QLabel;
class BulkEdit;
class ButtonSlice;
class QVBoxLayout;

class ProjectDetailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectDetailsWidget(QWidget* parent = nullptr);

public slots:
    void onEditProject(const QString& hash);
    void onNewProject(const QString& projectName, int templateNumber);

private slots:
    void onSaveClick();
    void onDeleteClick();

signals:
    void back();
    void done();

private:
    QString m_hash;
    QVBoxLayout* m_layout;
    QLabel* m_iconLabel;
    QLabel* m_settingsLabel;
    BulkEdit* m_bulkEdit;
    ButtonSlice* m_buttons;
    bool m_toTemplates;
};

#endif // PROJECTDETAILSWIDGET_H