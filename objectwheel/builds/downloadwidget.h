#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>

class QListWidget;
class ButtonSlice;

class DownloadWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DownloadWidget)

public:
    enum Buttons { New };

public:
    explicit DownloadWidget(QWidget* parent = nullptr);

    QListWidget* platformList() const;
    ButtonSlice* buttonSlice() const;

private:
    QListWidget* m_platformList;
    ButtonSlice* m_buttonSlice;
};

#endif // DOWNLOADWIDGET_H
