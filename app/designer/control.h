#ifndef CONTROL_H
#define CONTROL_H

#include <anchors.h>
#include <renderinfo.h>
#include <designeritem.h>

class Control : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(Control)

    friend class DesignerScene; // For m_geometrySyncKey

public:
    enum { Type = UserType + 3 };

public:
    explicit Control(Control* parent = nullptr);
    ~Control() override;

    int type() const override;
    bool gui() const;
    bool popup() const;
    bool window() const;
    bool visible() const;
    bool hasErrors() const;
    bool overlayPopup() const;
    bool hasWindowAncestor() const;

    Anchors* anchors() const;
    QMarginsF margins() const;
    QVector<QString> events() const;
    QVector<QmlError> errors() const;
    QVector<PropertyNode> properties() const;

    quint32 index() const;
    void setIndex(quint32 index);

    QString id() const;
    void setId(const QString& id);

    QString uid() const;
    void setUid(const QString& uid);

    QString dir() const;
    void setDir(const QString& dir);

    QPixmap pixmap() const;
    void setPixmap(const QPixmap& pixmap);

    RenderInfo renderInfo() const;
    void setRenderInfo(const RenderInfo& info);

    void syncGeometry();
    bool geometrySyncEnabled() const;
    void setGeometrySyncEnabled(bool geometrySyncEnabled);

    void updateAnchors();
    QVariant property(const QString& propertyName) const;
    Control* parentControl() const;
    Control* topLevelControl() const;

    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;

    static QList<Control*> controls();

protected:
    QRectF surroundingRect() const;
    void paintContent(QPainter* painter);
    void paintHighlight(QPainter* painter);
    void paintOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QVariant itemChange(int change, const QVariant& value) override;

signals:
    void renderInfoChanged(bool codeChanged);

private:
    static QList<Control*> s_controls;
    quint32 m_index;
    QString m_id;
    QString m_uid;
    QString m_dir;
    QPixmap m_pixmap;
    Anchors* m_anchors;
    RenderInfo m_renderInfo;
    QSizeF m_snapMargin;
    QString m_geometrySyncKey;
    bool m_geometrySyncEnabled;
    bool m_updateAnchorsScheduled;
};

#endif // CONTROL_H
