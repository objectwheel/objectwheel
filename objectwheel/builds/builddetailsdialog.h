#ifndef BUILDDETAILSDIALOG_H
#define BUILDDETAILSDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class BuildDetailsDialog final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildDetailsDialog)

public:
    explicit BuildDetailsDialog(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QPlainTextEdit* m_textEdit;
};

#endif // BUILDDETAILSDIALOG_H
