#include <qmleditor.h>
class QmlEditorPrivate
{
	public:
		QmlEditor* parent;
		QmlEditorPrivate(QmlEditor* p);

};

QmlEditorPrivate::QmlEditorPrivate(QmlEditor* p)
	: parent(p)
{

	//#if !defined(Q_OS_IOS) && !defined(Q_OS_WINPHONE) && !defined(Q_OS_ANDROID)
	//	engine.rootContext()->setContextProperty("isDesktop", true);
	//#else
	//	engine.rootContext()->setContextProperty("isDesktop", false);
	//#endif
	//	engine.load(QUrl("qrc:///main.qml"));


	//	QObject *root = engine.rootObjects().at(0);
	//	QObject *textEdit = root->findChild<QObject*>(QStringLiteral("editor"));
	//	QQuickTextDocument *quickTextDocument = textEdit->property("textDocument").value<QQuickTextDocument*>();
	//	QTextDocument *document = quickTextDocument->textDocument();
	//	QTextOption textOptions = document->defaultTextOption();

	//	QFont font;
	//	font.setFamily("Liberation Mono");
	//	font.setStyleHint(QFont::Monospace);
	//	font.setFixedPitch(true);
	//	font.setPointSize(10);
	//	textEdit->setProperty("font", font);

	//	const int tabStop = 4;  // 4 characters
	//	QFontMetrics metrics(font);
	//	textOptions.setTabStop(tabStop * metrics.width(' '));
	//	document->setDefaultTextOption(textOptions);

}

QmlEditor::QmlEditor(QWidget *parent)
	: QWidget(parent)
	, m_d(new QmlEditorPrivate(this))
{

}

QmlEditor::~QmlEditor()
{
	delete m_d;
}


