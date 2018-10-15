#include <pathfinder.h>
#include <saveutils.h>
#include <control.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <QRegularExpression>

namespace {
const char* g_globalPattern = "%1::([\\w\\\\\\/\\.\\d]+):(\\d+)";
const char* g_internalPattern = "[a-z_][a-zA-Z0-9_]+::([a-f0-9]+)::([\\w\\\\\\/\\.\\d]+):(\\d+)";
}

QString PathFinder::cleansed(const QString& text, bool withUid)
{
    QString cleansed(text);
    for (const Control* control : Control::controls()) {
        QRegularExpression exp("file:\\/{1,3}" + SaveUtils::toThisDir(control->dir()) + separator());
        const QString& clean = control->id() + "::" + (withUid ? control->uid() + "::" : "");
        if (cleansed.contains(exp))
            cleansed.replace(exp, clean);
    }

    QRegularExpression exp("file:\\/{1,3}" + SaveUtils::toGlobalDir(ProjectManager::dir()) + separator());
    const QString& clean = QObject::tr("GlobalResources") + "::";
    if (cleansed.contains(exp))
        cleansed.replace(exp, clean);

    return cleansed;
}

PathFinder::GlobalResult PathFinder::findGlobal(const QString& line)
{
    const QRegularExpression exp(QString(g_globalPattern).arg(QObject::tr("GlobalResources")));
    const QRegularExpressionMatch& match = exp.match(line);

    GlobalResult result;
    result.begin = match.capturedStart();
    result.end = match.capturedEnd();
    result.length = result.end - result.begin;
    result.relativePath = match.captured(1);
    result.line = match.captured(2).toInt();

    return result;
}

PathFinder::InternalResult PathFinder::findInternal(const QString& line)
{
    const QRegularExpression exp(g_internalPattern);
    const QRegularExpressionMatch& match = exp.match(line);
    const QString& uid = match.captured(1);

    InternalResult result;
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
