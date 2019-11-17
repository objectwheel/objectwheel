#ifndef ASSETSPANE_H
#define ASSETSPANE_H

#include <QWidget>

class AssetsTree;
class LineEdit;
class QComboBox;
class FileSearchModel;
class QCompleter;

class AssetsPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AssetsPane)

public:
    explicit AssetsPane(QWidget* parent = nullptr);

    AssetsTree* assetsTree() const;
    QComboBox* modeComboBox() const;
    LineEdit* searchEdit() const;
    QSize sizeHint() const override;

private:
    AssetsTree* m_assetsTree;
    QComboBox* m_modeComboBox;
    FileSearchModel* m_searchEditCompleterModel;
    QCompleter* m_searchEditCompleter;
    LineEdit* m_searchEdit;
};

#endif // ASSETSPANE_H