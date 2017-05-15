#include <savemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <model.h>
#include <rewriterview.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <modelnode.h>
#include <plaintexteditmodifier.h>
#include <qmlobjectnode.h>
#include <variantproperty.h>
#include <QPlainTextEdit>
#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <pageswidget.h>
#include <mainwindow.h>
#include <bindingproperty.h>
#include <bindingwidget.h>
#include <qmleditor.h>
#include <QApplication>
#include <QFileSystemWatcher>

#define SAVE_DIRECTORY "dashboard"
#define PARENTAL_RELATIONSHIP_FILE "parental_relationship.json"
#define PAGE_ORDER_FILE "page_order.json"
#define BINDINGS_FILE "bindings.json"
#define BINDING_SOURCE_ID_LABEL "sourceId"
#define BINDING_SOURCE_PROPERTY_LABEL "sourceProperty"
#define BINDING_TARGET_ID_LABEL "targetId"
#define BINDING_TARGET_PROPERTY_LABEL "targetProperty"

using namespace QmlDesigner;
using namespace QmlJS;

class SaveManagerPrivate
{
	public:
		SaveManagerPrivate(SaveManager* uparent);
        QString generateSavesDirectory() const;
		QString generateSaveDirectory(const QString& id) const;
		void parseImportDirectories(const QString& dir);
		void initPageOrder(const QString& file) const;
		void initEmptyParentalFile(const QString& file) const;
		void createPages(const QJsonArray& pages);
		bool fillDashboard(const QJsonObject& parentalRelationships, const QJsonArray& pages);
        void fillBindings(const QJsonObject& bindingSaves);
        void clearQmlCaches();

	public:
		SaveManager* parent = nullptr;
		QPlainTextEdit* plainTextEdit;
		ModelManagerInterface* modelManager;
        QTimer applierTimer;
        QString id, newId;
        QFileSystemWatcher fsWatcher;
};

SaveManagerPrivate::SaveManagerPrivate(SaveManager* uparent)
	: parent(uparent)
{
	plainTextEdit = new QPlainTextEdit;
	plainTextEdit->setHidden(true);
	modelManager = new ModelManagerInterface;
    applierTimer.setInterval(500);
    QObject::connect(&applierTimer, SIGNAL(timeout()), parent, SLOT(idApplier()));
    QObject::connect(&fsWatcher, (void(QFileSystemWatcher::*)(QString))(&QFileSystemWatcher::directoryChanged),
                     [=](QString){ clearQmlCaches(); });
    for (auto importPath : QQmlEngine().importPathList()) parseImportDirectories(importPath);
}

QString SaveManagerPrivate::generateSavesDirectory() const
{
    auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
    if (projectDir.isEmpty()) return projectDir;
    return projectDir + separator() + SAVE_DIRECTORY;
}

inline QString SaveManagerPrivate::generateSaveDirectory(const QString& id) const
{
    auto baseDir = generateSavesDirectory();
    if (baseDir.isEmpty()) return baseDir;
    return baseDir + separator() + id;
}

void SaveManagerPrivate::parseImportDirectories(const QString& dir)
{
	modelManager->updateLibraryInfo(dir, LibraryInfo(LibraryInfo::Found));
	for (auto subdir: lsdir(dir)) parseImportDirectories(dir + separator() + subdir);
}

void SaveManagerPrivate::initPageOrder(const QString& file) const
{
	QJsonArray jArray;
	jArray.append("page1");
	QJsonDocument jDoc(jArray);
	wrfile(file, jDoc.toJson());
}

void SaveManagerPrivate::initEmptyParentalFile(const QString& file) const
{
	QJsonObject jObj;
	QJsonDocument jDoc(jObj);
	wrfile(file, jDoc.toJson());
}

void SaveManagerPrivate::createPages(const QJsonArray& pages)
{
	auto firstPage = pages[0].toString();
	PagesWidget::changePageWithoutSave("page1", firstPage);
	for (int i = 1; i < pages.size(); i++) {
		auto currPage = pages[i].toString();
		PagesWidget::addPageWithoutSave(currPage);
	}
}

bool SaveManagerPrivate::fillDashboard(const QJsonObject& parentalRelationships, const QJsonArray& pages)
{
	QStringList createdObjects;
	for (auto page : pages) {
		createdObjects << page.toString();
	}
	while (!createdObjects.isEmpty()) {
		for (auto key : parentalRelationships.keys()) {
			if (parentalRelationships[key].toString() == createdObjects[0]) {
				auto saveId = parent->saveDirectory(key);
				if (saveId.isEmpty()) return false;
				if (!MainWindow::addControlWithoutSave(QUrl::fromLocalFile(saveId + separator() + "main.qml"),
													   parentalRelationships[key].toString()))
					return false;
				createdObjects.append(key);
                fsWatcher.addPath(generateSaveDirectory(key));
                clearQmlCaches();
			}
		}
		createdObjects.removeFirst();
	}
	return true;
}

void SaveManagerPrivate::fillBindings(const QJsonObject& bindingSaves)
{
    SaveManager::BindingInf inf;
    for (auto bindingKey : bindingSaves.keys()) {
        inf.bindingName = bindingKey;
        inf.sourceId = bindingSaves[bindingKey].toObject()[BINDING_SOURCE_ID_LABEL].toString();
        inf.sourceProperty = bindingSaves[bindingKey].toObject()[BINDING_SOURCE_PROPERTY_LABEL].toString();
        inf.targetId = bindingSaves[bindingKey].toObject()[BINDING_TARGET_ID_LABEL].toString();
        inf.targetProperty = bindingSaves[bindingKey].toObject()[BINDING_TARGET_PROPERTY_LABEL].toString();
        BindingWidget::addBindingWithoutSave(inf);
    }
}

void SaveManagerPrivate::clearQmlCaches()
{
//    for (auto save : parent->saves()) {
//        rm(parent->saveDirectory(save) + separator() + "main.qmlc");
//    }
}

SaveManagerPrivate* SaveManager::m_d = nullptr;

SaveManager::SaveManager(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
    m_d = new SaveManagerPrivate(this);
}

SaveManager::~SaveManager()
{
    delete m_d;
}

SaveManager* SaveManager::instance()
{
	return m_d->parent;
}

QString SaveManager::saveDirectory(const QString& id)
{
	if (!exists(id)) return QString();
    return m_d->generateSaveDirectory(id);
}

QString SaveManager::savesDirectory()
{
    return m_d->generateSavesDirectory();
}

QJsonObject SaveManager::getBindingSaves()
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return QJsonObject();
	auto bindingsFile = projDir + separator() + SAVE_DIRECTORY + separator() + BINDINGS_FILE;
	return QJsonDocument::fromJson(rdfile(bindingsFile)).object();
}

void SaveManager::addBindingSave(const SaveManager::BindingInf& bindingInf)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto bindingFile = projDir + separator() + SAVE_DIRECTORY + separator() + BINDINGS_FILE;
	QJsonObject cObj;
	cObj[BINDING_SOURCE_ID_LABEL] = bindingInf.sourceId;
	cObj[BINDING_SOURCE_PROPERTY_LABEL] = bindingInf.sourceProperty;
	cObj[BINDING_TARGET_ID_LABEL] = bindingInf.targetId;
	cObj[BINDING_TARGET_PROPERTY_LABEL] = bindingInf.targetProperty;
	QJsonObject pObj = QJsonDocument::fromJson(rdfile(bindingFile)).object();
	pObj[bindingInf.bindingName] = cObj;
	wrfile(bindingFile, QJsonDocument(pObj).toJson());
//	setBindingProperty(bindingInf.targetId, bindingInf.targetProperty, QString("%1.%2").
//					   arg(bindingInf.sourceId).arg(bindingInf.sourceProperty));
}

void SaveManager::changeBindingSave(const QString& bindingName, const SaveManager::BindingInf& toBindingInf)
{
	removeBindingSave(bindingName);
	addBindingSave(toBindingInf);
}

void SaveManager::removeBindingSave(const QString& bindingName)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto bindingFile = projDir + separator() + SAVE_DIRECTORY + separator() + BINDINGS_FILE;
//    auto binding = getBindingSaves()[bindingName].toObject();
	QJsonObject jObj = QJsonDocument::fromJson(rdfile(bindingFile)).object();
	jObj.remove(bindingName);
    wrfile(bindingFile, QJsonDocument(jObj).toJson());
//	removeProperty(binding[BINDING_TARGET_ID_LABEL].toString(), binding[BINDING_TARGET_PROPERTY_LABEL].toString());
}

QJsonObject SaveManager::getParentalRelationships()
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return QJsonObject();
	auto parentalFile = projDir + separator() + SAVE_DIRECTORY + separator() + PARENTAL_RELATIONSHIP_FILE;
	return QJsonDocument::fromJson(rdfile(parentalFile)).object();
}

QJsonArray SaveManager::getPageOrders()
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return QJsonArray();
	auto pageOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + PAGE_ORDER_FILE;
	return QJsonDocument::fromJson(rdfile(pageOrderFile)).array();
}

bool SaveManager::exists(const QString& id)
{
	auto saveDir = m_d->generateSaveDirectory(id);
	if (saveDir.isEmpty()) return false;
	return ::exists(saveDir);
}

QStringList SaveManager::saves()
{
	QStringList saveList;
	auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projectDir.isEmpty()) return saveList;
	auto saveBaseDir = projectDir + separator() + SAVE_DIRECTORY;
	for (auto save : lsdir(saveBaseDir)) {
		saveList << save;
	}
    return saveList;
}

void SaveManager::addSave(const QString& id, const QString& url)
{
	if (exists(id)) return;
	auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projectDir.isEmpty()) return;
	auto saveBaseDir = projectDir + separator() + SAVE_DIRECTORY;
	if (!mkdir(saveBaseDir + separator() + id)) return;
    cp(dname(url), saveBaseDir + separator() + id, true);
    m_d->clearQmlCaches();
    m_d->fsWatcher.addPath(m_d->generateSaveDirectory(id));
}

void SaveManager::changeSave(const QString& fromId, QString toId)
{
	if (!exists(fromId) || saves().contains(toId)) return;
	auto saveDir = saveDirectory(fromId);
	if (saveDir.isEmpty()) return;
	rn(saveDir, dname(saveDir) + separator() + toId);
    SaveManager::BindingInf inf;
    auto bindingSaves = getBindingSaves();
    for (auto bindingKey : bindingSaves.keys()) {
        inf.bindingName = bindingKey;
        inf.sourceId = bindingSaves[bindingKey].toObject()[BINDING_SOURCE_ID_LABEL].toString();
        inf.sourceProperty = bindingSaves[bindingKey].toObject()[BINDING_SOURCE_PROPERTY_LABEL].toString();
        inf.targetId = bindingSaves[bindingKey].toObject()[BINDING_TARGET_ID_LABEL].toString();
        inf.targetProperty = bindingSaves[bindingKey].toObject()[BINDING_TARGET_PROPERTY_LABEL].toString();

        if (inf.targetId == fromId && inf.sourceId == fromId) {
            inf.targetId = toId;
            inf.sourceId = toId;
        } else if (inf.targetId == fromId) {
            inf.targetId = toId;
        } else if(inf.sourceId == fromId) {
            inf.sourceId = toId;
        } else {
            continue;
        }
        changeBindingSave(inf.bindingName, inf);
    }
}

void SaveManager::removeSave(const QString& id)
{
	if (!exists(id)) return;
    rm(m_d->generateSaveDirectory(id));
    m_d->clearQmlCaches();
    m_d->fsWatcher.removePath(m_d->generateSaveDirectory(id));
}

bool SaveManager::buildNewDatabase(const QString& projDir)
{
	auto saveBaseDir = projDir + separator() + SAVE_DIRECTORY;
	if (!mkdir(saveBaseDir)) return false;
	if (!mkfile(saveBaseDir + separator() + PARENTAL_RELATIONSHIP_FILE)) return false;
	if (!mkfile(saveBaseDir + separator() + PAGE_ORDER_FILE)) return false;
	m_d->initPageOrder(saveBaseDir + separator() + PAGE_ORDER_FILE);
	m_d->initEmptyParentalFile(saveBaseDir + separator() + PARENTAL_RELATIONSHIP_FILE);
	return true;
}

bool SaveManager::loadDatabase()
{
	QJsonArray pageOrder = getPageOrders();
	QJsonObject parentalRelationship = getParentalRelationships();
	QJsonObject bindingSaves = getBindingSaves();
	if (pageOrder.isEmpty()) return false;
	if (parentalRelationship.size() != saves().size()) return false;
	m_d->createPages(pageOrder);
	if (!m_d->fillDashboard(parentalRelationship, pageOrder)) return false;
    m_d->fillBindings(bindingSaves);
    return true;
}

void SaveManager::setVariantProperty(const QString& id, const QString& property, const QVariant& value)
{
	if (saveDirectory(id).isEmpty()) return;
	auto mainQmlFilename = saveDirectory(id) + separator() + "main.qml";
	QString mainQmlContent = rdfile(mainQmlFilename);
	if (mainQmlContent.isEmpty()) return;
	m_d->plainTextEdit->setPlainText(mainQmlContent);
    auto model = Model::create("QtQuick.Item", 1, 0);
    auto rewriterView = new RewriterView(RewriterView::Amend, model);
    auto textModifier = new NotIndentingTextEditModifier(m_d->plainTextEdit);
    model->setTextModifier(textModifier);
    model->setRewriterView(rewriterView);
    model->setFileUrl(QUrl::fromLocalFile(mainQmlFilename));
    auto rootNode = rewriterView->rootModelNode();
	QmlObjectNode(rootNode).setVariantProperty(QByteArray().insert(0, property), value);
	wrfile(mainQmlFilename, QByteArray().insert(0, m_d->plainTextEdit->toPlainText()));
    QmlEditor::clearCacheFor(saveDirectory(id), true);
    delete rewriterView;
    delete textModifier;
    delete model;
}

void SaveManager::setBindingProperty(const QString& id, const QString& property, const QString& expression) //FIXME:
{
	if (saveDirectory(id).isEmpty()) return;
	auto mainQmlFilename = saveDirectory(id) + separator() + "main.qml";
	QString mainQmlContent = rdfile(mainQmlFilename);
	if (mainQmlContent.isEmpty()) return;
	m_d->plainTextEdit->setPlainText(mainQmlContent);
    auto model = Model::create("QtQuick.Item", 1, 0);
    auto rewriterView = new RewriterView(RewriterView::Amend, model);
    auto textModifier = new NotIndentingTextEditModifier(m_d->plainTextEdit);
    model->setTextModifier(textModifier);
    model->setRewriterView(rewriterView);
    model->setFileUrl(QUrl::fromLocalFile(mainQmlFilename));
    ModelNode rootNode = rewriterView->rootModelNode();
	QmlObjectNode(rootNode).setBindingProperty(QByteArray().insert(0, property), expression);
	wrfile(mainQmlFilename, QByteArray().insert(0, m_d->plainTextEdit->toPlainText()));
    delete rewriterView;
    delete textModifier;
    delete model;
}

void SaveManager::removeProperty(const QString& id, const QString& property) //FIXME: FOR BINDING PROPERTIES
{
	if (saveDirectory(id).isEmpty()) return;
	auto mainQmlFilename = saveDirectory(id) + separator() + "main.qml";
	QString mainQmlContent = rdfile(mainQmlFilename);
	if (mainQmlContent.isEmpty()) return;
	m_d->plainTextEdit->setPlainText(mainQmlContent);
    auto model = Model::create("QtQuick.Item", 1, 0);
    auto rewriterView = new RewriterView(RewriterView::Amend, model);
    auto textModifier = new NotIndentingTextEditModifier(m_d->plainTextEdit);
    model->setTextModifier(textModifier);
    model->setRewriterView(rewriterView);
    model->setFileUrl(QUrl::fromLocalFile(mainQmlFilename));
    ModelNode rootNode = rewriterView->rootModelNode();
    rootNode.removeProperty(QByteArray().insert(0, property));
    QmlObjectNode(rootNode).removeProperty(QByteArray().insert(0, property));
    wrfile(mainQmlFilename, QByteArray().insert(0, m_d->plainTextEdit->toPlainText()));
    delete rewriterView;
    delete textModifier;
    delete model;
}

void SaveManager::addParentalRelationship(const QString& id, const QString& parent)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto parentalFile = projDir + separator() + SAVE_DIRECTORY + separator() + PARENTAL_RELATIONSHIP_FILE;
	QJsonObject jObj = QJsonDocument::fromJson(rdfile(parentalFile)).object();
	jObj[id] = parent;
	wrfile(parentalFile, QJsonDocument(jObj).toJson());
}

void SaveManager::removeParentalRelationship(const QString& id)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto parentalFile = projDir + separator() + SAVE_DIRECTORY + separator() + PARENTAL_RELATIONSHIP_FILE;
	QJsonObject jObj = QJsonDocument::fromJson(rdfile(parentalFile)).object();
	jObj.remove(id);
	wrfile(parentalFile, QJsonDocument(jObj).toJson());
}

QString SaveManager::parentalRelationship(const QString& id)
{
	auto jObj = getParentalRelationships();
	return jObj[id].toString();
}

void SaveManager::addPageOrder(const QString& pageId)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto pageOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + PAGE_ORDER_FILE;
	QJsonArray jArr = QJsonDocument::fromJson(rdfile(pageOrderFile)).array();
	jArr.append(pageId);
	wrfile(pageOrderFile, QJsonDocument(jArr).toJson());
}

void SaveManager::removePageOrder(const QString& pageId)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto pageOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + PAGE_ORDER_FILE;
	QJsonArray jArr = QJsonDocument::fromJson(rdfile(pageOrderFile)).array();
	for (int i = 0; i < jArr.size(); i++) {
		if (jArr.at(i) == pageId) {
			jArr.removeAt(i);
			wrfile(pageOrderFile, QJsonDocument(jArr).toJson());
			break;
		}
	}
}

void SaveManager::changePageOrder(const QString& fromPageId, const QString& toPageId)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
	auto pageOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + PAGE_ORDER_FILE;
	QJsonArray jArr = QJsonDocument::fromJson(rdfile(pageOrderFile)).array();
	for (int i = 0; i < jArr.size(); i++) {
		if (jArr.at(i) == fromPageId) {
			jArr[i] = toPageId;
			wrfile(pageOrderFile, QJsonDocument(jArr).toJson());
			break;
		}
    }
}

void SaveManager::setId(const QString& id, const QString& newId)
{
    m_d->id = id;
    m_d->newId = newId;
    m_d->applierTimer.start();
}

void SaveManager::idApplier()
{
    m_d->applierTimer.stop();
   if (saveDirectory(m_d->id).isEmpty()) return;
   auto mainQmlFilename = saveDirectory(m_d->id) + separator() + "main.qml";
   QString mainQmlContent = rdfile(mainQmlFilename);
   if (mainQmlContent.isEmpty()) return;
   m_d->plainTextEdit->setPlainText(mainQmlContent);
   auto model = Model::create("QtQuick.Item", 1, 0);
   auto rewriterView = new RewriterView(RewriterView::Amend, model);
   auto textModifier = new NotIndentingTextEditModifier(m_d->plainTextEdit);
   model->setTextModifier(textModifier);
   model->setRewriterView(rewriterView);
   model->setFileUrl(QUrl::fromLocalFile(mainQmlFilename));
   ModelNode rootNode = rewriterView->rootModelNode();
   QmlObjectNode(rootNode).setId(m_d->newId);
   wrfile(mainQmlFilename, QByteArray().insert(0, m_d->plainTextEdit->toPlainText()));
   delete rewriterView;
   delete textModifier;
   delete model;
}
