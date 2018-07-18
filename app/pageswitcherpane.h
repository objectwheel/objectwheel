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
    void setLeftPanesShow(bool);
    void setRightPanesShow(bool);

protected:
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void changeLeftShowHideButtonToolTip(bool);
    void changeRightShowHideButtonToolTip(bool);

signals:
    void buildsActivated();
    void designerActivated();
    void splitViewActivated();
    void helpActivated();
    void qmlCodeEditorActivated();
    void projectOptionsActivated();
    void currentPageChanged(const Pages& page);
    void leftPanesShowChanged(bool);
    void rightPanesShowChanged(bool);

private:
    QVBoxLayout* m_layout;
    FlatButton* m_qmlCodeEditorButton;
    FlatButton* m_designerButton;
    FlatButton* m_projectOptionsButton;
    FlatButton* m_buildsButton;
    FlatButton* m_helpButton;
    FlatButton* m_splitViewButton;
    FlatButton* m_hideShowLeftPanesButton;
    FlatButton* m_hideShowRightPanesButton;

};

#endif // PAGESWITCHERPANE_H