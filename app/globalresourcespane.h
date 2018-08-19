#ifndef GLOBALRESOURCESPANE_H
#define GLOBALRESOURCESPANE_H

#include <QTreeView>

class FocuslessLineEdit;
class QFileSystemModel;

class GlobalResourcesPane : public QTreeView
{
    Q_OBJECT
public:
    explicit GlobalResourcesPane(QWidget* parent = nullptr);

public slots:
    void sweep();

private slots:
    void onProjectStart();

private:
    void filterList(const QString& filter);
    void paintEvent(QPaintEvent* e) override;
    void updateGeometries() override;
    QSize sizeHint() const override;

private:
    FocuslessLineEdit* m_searchEdit;
    QFileSystemModel* m_fileSystemModel;
};

#endif // GLOBALRESOURCESPANE_H