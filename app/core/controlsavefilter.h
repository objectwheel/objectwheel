#ifndef CONTROLSAVEFILTER_H
#define CONTROLSAVEFILTER_H

#include <qmlcodeeditorwidget.h>

class ControlSaveFilter final : public QObject, public QmlCodeEditorWidget::SaveFilter
{
    Q_OBJECT

public:
    explicit ControlSaveFilter(QObject* parent = nullptr);

private:
    void beforeSave(QmlCodeEditorWidget::Document*) override;
    void afterSave(QmlCodeEditorWidget::Document*) override;

private:
    QString m_id;

};

#endif // CONTROLSAVEFILTER_H