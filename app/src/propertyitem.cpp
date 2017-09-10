#include <css.h>
#include <fit.h>
#include <delayer.h>
#include <propertyitem.h>
#include <switch.h>
#include <savemanager.h>
#include <filemanager.h>
#include <control.h>
#include <designmanager.h>

#include <QtWidgets>

using namespace Fit;

PropertyItem::PropertyItem(Control* control, const QString& property, QWidget* parent)
    : QWidget(parent)
    , _control(control)
    , _property(property)
    , _valid(true)
{
    setObjectName("propertyItem"); //for css
	setStyleSheet(CSS::PropertyItem);
    fillCup();
}

Control* PropertyItem::control() const
{
    return _control.data();
}

QString PropertyItem::property() const
{
    return _property;
}

void PropertyItem::applyValue(const QVariant& value)
{
    if (DesignManager::mode() == DesignManager::ControlGUI && _property == TAG_ID)
        SaveManager::setProperty(_control, _property, value, DesignManager::controlScene()->mainControl()->dir());
    else
        SaveManager::setProperty(_control, _property, value);
    Delayer::delay(&SaveManager::inprogress);
    _control->refresh();
	emit valueApplied();
}

void PropertyItem::applyFont(const QFont& font)
{
    SaveManager::setProperty(_control, "font.family", font.family());
    SaveManager::setProperty(_control, "font.bold", font.bold());
    SaveManager::setProperty(_control, "font.italic", font.italic());
    SaveManager::setProperty(_control, "font.pointSize", font.pointSize());
    Delayer::delay(&SaveManager::inprogress);
    _control->refresh();
    emit valueApplied();
}

void PropertyItem::fillCup()
{
    QString name = _property;
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

    if (!_control->properties().contains(_property)) {
        _valid = false;
        return;
    }

    if (_property == TAG_ID && _control == DesignManager::controlScene()->mainControl()) {
        _valid = false;
        return;
    }

    auto property = _control->properties()[_property];
    switch (int(property.type())) {
		case QVariant::Bool: {
			Switch* widget = new Switch;
			widget->setFont(font);
			widget->setFixedSize(fit(widget->sizeHint()));
			widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            widget->setChecked(property.toBool());
			layout->addWidget(widget);
            connect(widget,static_cast<void(Switch::*)(bool)>(&Switch::toggled),[&](bool b){applyValue(b);});
			break;

		} case QVariant::String: {
			QLineEdit* widget = new QLineEdit;
			widget->setFont(font);
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setStyleSheet(CSS::LineEdit);
			widget->setFixedHeight(fit(30));
            widget->setText(property.toString());
			layout->addWidget(widget);
            connect(widget,static_cast<void(QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),[&](const QString& b){applyValue(b);});

			QHBoxLayout* lay = new QHBoxLayout;
			lay->setSpacing(fit(5));
			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(widget);
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
            widget->setValue(property.toInt());
			layout->addWidget(widget);
            connect(widget,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[&](int b){applyValue(b);});


			QHBoxLayout* lay = new QHBoxLayout;
			lay->setSpacing(fit(5));
			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(widget);
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
            widget->setValue(property.toDouble());
            connect(widget,static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[&](double b){applyValue(b);});

			QHBoxLayout* lay = new QHBoxLayout;
			lay->setSpacing(fit(5));
			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(widget);
            layout->addLayout(lay);
			break;

		} case QVariant::Font: {
			QFontComboBox* widget = new QFontComboBox;
			widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			widget->setFont(font);
            widget->setCurrentFont(qvariant_cast<QFont>(property));
			widget->installEventFilter(this);
			widget->setFixedHeight(fit(30));
			widget->setCursor(Qt::PointingHandCursor);
			widget->findChild<QLineEdit*>()->setReadOnly(true);
			connect(widget,static_cast<void(QFontComboBox::*)(const QFont&)>(&QFontComboBox::currentFontChanged),
                    [&](const QFont& b){applyFont(b);});

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
            size->setValue(qvariant_cast<QFont>(property).pointSize());
			connect(size,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=](int b){
                QFont f(qvariant_cast<QFont>(property));
				f.setPointSize(b);
                applyFont(f);
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
            QFont fnt(qvariant_cast<QFont>(property));
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
                QFont f(qvariant_cast<QFont>(property));
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
                applyFont(f);
			});

			layout->addWidget(widget);
            layout->addWidget(weight);
			layout->addWidget(size);
			layout->setSpacing(fit(5));
			break;

		} default: {
            _valid = false;
			break;
		}
	}
}

bool PropertyItem::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Wheel) {
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
