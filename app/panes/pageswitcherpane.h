#ifndef PAGESWITCHERPANE_H
#define PAGESWITCHERPANE_H

#include <QWidget>
#include <pages.h>

class QToolButton;
class QVBoxLayout;

class PageSwitcherPane : public QWidget
{
    Q_OBJECT

public:
    explicit PageSwitcherPane(QWidget* parent = nullptr);
    Pages currentPage() const;
    bool isPageEnabled(const Pages& page) const;

public slots:
    void discharge();
    void setCurrentPage(const Pages& page);
    void setPageEnabled(const Pages& page);
    void setPageDisabled(const Pages& page);

private slots:
    void updateColors();

protected:
    void paintEvent(QPaintEvent* e) override;

signals:
    void buildsActivated();
    void designerActivated();
    void splitViewActivated();
    void helpActivated();
    void qmlCodeEditorActivated();
    void projectOptionsActivated();
    void currentPageChanged(const Pages& page);

private:
    QVBoxLayout* m_layout;
    QToolButton* m_qmlCodeEditorButton;
    QToolButton* m_designerButton;
    QToolButton* m_projectOptionsButton;
    QToolButton* m_buildsButton;
    QToolButton* m_helpButton;
    QToolButton* m_splitViewButton;
};

#endif // PAGESWITCHERPANE_H