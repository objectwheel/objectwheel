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
        void onNewProject(const QString& projectName);

    private slots:
        void onSaveClick();
        void onDeleteClick();

    signals:
        void done();

    private:
        QString _hash;
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _settingsLabel;
        BulkEdit* _bulkEdit;
        ButtonSlice* _buttons;
};

#endif // PROJECTDETAILSWIDGET_H