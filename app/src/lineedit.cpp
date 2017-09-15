#include <lineedit.h>
#include <fit.h>
#include <QLineEdit>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QHBoxLayout>
#include <QIcon>

using namespace Fit;

class LineEditPrivate
{
	public:

		QLabel iconLabel;
		QLineEdit lineEdit;
		QVBoxLayout layout;
		QHBoxLayout labelLayout;
};

LineEdit::LineEdit(QWidget* parent)
	: QWidget(parent)
	, _d(new LineEditPrivate)
{
	QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect;
    shadowEffect->setBlurRadius(fit(3));
    shadowEffect->setOffset(0, fit(2));
    shadowEffect->setColor(QColor(0, 0, 0, 60));
	setGraphicsEffect(shadowEffect);

	_d->lineEdit.setStyleSheet(QString("background:white;border:none; border-top-right-radius:%1px;"
										 "border-bottom-right-radius:%1px;padding-left:%1;").arg(fit(2)));
	_d->lineEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	_d->lineEdit.setFixedHeight(fit(30));
	_d->lineEdit.setAlignment(Qt::AlignLeft);

	_d->iconLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_d->iconLabel.setFixedSize(fit(30), fit(30));
	_d->iconLabel.setStyleSheet(QString("background:#cccccc; border-top-left-radius:%1px;"
									"border-bottom-left-radius:%1px;").arg(fit(2)));

	_d->labelLayout.setSpacing(0);
	_d->labelLayout.addWidget(&_d->iconLabel);
	_d->labelLayout.addWidget(&_d->lineEdit);
	_d->labelLayout.setContentsMargins(0, 0, 0, 0);

	_d->layout.setSpacing(0);
	_d->layout.setContentsMargins(0,0,0,0);
	_d->layout.addLayout(&_d->labelLayout);
	setLayout(&_d->layout);
}

void LineEdit::setIcon(const QIcon& icon)
{
	_d->iconLabel.setPixmap(icon.pixmap(fit(30), fit(30)));
}

void LineEdit::setPlaceholderText(const QString& text)
{
	_d->lineEdit.setPlaceholderText(text);
}

void LineEdit::setColor(const QColor& color)
{
	_d->iconLabel.setStyleSheet(QString("background:%2; border-top-left-radius:%1px;"
										 "border-bottom-left-radius:%1px;").arg(fit(2)).arg(color.name()));
}

void LineEdit::setText(const QString& text)
{
	_d->lineEdit.setText(text);
}

const QString LineEdit::text() const
{
	return _d->lineEdit.text();
}

QLineEdit*LineEdit::lineEdit()
{
	return &_d->lineEdit;
}

LineEdit::~LineEdit()
{
	delete _d;
}
