#include <qmlformatter.h>
#include <qmljs/qmljsreformatter.h>
#include <qmljs/qmljsdocument.h>
#include <qmljs/qmljsbind.h>
#include <qmljs/parser/qmljsast_p.h>
#include <metainforeader.h>
#include <qmljs/qmljspropertyreader.h>
#include <QPlainTextEdit>

QmlFormatter::QmlFormatter(QObject *parent) : QObject(parent)
{
}

void QmlFormatter::format(QString& text)
{
    using namespace QmlJS;
    using namespace AST;

    Dialect dialect(Dialect::Qml);
    Document::MutablePtr document = Document::create(":/main.qml", dialect);
    document->setSource(text);
    document->parse();

    text = reformat(document);
}
