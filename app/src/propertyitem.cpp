#include <css.h>
#include <fit.h>
#include <propertyitem.h>
#include <switch.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QHBoxLayout>
#include <QtWidgets>

using namespace Fit;

PropertyItem::PropertyItem(const QPair<QMetaProperty, QObject*>& property, QWidget* parent)
	: QWidget(parent)
	, m_Property(property)
	, m_Valid(true)
{
	setObjectName("propertyItem"); //for css
	setStyleSheet(CSS::PropertyItem);
	fillCup();
}

PropertyItem::PropertyItem(QObject* const selectedItem, QQmlContext* const context, QWidget *parent)
	: QWidget(parent)
	, m_Valid(true)
{
	setObjectName("propertyItem"); //for css
	setStyleSheet(CSS::PropertyItem);
	fillId(selectedItem, context);
}

const QPair<QMetaProperty, QObject*>& PropertyItem::property() const
{
	return m_Property;
}

void PropertyItem::applyValue(const QVariant& value)
{
	m_Property.second->setProperty(m_Property.first.name(), value);
	emit valueApplied();
}

void PropertyItem::applyId(const QString& id, QObject* const selectedItem, QQmlContext* const context)
{
	context->setContextProperty(id, selectedItem);
}

void PropertyItem::fillCup()
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
			connect(widget,static_cast<void(Switch::*)(bool)>(&Switch::toggled),[&](bool b){applyValue(b);});
			break;

		} case QVariant::String: {
			QLineEdit* widget = new QLineEdit;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setStyleSheet(CSS::LineEdit);
			widget->setFixedHeight(fit(30));
			widget->setText(m_Property.first.read(m_Property.second).toString());
			layout->addWidget(widget);
			connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[&](const QString& b){applyValue(b);});

			widget->setReadOnly(true);
			QCheckBox* enabler = new QCheckBox;
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
			widget->setValue(m_Property.first.read(m_Property.second).toInt());
			layout->addWidget(widget);
			connect(widget,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[&](int b){applyValue(b);});

			widget->findChild<QLineEdit*>()->setReadOnly(true);
			QCheckBox* enabler = new QCheckBox;
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
			widget->installEventFilter(this);
			widget->setValue(m_Property.first.read(m_Property.second).toDouble());
			connect(widget,static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[&](double b){applyValue(b);});

			widget->findChild<QLineEdit*>()->setReadOnly(true);
			QCheckBox* enabler = new QCheckBox;
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
			widget->findChild<QLineEdit*>()->setReadOnly(true);
			connect(widget,static_cast<void(QFontComboBox::*)(const QFont&)>(&QFontComboBox::currentFontChanged),
					[&](const QFont& b){applyValue(b);});

			QSpinBox* size = new QSpinBox;
			size->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			size->setFont(font);
			size->installEventFilter(this);
			size->setFixedHeight(fit(30));
			size->setStyleSheet(CSS::SpinBox);
			size->setRange(6, 72);
			size->setAlignment(Qt::AlignCenter);
			size->findChild<QLineEdit*>()->setReadOnly(true);
			size->setValue(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)).pointSize());
			connect(size,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=](int b){
				QFont f(qvariant_cast<QFont>(m_Property.first.read(m_Property.second)));
				f.setPointSize(b);
				m_Property.second->setProperty("font", f);
			});

			QComboBox* weight = new QComboBox;
			weight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			weight->setFont(font);
			weight->installEventFilter(this);
			weight->setFixedHeight(fit(30));
			weight->setEditable(false);
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
				m_Property.second->setProperty("font", f);
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
	widget->setText(qmlContext(selectedItem)->nameForObject(selectedItem));
	layout->addWidget(widget);
	connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[=](const QString& b){applyId(b,selectedItem,context);});

	widget->setReadOnly(true);
	QCheckBox* enabler = new QCheckBox;
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
