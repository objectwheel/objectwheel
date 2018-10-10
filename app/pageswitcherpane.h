#ifndef PAGESWITCHERPANE_H
#define PAGESWITCHERPANE_H

#include <QWidget>
#include <pages.h>

class FlatButton;
class QVBoxLayout;

class PageSwitcherPane : public QWidget
{
    Q_OBJECT

public:
    explicit PageSwitcherPane(QWidget* parent = nullptr);
    Pages currentPage() const;
    bool isPageEnabled(const Pages& page) const;

public slots:
    void sweep();
    void setCurrentPage(const Pages& page);
    void setPageEnabled(const Pages& page);
    void setPageDisabled(const Pages& page);

protected:
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

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
    FlatButton* m_qmlCodeEditorButton;
    FlatButton* m_designerButton;
    FlatButton* m_projectOptionsButton;
    FlatButton* m_buildsButton;
    FlatButton* m_helpButton;
    FlatButton* m_splitViewButton;
};

#endif // PAGESWITCHERPANE_H