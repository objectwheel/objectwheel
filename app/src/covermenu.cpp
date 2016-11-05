#include <fit.h>
#include <covermenu.h>

#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

CoverMenu::CoverMenu(QWidget *parent)
	: QWidget(parent)
	, m_CoverSide(CoverSide::FromLeft)
	, m_CoverWidget(nullptr)
	, m_Container(nullptr)
	, m_Layout(new QVBoxLayout(this))
	, m_MenuWidth(fit(170))
	, m_Duration(500)
	, m_AnimationState(QParallelAnimationGroup::Stopped)
	, m_Shadow(new QWidget(this))
{
	m_Layout->setSpacing(0);
	m_Layout->setContentsMargins(0, 0, 0, 0);
	hide();
}

int CoverMenu::menuWidth() const
{
	return m_MenuWidth;
}

void CoverMenu::setMenuWidth(const int menuWidth)
{
	m_MenuWidth = menuWidth;
}

int CoverMenu::duration() const
{
	return m_Duration;
}

void CoverMenu::setDuration(const int duration)
{
	m_Duration = duration;
}

void CoverMenu::setCoverSide(const CoverSide& coverSide)
{
	m_CoverSide = coverSide;
}

CoverMenu::CoverSide CoverMenu::coverSide() const
{
	return m_CoverSide;
}

QWidget* CoverMenu::coverWidget() const
{
	return m_CoverWidget;
}

void CoverMenu::setCoverWidget(QWidget* const coverWidget)
{
	Q_ASSERT_X(coverWidget->parentWidget() != nullptr, "CoverMenu::setCoverWidget() ", "Cover widget must be a child widget.");
	setParent(coverWidget->parentWidget());
	m_CoverWidget = coverWidget;
}

QObject* CoverMenu::container() const
{
	return m_Container;
}

void CoverMenu::setContainer(QWidget* const container)
{
	removeContainer();
	m_Layout->insertWidget(0, container);
	m_Container = container;
}

void CoverMenu::setContainer(QLayout* const container)
{
	removeContainer();
	m_Layout->insertLayout(0, container);
	m_Container = container;
}

void CoverMenu::removeContainer()
{
	m_Layout->takeAt(0);
	m_Container = nullptr;
}

QAbstractAnimation::State CoverMenu::animationState() const
{
	return m_AnimationState;
}

void CoverMenu::fixShadow()
{
	switch (m_CoverSide)
	{
		case FromLeft:
			m_Shadow->setStyleSheet("background:qlineargradient(spread:pad, x1:1, y1:0.5, x2:0, y2:0.5,stop:0 "
									"rgba(0, 0, 0, 120), stop:0.6 rgba(0, 0, 0, 30), stop:1 rgba(0, 0, 0, 0));");
			m_Shadow->move(width() - fit(8), 0);
			m_Shadow->resize(fit(8), height());
			break;

		case FromRight:
			m_Shadow->setStyleSheet("background:qlineargradient(spread:pad, x1:0, y1:0.5, x2:1, y2:0.5,stop:0 "
									"rgba(0, 0, 0, 120), stop:0.6 rgba(0, 0, 0, 30), stop:1 rgba(0, 0, 0, 0));");
			m_Shadow->move(0, 0);
			m_Shadow->resize(fit(8), height());
			break;

		case FromTop:
			m_Shadow->setStyleSheet("background:qlineargradient(spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,stop:0 "
									"rgba(0, 0, 0, 120), stop:0.6 rgba(0, 0, 0, 30), stop:1 rgba(0, 0, 0, 0));");
			m_Shadow->move(0, height() - fit(8));
			m_Shadow->resize(width(), fit(8));
			break;

		case FromBottom:
			m_Shadow->setStyleSheet("background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,stop:0 "
									"rgba(0, 0, 0, 120), stop:0.6 rgba(0, 0, 0, 30), stop:1 rgba(0, 0, 0, 0));");
			m_Shadow->move(0, 0);
			m_Shadow->resize(width(), fit(8));
			break;

		default:
			qWarning("CoverMenu::cover() Wrong cover side.");
			break;
	}
	m_Shadow->show();
	m_Shadow->raise();
}

void CoverMenu::setAnimationState(const QAbstractAnimation::State animationState, const QAbstractAnimation::State)
{
	m_AnimationState = animationState;
}

void CoverMenu::cover()
{
	Q_ASSERT_X(m_CoverWidget != nullptr, "CoverMenu::cover() ", "Set cover widget first.");

	if (isVisible() || QParallelAnimationGroup::Running == m_AnimationState)
		return;

	QPropertyAnimation *first = new QPropertyAnimation(this, "pos");
	first->setDuration(m_Duration);
	first->setEasingCurve(QEasingCurve::OutExpo);

	QPropertyAnimation *second = new QPropertyAnimation(m_CoverWidget, "pos");
	second->setDuration(m_Duration);
	second->setEasingCurve(QEasingCurve::OutExpo);

	QParallelAnimationGroup *group = new QParallelAnimationGroup;
	group->addAnimation(first);
	group->addAnimation(second);
	connect(group, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)),
			this, SLOT(setAnimationState(QAbstractAnimation::State, QAbstractAnimation::State)));
	connect(group, &QParallelAnimationGroup::finished, [this] {emit toggled(true);});
	connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));

	switch (m_CoverSide)
	{
		case FromLeft:
			resize(m_MenuWidth, m_CoverWidget->height());
			first->setStartValue(m_CoverWidget->pos() - QPoint(m_MenuWidth, 0));
			first->setEndValue(m_CoverWidget->pos());
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() + QPoint(m_MenuWidth, 0));
			break;

		case FromRight:
			resize(m_MenuWidth, m_CoverWidget->height());
			first->setStartValue(m_CoverWidget->pos() + QPoint(m_CoverWidget->width(), 0));
			first->setEndValue(m_CoverWidget->pos() + QPoint(m_CoverWidget->width(), 0) - QPoint(m_MenuWidth, 0));
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() - QPoint(m_MenuWidth, 0));
			break;

		case FromTop:
			resize(m_CoverWidget->width(), m_MenuWidth);
			first->setStartValue(m_CoverWidget->pos() - QPoint(0, m_MenuWidth));
			first->setEndValue(m_CoverWidget->pos());
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() + QPoint(0, m_MenuWidth));
			break;

		case FromBottom:
			resize(m_CoverWidget->width(), m_MenuWidth);
			first->setStartValue(m_CoverWidget->pos() + QPoint(0, m_CoverWidget->height()));
			first->setEndValue(m_CoverWidget->pos() + QPoint(0, m_CoverWidget->height()) - QPoint(0, m_MenuWidth));
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() - QPoint(0, m_MenuWidth));
			break;

		default:
			qWarning("CoverMenu::cover() Wrong cover side.");
			break;
	}

	show();

	fixShadow();

	group->start();
}

void CoverMenu::uncover()
{
	Q_ASSERT_X(m_CoverWidget != nullptr, "CoverMenu::cover() ", "Set cover widget first.");

	if (isHidden() || QParallelAnimationGroup::Running == m_AnimationState)
		return;

	QPropertyAnimation *first = new QPropertyAnimation(this, "pos");
	first->setDuration(m_Duration);
	first->setEasingCurve(QEasingCurve::OutExpo);

	QPropertyAnimation *second = new QPropertyAnimation(m_CoverWidget, "pos");
	second->setDuration(m_Duration);
	second->setEasingCurve(QEasingCurve::OutExpo);

	QParallelAnimationGroup *group = new QParallelAnimationGroup;
	group->addAnimation(first);
	group->addAnimation(second);
	connect(group, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)),
			this, SLOT(setAnimationState(QAbstractAnimation::State, QAbstractAnimation::State)));
	connect(group, SIGNAL(finished()), this, SLOT(hide()));
	connect(group, &QParallelAnimationGroup::finished, [this] {emit toggled(false);});
	connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));

	switch (m_CoverSide)
	{
		case FromLeft:
			first->setStartValue(pos());
			first->setEndValue(pos() - QPoint(m_MenuWidth, 0));
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() - QPoint(m_MenuWidth, 0));
			break;

		case FromRight:
			first->setStartValue(pos());
			first->setEndValue(pos() + QPoint(m_MenuWidth, 0));
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() + QPoint(m_MenuWidth, 0));
			break;

		case FromTop:
			first->setStartValue(pos());
			first->setEndValue(pos() - QPoint(0, m_MenuWidth));
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() - QPoint(0, m_MenuWidth));
			break;

		case FromBottom:
			first->setStartValue(pos());
			first->setEndValue(pos() + QPoint(0, m_MenuWidth));
			second->setStartValue(m_CoverWidget->pos());
			second->setEndValue(m_CoverWidget->pos() + QPoint(0, m_MenuWidth));
			break;

		default:
			qWarning("CoverMenu::cover() Wrong cover side.");
			break;
	}

	group->start();
}

void CoverMenu::setCovered(bool covered)
{
	if (true == covered)
		cover();
	else
		uncover();
}
