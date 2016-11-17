#include <propertyitem.h>
#include <css.h>
#include <fitter.h>
#include <QHBoxLayout>
#include <QtWidgets>

PropertyItem::PropertyItem(QWidget *parent)
	: PropertyItem(QPair<QMetaProperty, QObject*>(), parent)
{}

PropertyItem::PropertyItem(const QPair<QMetaProperty, QObject*>& property, QWidget* parent)
	: QWidget(parent)
	, m_Property(property)
{
	fillCup();
}

const QPair<QMetaProperty, QObject*>& PropertyItem::property() const
{
	return m_Property;
}

void PropertyItem::setProperty(const QPair<QMetaProperty, QObject*>& property)
{
	m_Property = property;
	fillCup();
}

void PropertyItem::applyValue(const QVariant& value)
{
	m_Property.second->setProperty(m_Property.first.name(), value);
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

	QWidget* valueWidget;

	switch (int(m_Property.first.type())) {
		case QVariant::Bool: {
			QCheckBox* widget = new QCheckBox;
			widget->setChecked(m_Property.first.read(m_Property.second).toBool());
			widget->setFont(font);
			valueWidget = widget;
			connect(widget,static_cast<void(QCheckBox::*)(bool)>(&QCheckBox::toggled),[&](bool b){applyValue(b);});
			break;

		} case QVariant::String: {
			QLineEdit* widget = new QLineEdit;
			widget->setText(m_Property.first.read(m_Property.second).toString());
			widget->setFont(font);
			valueWidget = widget;
			connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[&](const QString& b){applyValue(b);});
			break;

		} case QVariant::Int: {
			QSpinBox* widget = new QSpinBox;
			widget->setValue(m_Property.first.read(m_Property.second).toInt());
			widget->setFont(font);
			widget->findChild<QLineEdit*>()->setReadOnly(true);
			widget->setStyleSheet(CSS::SpinBox);
			widget->setAlignment(Qt::AlignCenter);
			widget->setFixedSize(fit(120), fit(30));
			widget->setRange(-1000,1000);
			valueWidget = widget;
			connect(widget,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[&](int b){applyValue(b);});
			break;

		} case QVariant::Double: {
			QDoubleSpinBox* widget = new QDoubleSpinBox;
			widget->setValue(m_Property.first.read(m_Property.second).toDouble());
			widget->setFont(font);
			widget->findChild<QLineEdit*>()->setReadOnly(true);
			widget->setStyleSheet(CSS::SpinBox);
			widget->setAlignment(Qt::AlignCenter);
			widget->setFixedSize(fit(120), fit(30));
			widget->setSingleStep(0.1);
			widget->setRange(-1000,1000);
			valueWidget = widget;
			connect(widget,static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[&](double b){applyValue(b);});
			break;

		} case QVariant::Font: {
			QFontComboBox* widget = new QFontComboBox;
			widget->setCurrentText(m_Property.first.read(m_Property.second).toString());
			widget->setFont(font);
			valueWidget = widget;
			break;

		} default: {
//			QLineEdit* widget = new QLineEdit;
//			widget->setText(m_Property.first.read(m_Property.second).toString());
//			widget->setFont(font);
//			valueWidget = widget;
			 valueWidget = new QWidget;
			break;
		}
	}

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, fit(15));

	layout->addWidget(label);
	layout->addWidget(valueWidget);
}
