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
	, m_d(new LineEditPrivate)
{
	m_d->lineEdit.setStyleSheet(QString("background:white;border:none; border-top-right-radius:%1px;"
										 "border-bottom-right-radius:%1px;padding-left:%1;").arg(fit(2)));
	m_d->lineEdit.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_d->lineEdit.setPlaceholderText("Search");
	m_d->lineEdit.setFixedHeight(fit(30));
	m_d->lineEdit.setAlignment(Qt::AlignLeft);

	QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect;
	shadowEffect->setBlurRadius(fit(7));
	shadowEffect->setOffset(0, fit(4));
	shadowEffect->setColor(QColor(0, 0, 0, 50));
	m_d->lineEdit.setGraphicsEffect(shadowEffect);

	m_d->iconLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_d->iconLabel.setFixedSize(fit(30), fit(30));
	m_d->iconLabel.setStyleSheet(QString("background:#cccccc; border-top-left-radius:%1px;"
									"border-bottom-left-radius:%1px;").arg(fit(2)));

	QGraphicsDropShadowEffect* shadowEffect2 = new QGraphicsDropShadowEffect;
	shadowEffect2->setBlurRadius(fit(7));
	shadowEffect2->setOffset(0, fit(4));
	shadowEffect2->setColor(QColor(0, 0, 0, 50));
	m_d->iconLabel.setGraphicsEffect(shadowEffect2);

	m_d->labelLayout.setSpacing(0);
	m_d->labelLayout.addWidget(&m_d->iconLabel);
	m_d->labelLayout.addWidget(&m_d->lineEdit);
	m_d->labelLayout.setContentsMargins(0, 0, fit(7), 0);

	m_d->layout.setSpacing(0);
	m_d->layout.setContentsMargins(0,0,0,0);
	m_d->layout.addLayout(&m_d->labelLayout);
	setLayout(&m_d->layout);
}

void LineEdit::setIcon(const QIcon& icon)
{
	m_d->iconLabel.setPixmap(icon.pixmap(fit(30), fit(30)));
}

LineEdit::~LineEdit()
{
	delete m_d;
}
