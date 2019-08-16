#ifndef CONTROL_H
#define CONTROL_H

#include <renderinfo.h>
#include <designeritem.h>

class Control : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(Control)

    friend class DesignerScene;

public:
    enum { Type = UserType + 2 };

public:
    explicit Control(Control* parent = nullptr);

    int type() const override;
    bool gui() const;
    bool popup() const;
    bool window() const;
    bool visible() const;
    bool hasErrors() const;

    QMarginsF margins() const;
    QVariantMap anchors() const;
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

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(const qreal& devicePixelRatio);

    RenderInfo renderInfo() const;
    void setRenderInfo(const RenderInfo& info);

    void syncGeometry();
    bool geometrySyncEnabled() const;
    void setGeometrySyncEnabled(bool geometrySyncEnabled);

    QVariant property(const QString& propertyName) const;
    Control* parentControl() const;
    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;

protected:
    QRectF contentRect() const;
    void paintContent(QPainter* painter);
    void paintHighlight(QPainter* painter);
    void paintOutline(QPainter* painter);
    void paintHoverOutline(QPainter* painter);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QVariant itemChange(int change, const QVariant& value) override;

signals:
    void renderInfoChanged(bool codeChanged);

private:
    quint32 m_index;
    QString m_id;
    QString m_uid;
    QString m_dir;
    QPixmap m_pixmap;
    qreal m_devicePixelRatio;
    RenderInfo m_renderInfo;
    QSizeF m_snapMargin;
    QString m_geometrySyncKey;
    bool m_geometrySyncEnabled;
};

#endif // CONTROL_H
