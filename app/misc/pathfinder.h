#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QString>

class Control;

class PathFinder final
{
    Q_DISABLE_COPY(PathFinder)

public:
    struct Result {
        int line;
        int begin, end, length;
        QString relativePath;
        virtual bool isNull() const = 0;
        enum Type { Assets, Designs } type;
    };

    struct AssetsResult : public Result {
        bool isNull() const override
        { return relativePath.isEmpty(); }
    };

    struct DesignsResult : public Result {
        Control* control = nullptr;
        bool isNull() const override
        { return !control || relativePath.isEmpty(); }
    };

public:
    PathFinder() = delete;
    static QString cleansed(const QString& text, bool withUid = false);
    static QString locallyCleansed(const QString& text, bool withUid = false);
    static AssetsResult findAssets(const QString& line);
    static DesignsResult findDesigns(const QString& line);
};

#endif // PATHFINDER_H