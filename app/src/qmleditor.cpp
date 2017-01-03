#include <qmleditor.h>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QQuickTextDocument>
#include <QTextDocument>
#include <QTextOption>
#include <flatbutton.h>
#include <fit.h>

using namespace Fit;

class QmlEditorPrivate
{
	public:
		QmlEditor* parent;
		QVBoxLayout mainLayout;
		QQuickWidget quickWidget;
		FlatButton minimizeButton;

		QmlEditorPrivate(QmlEditor* p);
		void resize();
};

QmlEditorPrivate::QmlEditorPrivate(QmlEditor* p)
	: parent(p)
{
	parent->setLayout(&mainLayout);
	mainLayout.addWidget(&quickWidget);
	mainLayout.setSpacing(0);
	mainLayout.setContentsMargins(0, 0, 0, 0);

#if !defined(Q_OS_IOS) && !defined(Q_OS_WINPHONE) && !defined(Q_OS_ANDROID)
	quickWidget.rootContext()->setContextProperty("isDesktop", true);
#else
	quickWidget.rootContext()->setContextProperty("isDesktop", false);
#endif

	quickWidget.setSource(QUrl("qrc:/resources/qmls/qml-editor.qml"));
	quickWidget.setResizeMode(QQuickWidget::SizeRootObjectToView);
	quickWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QObject *root = quickWidget.rootObject();
	QObject::connect(root, SIGNAL(saved(QString)), parent, SLOT(hide()));

	QObject *textEdit = root->findChild<QObject*>(QStringLiteral("editor"));
	QQuickTextDocument *quickTextDocument = textEdit->property("textDocument").value<QQuickTextDocument*>();
	QTextDocument *document = quickTextDocument->textDocument();
	QTextOption textOptions = document->defaultTextOption();

	QFont font;
	font.setFamily("Liberation Mono");
	font.setStyleHint(QFont::Monospace);
	font.setFixedPitch(true);
	font.setPixelSize(fit(12));
	textEdit->setProperty("font", font);

	const int tabStop = 4;  // 4 characters
	QFontMetrics metrics(font);
	textOptions.setTabStop(tabStop * metrics.width(' '));
	document->setDefaultTextOption(textOptions);

	minimizeButton.setParent(parent);
	minimizeButton.setIconButton(true);
	minimizeButton.setIcon(QIcon(":/resources/images/mask.png"));
	minimizeButton.resize(fit(30), fit(30));
	QObject::connect(&minimizeButton, SIGNAL(clicked(bool)), parent, SLOT(hide()));

}

void QmlEditorPrivate::resize()
{
	minimizeButton.move(fit(10), parent->height() - minimizeButton.height() - fit(2));
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

void QmlEditor::resizeEvent(QResizeEvent* event)
{
	m_d->resize();
	QWidget::resizeEvent(event);
}


