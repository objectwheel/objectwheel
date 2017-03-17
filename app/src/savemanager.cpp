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

#define SAVE_DIRECTORY "dashboard"

using namespace QmlDesigner;
using namespace QmlJS;

class SaveManagerPrivate
{
	public:
		SaveManagerPrivate(SaveManager* uparent);
		QString generateSaveDirectory(const QString& id) const;
		void parseImportDirectories(const QString& dir);

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
	qDebug() << dir;
	modelManager->updateLibraryInfo(dir, LibraryInfo(LibraryInfo::Found));
	for (auto subdir: lsdir(dir)) parseImportDirectories(dir + separator() + subdir);
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

void SaveManager::addSave(const QString& id, const QString& url)
{
	if (exists(id)) return;
	auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projectDir.isEmpty()) return;
	auto saveBaseDir = projectDir + separator() + SAVE_DIRECTORY;
	if (!mkdir(saveBaseDir)) return;
	cp(dname(url), saveBaseDir);
	rn(saveBaseDir + separator() + fname(dname(url)), saveBaseDir + separator() + id);
}

void SaveManager::removeSave(const QString& id)
{
	if (!exists(id)) return;
	rm(m_d->generateSaveDirectory(id));
}

QStringList SaveManager::saves()
{
	QStringList saveList;
	auto projectDir = ProjectManager::projectDirectory(ProjectManager::currentProject());
	if (projectDir.isEmpty()) return saveList;
	for (auto savename : lsdir(projectDir)) saveList << savename;
	return saveList;
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
