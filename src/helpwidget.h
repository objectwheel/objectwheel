#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>

class QGridLayout;
class QComboBox;
class QHelpEngine;
class WebEngineHelpViewer;

class HelpWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit HelpWidget(QWidget *parent = nullptr);
        void showHelpForKeyword(const QString& id);

    protected:
        QSize sizeHint() const override;

    public slots:
        void reset();

    private:
        QHelpEngine* m_helpEngine;
        QGridLayout* m_layout;
        QComboBox* m_indexTypeCombo;
        WebEngineHelpViewer* m_helpViewer;
};

#endif // HELPWIDGET_H