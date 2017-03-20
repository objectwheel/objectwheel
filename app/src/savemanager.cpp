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

#define SAVE_DIRECTORY "dashboard"
#define PARENTAL_RELATIONSHIP_FILE "parental_relationship.json"
#define PAGE_ORDER_FILE "page_order.json"

using namespace QmlDesigner;
using namespace QmlJS;

class SaveManagerPrivate
{
	public:
		SaveManagerPrivate(SaveManager* uparent);
		QString generateSaveDirectory(const QString& id) const;
		void parseImportDirectories(const QString& dir);
		void initPageOrder(const QString& file) const;
		void initEmptyParentalFile(const QString& file) const;
		QJsonObject getParentalRelationship() const;
		QJsonArray getPageOrder() const;
		void createPages(const QJsonArray& pages);
		bool fillDashboard(const QJsonObject& parentalRelationships);

	public:
		SaveManager* parent = nullptr;
		QPlainTextEdit* plainTextEdit;
		ModelManagerInterface* modelManager;
		Model* model;
		RewriterView* rewriterView;
		NotIndentingTextEditModifier* textModifier;
};

SaveManagerPrivate::SaveManagerPrivate(SaveManager* uparent)
	: parent(uparent)
{
	plainTextEdit = new QPlainTextEdit;
	plainTextEdit->setHidden(true);
	modelManager = new ModelManagerInterface;
	for (auto importPath : QQmlEngine().importPathList()) parseImportDirectories(importPath);
	model = Model::create("QtQuick.Item", 1, 0);
	rewriterView = new RewriterView(RewriterView::Amend, model);
	textModifier = new NotIndentingTextEditModifier(plainTextEdit);
	plainTextEdit->setPlainText("import QtQuick 2.0\nItem {id: test}");
	model->setTextModifier(textModifier);
	model->setRewriterView(rewriterView);
}

inline QString SaveManagerPrivate::generateSaveDirectory(const QString& id) const
{
	auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projectDir.isEmpty()) return projectDir;
	return projectDir + separator() + SAVE_DIRECTORY + separator() + id;
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

QJsonObject SaveManagerPrivate::getParentalRelationship() const
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return QJsonObject();
	auto parentalFile = projDir + separator() + SAVE_DIRECTORY + separator() + PARENTAL_RELATIONSHIP_FILE;
	return QJsonDocument::fromJson(rdfile(parentalFile)).object();
}

QJsonArray SaveManagerPrivate::getPageOrder() const
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return QJsonArray();
	auto pageOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + PAGE_ORDER_FILE;
	return QJsonDocument::fromJson(rdfile(pageOrderFile)).array();
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

bool SaveManagerPrivate::fillDashboard(const QJsonObject& parentalRelationships)
{
	for (auto jKey : parentalRelationships.keys()) {
		auto saveId = parent->saveDirectory(jKey);
		if (saveId.isEmpty()) return false;
		if (!MainWindow::addControlWithoutSave(QUrl::fromLocalFile(saveId + separator() + "main.qml"), parentalRelationships[jKey].toString()))
			return false;
	}
	return true;
}

SaveManagerPrivate* SaveManager::m_d = nullptr;

SaveManager::SaveManager(QObject *parent)
	: QObject(parent)
{
	if (m_d) return;
	m_d = new SaveManagerPrivate(this);
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
}

void SaveManager::changeSave(const QString& fromId, QString toId)
{
	if (!exists(fromId) || saves().contains(toId)) return;
	auto saveDir = saveDirectory(fromId);
	if (saveDir.isEmpty()) return;
	rn(saveDir, dname(saveDir) + separator() + toId);
}

void SaveManager::removeSave(const QString& id)
{
	if (!exists(id)) return;
	rm(m_d->generateSaveDirectory(id));
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
	QJsonArray pageOrder = m_d->getPageOrder();
	QJsonObject parentalRelationship = m_d->getParentalRelationship();
	if (pageOrder.isEmpty()) return false;
	if (parentalRelationship.size() != saves().size()) return false;
	m_d->createPages(pageOrder);
	return m_d->fillDashboard(parentalRelationship);
}

void SaveManager::setVariantProperty(const QString& id, const QString& property, const QVariant& value)
{
	if (saveDirectory(id).isEmpty()) return;
	auto mainQmlFilename = saveDirectory(id) + separator() + "main.qml";
	QString mainQmlContent = rdfile(mainQmlFilename);
	if (mainQmlContent.isEmpty()) return;
	m_d->plainTextEdit->setPlainText(mainQmlContent);
	ModelNode rootNode = m_d->rewriterView->rootModelNode();
	m_d->model->setFileUrl(QUrl::fromLocalFile(mainQmlFilename));
	QmlObjectNode(rootNode).setVariantProperty(QByteArray().insert(0, property), value);
	wrfile(mainQmlFilename, QByteArray().insert(0, m_d->plainTextEdit->toPlainText()));
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
	auto jObj = m_d->getParentalRelationship();
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
