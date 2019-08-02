#ifndef CONTROL_H
#define CONTROL_H

#include <renderinfo.h>
#include <designeritem.h>

class DesignerScene;
class Control : public DesignerItem
{
    Q_OBJECT
    Q_DISABLE_COPY(Control)

    friend class ControlCreationManager; // For constructor
    friend class ProjectExposingManager; // For constructor
    friend class DesignerScene; // For m_geometryCorrection and m_geometryHash

public:
    enum { Type = UserType + 2 };
    int type() const override;

    bool gui() const;
    bool form() const;
    bool popup() const;
    bool window() const;
    bool visible() const;
    bool hasErrors() const;

    quint32 index() const;
    QString id() const;
    QString uid() const;
    QString dir() const;
    QMarginsF margins() const;
    QPixmap pixmap() const;

    QVector<QString> events() const;
    QVector<QmlError> errors() const;
    QVector<PropertyNode> properties() const;
    QVariant property(const QString& propertyName) const;

    Control* parentControl() const;
    QList<Control*> siblings() const;
    QList<Control*> childControls(bool recursive = true) const;

public:
    void setIndex(quint32 index);
    void setId(const QString& id);
    void setUid(const QString& uid);
    void setDir(const QString& dir);
    void setPixmap(const QPixmap& pixmap);
    void setBoundingRect(const QRectF& outerRect);

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(const qreal& devicePixelRatio);

protected:
    QVariant itemChange(int change, const QVariant& value) override;
    void paintContent(QPainter* painter);
    void paintHighlight(QPainter* painter);
    void paintOutline(QPainter* painter);
    void paintHoverOutline(QPainter* painter, bool hovered);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    explicit Control(Control* parent = nullptr);

private slots:
    void applyGeometryCorrection();
    void setRenderInfo(const RenderInfo& info);

signals:
    void renderInfoChanged(bool codeChanged);

private:
    qreal m_devicePixelRatio;
    quint32 m_index;
    QString m_dir;
    QString m_id;
    QString m_uid;
    QPixmap m_pixmap;
    QSizeF m_snapMargin;
    QMarginsF m_margins;
    QString m_geometryHash;
    QRectF m_geometryCorrection;
    RenderInfo m_renderInfo;
};

#endif // CONTROL_H
