#include <utilityfunctions.h>
#include <saveutils.h>

#include <QQmlEngine>
#include <QTextDocument>
#include <QRegularExpression>
#include <QTextCursor>
#include <QWidget>

namespace {

QString projectDirectory;
}

void UtilityFunctions::registerGlobalPath(const QString& projectDir)
{
    projectDirectory = projectDir;
    qmlRegisterSingletonType("Global", 1, 0, "Global",
                             [] (QQmlEngine* engine, QJSEngine* scriptEngine) -> QJSValue {
        Q_UNUSED(engine)
        QJSValue globalPath = scriptEngine->newObject();
        globalPath.setProperty("path", SaveUtils::toGlobalDir(projectDirectory));
        globalPath.setProperty("url", scriptEngine->toScriptValue(
                                   QUrl::fromLocalFile(SaveUtils::toGlobalDir(projectDirectory))));
        return globalPath;
    });
}

void UtilityFunctions::trimCommentsAndStrings(QTextDocument* document)
{
    QRegularExpression exp("(([\"'])(?:\\\\[\\s\\S]|.)*?\\2|\\/(?![*\\/])(?:\\\\.|\\[(?:\\\\.|.)\\]|.)*?\\/)|\\/\\/.*?$|\\/\\*[\\s\\S]*?\\*\\/",
                           QRegularExpression::MultilineOption); // stackoverflow.com/q/24518020

    QRegularExpressionMatchIterator i = exp.globalMatch(document->toPlainText());
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString capturedText = match.captured();
        int start = match.capturedStart();
        int end = match.capturedEnd();

        capturedText.replace(QRegularExpression("[^\\n\\r\\'\\\"]", QRegularExpression::MultilineOption), "x");

        QTextCursor cursor(document);
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.insertText(capturedText);
    }
}

QWidget* UtilityFunctions::createSpacerWidget(Qt::Orientation orientation)
{
    auto spacer = new QWidget;
    spacer->setSizePolicy((orientation & Qt::Horizontal)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred,
                          (orientation & Qt::Vertical)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred);
    return spacer;
}
