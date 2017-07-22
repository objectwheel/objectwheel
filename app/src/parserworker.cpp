#include <parserworker.h>
#include <model.h>
#include <rewriterview.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <modelnode.h>
#include <plaintexteditmodifier.h>
#include <qmlobjectnode.h>
#include <variantproperty.h>
#include <filemanager.h>

#include <QQmlEngine>

using namespace QmlDesigner;
using namespace QmlJS;

static void parseImportDirectories(const QString& dir, ModelManagerInterface* modelManager)
{
    modelManager->updateLibraryInfo(dir, LibraryInfo(LibraryInfo::Found));
    for (auto subdir: lsdir(dir))
        parseImportDirectories(dir + separator() + subdir, modelManager);
}

ParserWorker::ParserWorker(QObject *parent) : QObject(parent)
{
    _modelManager = new ModelManagerInterface;

    QQmlEngine engine;
    for (auto importPath : engine.importPathList())
        parseImportDirectories(importPath, _modelManager);
}

void ParserWorker::setVariantProperty(const QString& fileName, const QString& property, const QVariant& value)
{
    auto fileContent = rdfile(fileName);
    if (fileContent.isEmpty())
        return;

    auto bproperty = QByteArray().insert(0, property);
    auto model = Model::create("QtQuick.Item", 1, 0);
    auto rewriterView = new RewriterView(RewriterView::Amend, model);
    auto textModifier = new NotIndentingTextEditModifier;

    textModifier->setText(fileContent);
    model->setTextModifier(textModifier);
    model->setRewriterView(rewriterView);
    model->setFileUrl(QUrl::fromLocalFile(fileName));

    auto rootNode = rewriterView->rootModelNode();
    auto objectNode = QmlObjectNode(rootNode);

    objectNode.setVariantProperty(bproperty, value);
    wrfile(fileName, QByteArray().insert(0, textModifier->text()));

    delete rewriterView;
    delete textModifier;
    delete model;
}

void ParserWorker::removeVariantProperty(const QString& fileName, const QString& property)
{
    auto fileContent = rdfile(fileName);
    if (fileContent.isEmpty())
        return;

    auto bproperty = QByteArray().insert(0, property);
    auto model = Model::create("QtQuick.Item", 1, 0);
    auto rewriterView = new RewriterView(RewriterView::Amend, model);
    auto textModifier = new NotIndentingTextEditModifier;

    textModifier->setText(fileContent);
    model->setTextModifier(textModifier);
    model->setRewriterView(rewriterView);
    model->setFileUrl(QUrl::fromLocalFile(fileName));

    auto rootNode = rewriterView->rootModelNode();
    auto objectNode = QmlObjectNode(rootNode);

    objectNode.removeProperty(bproperty);
    wrfile(fileName, QByteArray().insert(0, textModifier->text()));

    delete rewriterView;
    delete textModifier;
    delete model;
}
