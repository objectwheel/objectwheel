#include <savemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <parsercontroller.h>
#include <pageswidget.h>
#include <mainwindow.h>
#include <bindingwidget.h>
#include <eventswidget.h>
#include <qmleditor.h>
#include <eventswidget.h>
#include <bindingwidget.h>

#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QTimer>

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
        void fillEvents(const QJsonObject& eventSaves);
        void removeSave(const QString& id);

	public:
        SaveManager* parent = nullptr;
        ParserController parserController;
};

SaveManagerPrivate::SaveManagerPrivate(SaveManager* uparent)
    : parent(uparent)
{
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
//	auto firstPage = pages[0].toString();
//	PagesWidget::changePageWithoutSave("page1", firstPage);
//	for (int i = 1; i < pages.size(); i++) {
//		auto currPage = pages[i].toString();
//		PagesWidget::addPageWithoutSave(currPage);
//	}
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

void SaveManagerPrivate::fillEvents(const QJsonObject& eventSaves)
{
    SaveManager::EventInf inf;
    for (auto eventKey : eventSaves.keys()) {
        inf.eventName = eventKey;
        inf.targetId = eventSaves[eventKey].toObject()[EVENT_TARGET_ID_LABEL].toString();
        inf.targetEventname = eventSaves[eventKey].toObject()[EVENT_TARGET_EVENTNAME_LABEL].toString();
        inf.eventCode = QString(QByteArray::fromBase64(QByteArray().insert(0, eventSaves[eventKey].toObject()[EVENT_EVENT_CODE_LABEL].toString())));
        EventsWidget::addEventWithoutSave(inf);
    }
}

void SaveManagerPrivate::removeSave(const QString& id)
{
    if (!parent->exists(id)) return;
    rm(generateSaveDirectory(id));
    SaveManager::removeParentalRelationship(id);
    BindingWidget::instance()->detachBindingsFor(id);
    EventsWidget::instance()->detachEventsFor(id);
    QmlEditor::clearCacheFor(parent->saveDirectory(id), true);
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
	QJsonObject jObj = QJsonDocument::fromJson(rdfile(bindingFile)).object();
	jObj.remove(bindingName);
    wrfile(bindingFile, QJsonDocument(jObj).toJson());
}

QJsonObject SaveManager::getEventSaves()
{
    auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
    if (projDir.isEmpty()) return QJsonObject();
    auto eventsFile = projDir + separator() + SAVE_DIRECTORY + separator() + EVENTS_FILE;
    return QJsonDocument::fromJson(rdfile(eventsFile)).object();
}

void SaveManager::addEventSave(const SaveManager::EventInf& eventInf)
{
    auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
    if (projDir.isEmpty()) return;
    auto eventFile = projDir + separator() + SAVE_DIRECTORY + separator() + EVENTS_FILE;
    QJsonObject cObj;
    cObj[EVENT_TARGET_ID_LABEL] = eventInf.targetId;
    cObj[EVENT_TARGET_EVENTNAME_LABEL] = eventInf.targetEventname;
    cObj[EVENT_EVENT_CODE_LABEL] = QString(QByteArray().insert(0, eventInf.eventCode).toBase64());
    QJsonObject pObj = QJsonDocument::fromJson(rdfile(eventFile)).object();
    pObj[eventInf.eventName] = cObj;
    wrfile(eventFile, QJsonDocument(pObj).toJson());
}

void SaveManager::changeEventSave(const QString& eventName, const SaveManager::EventInf& toEventInf)
{
    removeEventSave(eventName);
    addEventSave(toEventInf);
}

void SaveManager::removeEventSave(const QString& eventName)
{
    auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
    if (projDir.isEmpty()) return;
    auto eventFile = projDir + separator() + SAVE_DIRECTORY + separator() + EVENTS_FILE;
    QJsonObject jObj = QJsonDocument::fromJson(rdfile(eventFile)).object();
    jObj.remove(eventName);
    wrfile(eventFile, QJsonDocument(jObj).toJson());
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

QStringList SaveManager::childSaves(const QString& id)
{
    QStringList childSaves;
    for (auto save : saves()) {
        if (parentalRelationship(save) == id) {
            childSaves << save;
            childSaves << SaveManager::childSaves(save);
        }
    }
    return childSaves;
}

void SaveManager::addSave(const QString& id, const QString& url)
{
    if (exists(id)  || url.isEmpty()) return;
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

    for (auto save : childSaves(fromId))
        if (parentalRelationship(save) == fromId)
            SaveManager::addParentalRelationship(save, toId);

    auto prevParent = parentalRelationship(fromId);
    if (!prevParent.isEmpty()) {
        SaveManager::removeParentalRelationship(fromId);
        SaveManager::addParentalRelationship(toId, prevParent);
    }

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

    SaveManager::EventInf einf;
    auto eventSaves = getEventSaves();
    for (auto eventKey : eventSaves.keys()) {
        einf.eventName = eventKey;
        einf.targetId = eventSaves[eventKey].toObject()[EVENT_TARGET_ID_LABEL].toString();
        einf.targetEventname = eventSaves[eventKey].toObject()[EVENT_TARGET_EVENTNAME_LABEL].toString();
        einf.eventCode = QString(QByteArray::fromBase64(QByteArray().insert(0, eventSaves[eventKey].toObject()[EVENT_EVENT_CODE_LABEL].toString())));

        if (einf.targetId == fromId) {
            einf.targetId = toId;
        } else {
            continue;
        }
        changeEventSave(einf.eventName, einf);
    }

   QmlEditor::updateCacheForRenamedEntry(saveDir, dname(saveDir) + separator() + toId, true);
}

void SaveManager::removeSave(const QString& id)
{
    m_d->removeSave(id);
    removeChildSavesOnly(id);
}

void SaveManager::removeChildSavesOnly(const QString& id)
{
    for (auto save : childSaves(id))
        removeSave(save);
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
    QJsonObject eventSaves = getEventSaves();
	if (pageOrder.isEmpty()) return false;
	if (parentalRelationship.size() != saves().size()) return false;
	m_d->createPages(pageOrder);
	if (!m_d->fillDashboard(parentalRelationship, pageOrder)) return false;
    m_d->fillBindings(bindingSaves);
    m_d->fillEvents(eventSaves);
    return true;
}

void SaveManager::setVariantProperty(const QString& id, const QString& property, const QVariant& value)
{
    if (saveDirectory(id).isEmpty()) return;
    auto filename = saveDirectory(id) + separator() + "main.qml";
    ParserController::setVariantProperty(filename, property, value);
    QmlEditor::clearCacheFor(saveDirectory(id), true);
}

void SaveManager::removeVariantProperty(const QString& id, const QString& property) //FIXME: FOR BINDING PROPERTIES
{
    if (saveDirectory(id).isEmpty()) return;
    auto filename = saveDirectory(id) + separator() + "main.qml";
    ParserController::removeVariantProperty(filename, property);
}

bool SaveManager::inprogress()
{
    return m_d->parserController.running();
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
