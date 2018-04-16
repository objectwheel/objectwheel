#ifndef THEMECHOOSERWIDGET_H
#define THEMECHOOSERWIDGET_H

#include <QWidget>

class QLabel;
class QComboBox;
class QToolButton;
class QGridLayout;
class QPushButton;
class WaitingSpinnerWidget;

class ThemeChooserWidget : public QWidget
{
    Q_OBJECT

public:
    enum Version { V1, V2 };

public:
    explicit ThemeChooserWidget(const Version& version, QWidget *parent = nullptr);

public slots:
    void reset();

private slots:
    void run();
    void save();
    void enable();
    void refresh();

private:
    QJsonObject toJson() const;
    QString toItem(QComboBox* comboBox, const QString& colorName) const;

private:
    Version m_version;
    QLabel* m_stylesLabel;
    QComboBox* m_stylesCombo;
    QPushButton* m_seeRunningButton;
    QPushButton* m_saveButton;
    QPushButton* m_resetButton;
    QLabel* m_themesLabel;
    QComboBox* m_themesCombo;
    QLabel* m_detailsLabel;
    QLabel* m_accentDetailLabel;
    QLabel* m_primaryDetailLabel;
    QLabel* m_foregroundDetailLabel;
    QLabel* m_backgroundDetailLabel;
    QComboBox* m_accentColorsCombo;
    QComboBox* m_primaryColorsCombo;
    QComboBox* m_foregroundColorsCombo;
    QComboBox* m_backgroundColorsCombo;
    QLabel* m_accentColorLabel;
    QLabel* m_primaryColorLabel;
    QLabel* m_foregroundColorLabel;
    QLabel* m_backgroundColorLabel;
    QToolButton* m_accentColorButton;
    QToolButton* m_primaryColorButton;
    QToolButton* m_foregroundColorButton;
    QToolButton* m_backgroundColorButton;
    QLabel* m_customizationLabel;
    QLabel* m_customizationPicture;
    QLabel* m_previewLabel;
    QLabel* m_previewPicture;
    QGridLayout* m_gridLayout;
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // THEMECHOOSERWIDGET_H