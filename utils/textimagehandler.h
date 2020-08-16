#ifndef TEXTIMAGEHANDLER_H
#define TEXTIMAGEHANDLER_H

#include <utils_global.h>
#include <private/qtextimagehandler_p.h>

class UTILS_EXPORT TextImageHandler final : public QTextImageHandler
{
public:
    explicit TextImageHandler(QObject* parent = nullptr);
    void drawObject(QPainter* p, const QRectF& r, QTextDocument* d, int o, const QTextFormat& f) override;
};

#endif // TEXTIMAGEHANDLER_H
