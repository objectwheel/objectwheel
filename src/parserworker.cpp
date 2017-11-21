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

QmlJS::ModelManagerInterface* ParserWorker::_modelManager = nullptr;

ParserWorker::ParserWorker(QObject *parent)
    : QObject(parent)
{
    if (!_modelManager) {
        _modelManager = new ModelManagerInterface(this);

        QQmlEngine engine;
        for (auto importPath : engine.importPathList())
            parseImportDirectories(importPath, _modelManager);
    }
}

QString ParserWorker::typeName(const QByteArray& data) const
{
    QString type;
    if (data.isEmpty())
        return type;

    try {
        auto model = Model::create("QtQuick.Item", 1, 0);
        auto rewriterView = new RewriterView(RewriterView::Amend, model);
        auto textModifier = new NotIndentingTextEditModifier;

        textModifier->setText(data);
        model->setTextModifier(textModifier);
        model->setRewriterView(rewriterView);

        auto rootNode = rewriterView->rootModelNode();
        type = rootNode.simplifiedTypeName();

        textModifier->deleteLater();
        rewriterView->deleteLater();
        model->deleteLater();
    } catch (Exception&) {
        // NOTE
    }
    return type;
}

QVariant ParserWorker::variantProperty(const QByteArray& data, const QString& name) const
{
    QVariant value;
    if (data.isEmpty())
        return value;

    try {
        auto model = Model::create("QtQuick.Item", 1, 0);
        auto rewriterView = new RewriterView(RewriterView::Amend, model);
        auto textModifier = new NotIndentingTextEditModifier;

        textModifier->setText(data);
        model->setTextModifier(textModifier);
        model->setRewriterView(rewriterView);

        auto rootNode = rewriterView->rootModelNode();
        value = rootNode.property(name.toUtf8()).toVariantProperty().value();

        textModifier->deleteLater();
        rewriterView->deleteLater();
        model->deleteLater();
    } catch (Exception&) {
        // NOTE
    }
    return value;
}

//BUG: This function doesn't work for Timer control
void ParserWorker::setVariantProperty(QByteArray& data, const QString& fileName, const QString& property, const QVariant& value) const
{
    if (data.isEmpty()) {
        emit done();
        return;
    }

    try {
        auto bproperty = QByteArray().insert(0, property);
        auto model = Model::create("QtQuick.Item", 1, 0);
        auto rewriterView = new RewriterView(RewriterView::Amend, model);
        auto textModifier = new NotIndentingTextEditModifier;

        textModifier->setText(data);
        model->setTextModifier(textModifier);
        model->setRewriterView(rewriterView);
        model->setFileUrl(QUrl::fromLocalFile(fileName));

        auto rootNode = rewriterView->rootModelNode();
        auto objectNode = QmlObjectNode(rootNode);

        objectNode.setVariantProperty(bproperty, value);
        data = QByteArray().insert(0, textModifier->text());

        textModifier->deleteLater();
        rewriterView->deleteLater();
        model->deleteLater();
    } catch (Exception&) {
        // NOTE
    }

    emit done();
}

void ParserWorker::setVariantProperty(const QString& fileName, const QString& property, const QVariant& value) const
{
    auto fileContent = rdfile(fileName);
    if (fileContent.isEmpty()) {
        emit done();
        return;
    }

    try {
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

        textModifier->deleteLater();
        rewriterView->deleteLater();
        model->deleteLater();
    } catch (Exception&) {
        // NOTE
    }

    emit done();
}

void ParserWorker::removeVariantProperty(const QString& fileName, const QString& property) const
{
    auto fileContent = rdfile(fileName);
    if (fileContent.isEmpty()) {
        emit done();
        return;
    }

    try {
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

        textModifier->deleteLater();
        rewriterView->deleteLater();
        model->deleteLater();
    } catch (Exception&) {
        // NOTE
    }

    emit done();
}
