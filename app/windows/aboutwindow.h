#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QLabel>

class QVBoxLayout;
class QPushButton;

class AboutWindow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AboutWindow)

public:
    explicit AboutWindow(QWidget* parent = nullptr);

    QSize sizeHint() const override;

signals:
    void done();

private:
    QVBoxLayout* m_layout;
    QLabel* m_logoLabel;
    QLabel* m_versionLabel;
    QPushButton* m_okButton;
    QLabel* m_legalLabel;
};

#endif //  ABOUTWINDOW_H
