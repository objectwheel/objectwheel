#include <pathfinder.h>
#include <saveutils.h>
#include <control.h>
#include <projectmanager.h>
#include <runmanager.h>
#include <QRegularExpression>

namespace {

QString withBase(const QString& path, const QString& base)
{
    QString copy(path);
    return copy.replace(ProjectManager::dir(), base);
}

}

QString PathFinder::cleansed(const QString& text, bool withUid)
{
    QString cleansed(text);
    for (const Control* control : Control::controls()) {
        const QString& clean = control->id() + "::" + (withUid ? control->uid() + "::" : "");
        QRegularExpression exp("file:\\/{1,3}" + SaveUtils::toThisDir(
            withBase(control->dir(), RunManager::recentProjectDirectory())) + '/');
        if (cleansed.contains(exp))
            cleansed.replace(exp, clean);
    }

    const QString& clean = QObject::tr("GlobalResources") + "::";
    QRegularExpression exp("file:\\/{1,3}" +
        SaveUtils::toGlobalDir(RunManager::recentProjectDirectory()) + '/');
    if (cleansed.contains(exp))
        cleansed.replace(exp, clean);

    return cleansed;
}

QString PathFinder::locallyCleansed(const QString& text, bool withUid)
{
    QString cleansed(text);
    for (const Control* control : Control::controls()) {
        const QString& clean = control->id() + "::" + (withUid ? control->uid() + "::" : "");
        QRegularExpression exp("file:\\/{1,3}" + SaveUtils::toThisDir(control->dir()) + '/');
        if (cleansed.contains(exp))
            cleansed.replace(exp, clean);
    }

    const QString& clean = QObject::tr("GlobalResources") + "::";
    QRegularExpression exp("file:\\/{1,3}" + SaveUtils::toGlobalDir(ProjectManager::dir()) + '/');
    if (cleansed.contains(exp))
        cleansed.replace(exp, clean);

    return cleansed;
}

PathFinder::GlobalResult PathFinder::findGlobal(const QString& line)
{
    const QRegularExpression exp(QString("%1::([\\w\\\\\\/\\.\\d]+):-?(\\d+)").arg(QObject::tr("GlobalResources")));
    const QRegularExpressionMatch& match = exp.match(line);

    GlobalResult result;
    result.type = Result::Global;
    result.begin = match.capturedStart();
    result.end = match.capturedEnd();
    result.length = result.end - result.begin;
    result.relativePath = match.captured(1);
    result.line = match.captured(2).toInt();

    return result;
}

PathFinder::InternalResult PathFinder::findInternal(const QString& line)
{
    const QRegularExpression exp("[a-z_][a-zA-Z0-9_]+::([a-f0-9]+)::([\\w\\\\\\/\\.\\d]+):-?(\\d+)");
    const QRegularExpressionMatch& match = exp.match(line);
    const QString& uid = match.captured(1);

    InternalResult result;
    result.type = Result::Internal;
    result.begin = match.capturedStart();
    result.end = match.capturedEnd();
    result.length = result.end - result.begin;
    result.relativePath = match.captured(2);
    result.line = match.captured(3).toInt();

    for (Control* control : Control::controls()) {
        if (control->uid() == uid)
            result.control = control;
    }

    return result;
}
