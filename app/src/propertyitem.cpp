#include <css.h>
#include <fit.h>
#include <propertyitem.h>
#include <switch.h>

#include <QHBoxLayout>
#include <QtWidgets>

using namespace Fit;

PropertyItem::PropertyItem(const QPair<QMetaProperty, QObject*>& property, QWidget* parent)
	: QWidget(parent)
	, m_Property(property)
{
	setObjectName("propertyItem");
	setStyleSheet(CSS::PropertyItem);
	fillCup();
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

void PropertyItem::fillCup()
{
	QString name = m_Property.first.name();
	name[0] = name.at(0).toUpper();

	QFont font;
	font.setPixelSize(fit(13));

	QWidget* valueWidget = nullptr;
	switch (int(m_Property.first.type())) {
		case QVariant::Bool: {
			Switch* widget = new Switch;
			widget->setFont(font);
			widget->setFixedSize(fit(widget->sizeHint()));
			widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			widget->setChecked(m_Property.first.read(m_Property.second).toBool());
			valueWidget = widget;
			connect(widget,static_cast<void(Switch::*)(bool)>(&Switch::toggled),[&](bool b){applyValue(b);});
			break;

		} case QVariant::String: {
			QLineEdit* widget = new QLineEdit;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setStyleSheet(CSS::LineEdit);
			widget->setFixedHeight(fit(30));
			widget->setText(m_Property.first.read(m_Property.second).toString());
			valueWidget = widget;
			connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[&](const QString& b){applyValue(b);});
			break;

		} case QVariant::Int: {
			QSpinBox* widget = new QSpinBox;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			// widget->findChild<QLineEdit*>()->setReadOnly(true);
			widget->setStyleSheet(CSS::SpinBox);
			widget->setAlignment(Qt::AlignCenter);
			widget->setFixedHeight(fit(30));
			widget->setRange(-9999,9999);
			widget->setAccelerated(true);
			widget->installEventFilter(this);
			widget->setValue(m_Property.first.read(m_Property.second).toInt());
			valueWidget = widget;
			connect(widget,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[&](int b){applyValue(b);});
			break;

		} case QVariant::Double: {
			QDoubleSpinBox* widget = new QDoubleSpinBox;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			// widget->findChild<QLineEdit*>()->setReadOnly(true);
			widget->setStyleSheet(CSS::SpinBox);
			widget->setAlignment(Qt::AlignCenter);
			widget->setFixedHeight(fit(30));
			widget->setSingleStep(0.1);
			widget->setRange(-9999,9999);
			widget->setAccelerated(true);
			widget->installEventFilter(this);
			widget->setValue(m_Property.first.read(m_Property.second).toDouble());
			valueWidget = widget;
			connect(widget,static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[&](double b){applyValue(b);});
			break;

		} case QVariant::Font: {
			QFontComboBox* widget = new QFontComboBox;
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setFont(font);
			widget->setCurrentText(m_Property.first.read(m_Property.second).toString());
			valueWidget = widget;
			break;

		} default: {
			break;
		}
	}

	if (nullptr == valueWidget) {
		return;
	}

	QLabel* label = new QLabel;
	label->setText(name + ":");
	label->setFont(font);
	label->setStyleSheet("color:white;");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(fit(5), fit(5), fit(7), fit(10));
	layout->addWidget(label);
	layout->addWidget(valueWidget);
	m_Valid = true;
}

bool PropertyItem::eventFilter( QObject * o, QEvent * e )
{
	if ( e->type() == QEvent::Wheel && qobject_cast<QAbstractSpinBox*>(o)) {
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
