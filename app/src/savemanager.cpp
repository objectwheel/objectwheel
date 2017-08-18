#include <savemanager.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <parsercontroller.h>
#include <formswidget.h>
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

class SaveManagerPrivate : public QObject
{
        Q_OBJECT

	public:
        SaveManagerPrivate(SaveManager* parent);
        QString generateSavesDirectory() const;
		QString generateSaveDirectory(const QString& id) const;

	public:
        SaveManager* parent = nullptr;
        ParserController parserController;
};

SaveManagerPrivate::SaveManagerPrivate(SaveManager* parent)
    : QObject(parent)
    , parent(parent)
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
    if (!mkfile(saveBaseDir + separator() + FORM_ORDER_FILE)) return false;
    m_d->initFormOrder(saveBaseDir + separator() + FORM_ORDER_FILE);
	m_d->initEmptyParentalFile(saveBaseDir + separator() + PARENTAL_RELATIONSHIP_FILE);
	return true;
}

bool SaveManager::loadDatabase()
{
    QJsonArray formOrder = getFormOrders();
	QJsonObject parentalRelationship = getParentalRelationships();
	QJsonObject bindingSaves = getBindingSaves();
    QJsonObject eventSaves = getEventSaves();
    if (formOrder.isEmpty()) return false;
	if (parentalRelationship.size() != saves().size()) return false;
    m_d->createForms(formOrder);
    if (!m_d->fillDashboard(parentalRelationship, formOrder)) return false;
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

void SaveManager::addFormOrder(const QString& formId)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
    auto formOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + FORM_ORDER_FILE;
    QJsonArray jArr = QJsonDocument::fromJson(rdfile(formOrderFile)).array();
    jArr.append(formId);
    wrfile(formOrderFile, QJsonDocument(jArr).toJson());
}

void SaveManager::removeFormOrder(const QString& formId)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
    auto formOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + FORM_ORDER_FILE;
    QJsonArray jArr = QJsonDocument::fromJson(rdfile(formOrderFile)).array();
	for (int i = 0; i < jArr.size(); i++) {
        if (jArr.at(i) == formId) {
			jArr.removeAt(i);
            wrfile(formOrderFile, QJsonDocument(jArr).toJson());
			break;
		}
	}
}

void SaveManager::changeFormOrder(const QString& fromFormId, const QString& toFormId)
{
	auto projDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projDir.isEmpty()) return;
    auto formOrderFile = projDir + separator() + SAVE_DIRECTORY + separator() + FORM_ORDER_FILE;
    QJsonArray jArr = QJsonDocument::fromJson(rdfile(formOrderFile)).array();
	for (int i = 0; i < jArr.size(); i++) {
        if (jArr.at(i) == fromFormId) {
            jArr[i] = toFormId;
            wrfile(formOrderFile, QJsonDocument(jArr).toJson());
			break;
		}
    }
}
