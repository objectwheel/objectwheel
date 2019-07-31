#include <pathfinder.h>
#include <saveutils.h>
#include <control.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <QRegularExpression>

static QString withBase(const QString& path, const QString& base)
{
    QString copy(path);
    return copy.replace(ProjectManager::dir(), base);
}

QString PathFinder::cleansed(const QString& text, bool withUid)
{
    QString cleansed(text);
// FIXME   for (const Control* control : Control::controls()) {
//        const QString& clean = control->id() + "::" + (withUid ? control->uid() + "::" : "");
//        QRegularExpression exp("file:\\/{1,3}" + QRegularExpression::escape(SaveUtils::toControlThisDir(
//            withBase(control->dir(), RunManager::recentProjectDirectory())) + '/'));
//        if (cleansed.contains(exp))
//            cleansed.replace(exp, clean);
//    }

    const QString& clean = QObject::tr("Assets::");
    QRegularExpression exp("file:\\/{1,3}" + QRegularExpression::escape(SaveUtils::toProjectAssetsDir(RunManager::recentProjectDirectory()) + '/'));
    if (cleansed.contains(exp))
        cleansed.replace(exp, clean);

    return cleansed;
}

QString PathFinder::locallyCleansed(const QString& text, bool withUid)
{
    QString cleansed(text);
//  FIXME  for (const Control* control : Control::controls()) {
//        const QString& clean = control->id() + "::" + (withUid ? control->uid() + "::" : "");
//        QRegularExpression exp("file:\\/{1,3}" + QRegularExpression::escape(SaveUtils::toControlThisDir(control->dir()) + '/'));
//        if (cleansed.contains(exp))
//            cleansed.replace(exp, clean);
//    }

    const QString& clean = QObject::tr("Assets::");
    QRegularExpression exp("file:\\/{1,3}" + QRegularExpression::escape(SaveUtils::toProjectAssetsDir(ProjectManager::dir()) + '/'));
    if (cleansed.contains(exp))
        cleansed.replace(exp, clean);

    return cleansed;
}

PathFinder::AssetsResult PathFinder::findAssets(const QString& line)
{
    const QRegularExpression exp(QString("%1::([\\w\\\\\\/\\.\\d]+):-?(\\d+)").arg(QObject::tr("Assets")));
    const QRegularExpressionMatch& match = exp.match(line);

    AssetsResult result;
    result.type = Result::Assets;
    result.begin = match.capturedStart();
    result.end = match.capturedEnd();
    result.length = result.end - result.begin;
    result.relativePath = match.captured(1);
    result.line = match.captured(2).toInt();

    return result;
}

PathFinder::DesignsResult PathFinder::findDesigns(const QString& line)
{
    const QRegularExpression exp("[a-z_][a-zA-Z0-9_]+::([a-f0-9]+)::([\\w\\\\\\/\\.\\d]+):-?(\\d+)");
    const QRegularExpressionMatch& match = exp.match(line);
    const QString& uid = match.captured(1);

    DesignsResult result;
    result.type = Result::Designs;
    result.begin = match.capturedStart();
    result.end = match.capturedEnd();
    result.length = result.end - result.begin;
    result.relativePath = match.captured(2);
    result.line = match.captured(3).toInt();

// FIXME   for (Control* control : Control::controls()) {
//        if (control->uid() == uid)
//            result.control = control;
//    }

    return result;
}
