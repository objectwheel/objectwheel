#ifndef MODESELECTORPANE_H
#define MODESELECTORPANE_H

#include <QToolBar>

class FlatButton;
class QVBoxLayout;

class ModeSelectorPane final : public QToolBar
{
    Q_OBJECT
    Q_DISABLE_COPY(ModeSelectorPane)

public:
    explicit ModeSelectorPane(QWidget* parent = nullptr);
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
    FlatButton* m_qmlCodeEditorButton;
    FlatButton* m_designerButton;
    FlatButton* m_projectOptionsButton;
    FlatButton* m_buildsButton;
    FlatButton* m_helpButton;
    FlatButton* m_splitViewButton;
};

#endif // MODESELECTORPANE_H