#include <css.h>
#include <fit.h>
#include <propertyitem.h>
#include <switch.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QHBoxLayout>
#include <QtWidgets>
#include <savemanager.h>
#include <mainwindow.h>
#include <filemanager.h>

using namespace Fit;

PropertyItem::PropertyItem(const QPair<QMetaProperty, QObject*>& property, QQmlContext* const context, QWidget* parent)
	: QWidget(parent)
	, m_Property(property)
	, m_Valid(true)
{
	setObjectName("propertyItem"); //for css
	setStyleSheet(CSS::PropertyItem);
	fillCup(context);
}

PropertyItem::PropertyItem(QObject* const selectedItem, QQmlContext* const context, QWidget *parent)
	: QWidget(parent)
	, m_Valid(true)
{
	setObjectName("propertyItem"); //for css
	setStyleSheet(CSS::PropertyItem);
	fillId(selectedItem, context);
}

QList<QQuickItem*>* PropertyItem::itemSource() const
{
	return m_Items;
}

void PropertyItem::setItemSource(QList<QQuickItem*>* items)
{
    m_Items = items;
}

void PropertyItem::setUrlList(QList<QUrl>* urlList)
{
    m_UrlList = urlList;
}

const QPair<QMetaProperty, QObject*>& PropertyItem::property() const
{
	return m_Property;
}

void PropertyItem::applyValue(const QVariant& value, QQmlContext* const ctx)
{
	m_Property.second->setProperty(m_Property.first.name(), value);
	SaveManager::setVariantProperty(ctx->nameForObject(m_Property.second), m_Property.first.name(), value);
	emit valueApplied();
}

void PropertyItem::applyFont(const QFont& font, QQmlContext* const ctx)
{
	auto id = ctx->nameForObject(m_Property.second);
	m_Property.second->setProperty(m_Property.first.name(), font);
	SaveManager::setVariantProperty(id, "font.family", font.family());
	SaveManager::setVariantProperty(id, "font.bold", font.bold());
	SaveManager::setVariantProperty(id, "font.italic", font.italic());
	SaveManager::setVariantProperty(id, "font.pointSize", font.pointSize());
//	emit valueApplied();
}

void PropertyItem::applyId(const QString& id, QObject* const selectedItem, QQmlContext* const context)
{
	int count = 1;
	QString componentName = id;
	for (int i=0; i<m_Items->size();i++) {
		if (componentName == QString(context->nameForObject((QObject*)(m_Items->at(i)))) ||
			componentName == QString("dpi") || componentName == QString("swipeView")) {
			if (componentName.at(componentName.size() - 1).isNumber()) {
				componentName.remove(componentName.size() - 1, 1);
			}
			componentName += QString::number(count);
			count++;
			i = -1;
		}
	}

	auto items = MainWindow::GetAllChildren((QQuickItem*)selectedItem);
	for (auto item : items) {
		auto ctxId = context->nameForObject((QObject*)item);
		if (item == (QQuickItem*)selectedItem) {
			auto prevParent = SaveManager::parentalRelationship(ctxId);
			SaveManager::removeParentalRelationship(ctxId);
			SaveManager::addParentalRelationship(componentName, prevParent);
			SaveManager::changeSave(ctxId, componentName);
            if (m_Items->indexOf(item) >= 0) {
                auto oldUrl = (*m_UrlList)[m_Items->indexOf(item)].toLocalFile();
                auto newUrl = dname(dname(oldUrl)) + separator() + componentName + separator() + "main.qml";
                (*m_UrlList)[m_Items->indexOf(item)] = QUrl::fromLocalFile(newUrl);
            }
		} else if (m_Items->contains(item)) {
			SaveManager::addParentalRelationship(ctxId, componentName);
		}
	}
	context->setContextProperty(context->nameForObject(selectedItem), 0);
	context->setContextProperty(componentName, selectedItem);
    SaveManager::setId(componentName, componentName);
}

void PropertyItem::fillCup(QQmlContext* const context)
{
	QString name = m_Property.first.name();
	name[0] = name.at(0).toUpper();

	QFont font;
	font.setPixelSize(fit(13));

	QLabel* label = new QLabel;
	label->setText(name + ":");
	label->setFont(font);
	label->setStyleSheet("color:white;");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(fit(5), fit(5), fit(7), fit(10));
	layout->addWidget(label);

	switch (int(m_Property.first.type())) {

		case QVariant::Bool: {
			Switch* widget = new Switch;
			widget->setFont(font);
			widget->setFixedSize(fit(widget->sizeHint()));
			widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			widget->setChecked(m_Property.first.read(m_Property.second).toBool());
			layout->addWidget(widget);
			connect(widget,static_cast<void(Switch::*)(bool)>(&Switch::toggled),[&, context](bool b){applyValue(b, context);});
			break;

		} case QVariant::String: {
			QLineEdit* widget = new QLineEdit;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setStyleSheet(CSS::LineEdit);
			widget->setFixedHeight(fit(30));
			widget->setText(m_Property.first.read(m_Property.second).toString());
			layout->addWidget(widget);
			connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[&, context](const QString& b){applyValue(b, context);});

			widget->setReadOnly(true);
			QCheckBox* enabler = new QCheckBox;
			enabler->setCursor(Qt::PointingHandCursor);
			connect(enabler, static_cast<void(QCheckBox::*)(int)>(&QCheckBox::stateChanged),[=](int checked){
				if (checked == Qt::Unchecked) {
					widget->setReadOnly(true);
				} else if (checked == Qt::Checked) {
					widget->setReadOnly(false);
				}
			});
			QHBoxLayout* lay = new QHBoxLayout;
			lay->setSpacing(fit(5));
			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(widget);
			lay->addWidget(enabler);
			layout->addLayout(lay);
			break;

		} case QVariant::Int: {
			QSpinBox* widget = new QSpinBox;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setStyleSheet(CSS::SpinBox);
			widget->setAlignment(Qt::AlignCenter);
			widget->setFixedHeight(fit(30));
			widget->setRange(-9999,9999);
			widget->setAccelerated(true);
			widget->installEventFilter(this);
			widget->setCursor(Qt::PointingHandCursor);
			widget->setValue(m_Property.first.read(m_Property.second).toInt());
			layout->addWidget(widget);
			connect(widget,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[&, context](int b){applyValue(b, context);});

			widget->findChild<QLineEdit*>()->setReadOnly(true);
			QCheckBox* enabler = new QCheckBox;
			enabler->setCursor(Qt::PointingHandCursor);
			connect(enabler, static_cast<void(QCheckBox::*)(int)>(&QCheckBox::stateChanged),[=](int checked){
				if (checked == Qt::Unchecked) {
					widget->findChild<QLineEdit*>()->setReadOnly(true);
				} else if (checked == Qt::Checked) {
					widget->findChild<QLineEdit*>()->setReadOnly(false);
				}
			});
			QHBoxLayout* lay = new QHBoxLayout;
			lay->setSpacing(fit(5));
			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(widget);
			lay->addWidget(enabler);
			layout->addLayout(lay);
			break;

		} case QVariant::Double: {
			QDoubleSpinBox* widget = new QDoubleSpinBox;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setStyleSheet(CSS::SpinBox);
			widget->setAlignment(Qt::AlignCenter);
			widget->setFixedHeight(fit(30));
			widget->setSingleStep(0.1);
			widget->setRange(-9999,9999);
			widget->setAccelerated(true);
			widget->setCursor(Qt::PointingHandCursor);
			widget->installEventFilter(this);
			widget->setValue(m_Property.first.read(m_Property.second).toDouble());
			connect(widget,static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[&, context](double b){applyValue(b, context);});

			widget->findChild<QLineEdit*>()->setReadOnly(true);
			QCheckBox* enabler = new QCheckBox;
			enabler->setCursor(Qt::PointingHandCursor);
			connect(enabler, static_cast<void(QCheckBox::*)(int)>(&QCheckBox::stateChanged),[=](int checked){
				if (checked == Qt::Unchecked) {
					widget->findChild<QLineEdit*>()->setReadOnly(true);
				} else if (checked == Qt::Checked) {
					widget->findChild<QLineEdit*>()->setReadOnly(false);
				}
			});
			QHBoxLayout* lay = new QHBoxLayout;
			lay->setSpacing(fit(5));
			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(widget);
			lay->addWidget(enabler);
			layout->addLayout(lay);
			break;

		} case QVariant::Font: {
			QFontComboBox* widget = new QFontComboBox;
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setFont(font);
			widget->setCurrentFont(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)));
			widget->installEventFilter(this);
			widget->setFixedHeight(fit(30));
			widget->setCursor(Qt::PointingHandCursor);
			widget->findChild<QLineEdit*>()->setReadOnly(true);
			connect(widget,static_cast<void(QFontComboBox::*)(const QFont&)>(&QFontComboBox::currentFontChanged),
					[&, context](const QFont& b){applyFont(b, context);});

			QSpinBox* size = new QSpinBox;
			size->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			size->setFont(font);
			size->installEventFilter(this);
			size->setFixedHeight(fit(30));
			size->setStyleSheet(CSS::SpinBox);
			size->setRange(6, 72);
			size->setCursor(Qt::PointingHandCursor);
			size->setAlignment(Qt::AlignCenter);
			size->findChild<QLineEdit*>()->setReadOnly(true);
			size->setValue(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)).pointSize());
			connect(size,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=](int b){
				QFont f(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)));
				f.setPointSize(b);
				applyFont(f, context);
			});

			QComboBox* weight = new QComboBox;
			weight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			weight->setFont(font);
			weight->installEventFilter(this);
			weight->setFixedHeight(fit(30));
			weight->setEditable(false);
			weight->setCursor(Qt::PointingHandCursor);
			weight->addItem("Bold");
			weight->addItem("Regular");
			weight->addItem("Bold Italic");
			weight->addItem("Regular Italic");
			QFont fnt(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)));
			if (fnt.bold() && !fnt.italic()) {
				weight->setCurrentIndex(0);
			} else if (!fnt.bold() && !fnt.italic()) {
				weight->setCurrentIndex(1);
			} else if (fnt.bold() && fnt.italic()) {
				weight->setCurrentIndex(2);
			} else if (!fnt.bold() && fnt.italic()) {
				weight->setCurrentIndex(3);
			}
			connect(weight, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
				QFont f(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)));
				if (index == 0) {
					f.setBold(true);
					f.setItalic(false);
				} else if (index == 1) {
					f.setBold(false);
					f.setItalic(false);
				} else if (index == 2) {
					f.setBold(true);
					f.setItalic(true);
				} else if (index == 3) {
					f.setBold(false);
					f.setItalic(true);
				}
				applyFont(f, context);
			});

			layout->addWidget(widget);
			layout->addWidget(weight);
			layout->addWidget(size);
			layout->setSpacing(fit(5));
			break;

		} default: {
			m_Valid = false;
			break;
		}
	}
}

void PropertyItem::fillId(QObject* const selectedItem, QQmlContext* const context)
{
	QFont font;
	font.setPixelSize(fit(13));

	QLabel* label = new QLabel;
	label->setText("Id:");
	label->setFont(font);
	label->setStyleSheet("color:white;");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(fit(5), fit(5), fit(7), fit(10));
	layout->addWidget(label);

	QLineEdit* widget = new QLineEdit;
	widget->setFont(font);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	widget->setStyleSheet(CSS::LineEdit);
	widget->setFixedHeight(fit(30));
	widget->setText(context->nameForObject(selectedItem));
	layout->addWidget(widget);
	connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[=](const QString& b){applyId(b,selectedItem,context);});

	widget->setReadOnly(true);
	QCheckBox* enabler = new QCheckBox;
	enabler->setCursor(Qt::PointingHandCursor);
	connect(enabler, static_cast<void(QCheckBox::*)(int)>(&QCheckBox::stateChanged),[=](int checked){
		if (checked == Qt::Unchecked) {
			widget->setReadOnly(true);
		} else if (checked == Qt::Checked) {
			widget->setReadOnly(false);
		}
	});
	QHBoxLayout* lay = new QHBoxLayout;
	lay->setSpacing(fit(5));
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(widget);
	lay->addWidget(enabler);
	layout->addLayout(lay);
}

bool PropertyItem::eventFilter( QObject * o, QEvent * e )
{
	if ( e->type() == QEvent::Wheel) {
		e->ignore();
		return true;
	}
	return QWidget::eventFilter(o, e);
}

void PropertyItem::paintEvent(QPaintEvent* e)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(e);
}
