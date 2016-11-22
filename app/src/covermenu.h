#ifndef COVERMENU_H
#define COVERMENU_H

#include <QWidget>
#include <QParallelAnimationGroup>

class QVBoxLayout;

class CoverMenu : public QWidget
{
		Q_OBJECT
	public:
		enum CoverSide
		{ FromLeft, FromRight, FromTop, FromBottom };

	private:
		CoverSide m_CoverSide;
		QWidget* m_CoverWidget;
		QObject* m_Container;
		QVBoxLayout* m_Layout;
		int m_MenuWidth;
		int m_Duration;
		QAbstractAnimation::State m_AnimationState;
		QWidget* m_ShadowWidget;

	public:
		explicit CoverMenu(QWidget *parent = 0);

		CoverSide coverSide() const;
		void setCoverSide(const CoverSide& coverSide);

		QWidget* coverWidget() const;
		void setCoverWidget(QWidget* const coverWidget);

		QObject* container() const;
		void setContainer(QWidget* const container);
		void setContainer(QLayout* const container);
		void removeContainer();

		int menuWidth() const;
		void setMenuWidth(const int menuWidth);

		int duration() const;
		void setDuration(const int duration);

	protected:
		QAbstractAnimation::State animationState() const;
		void fixShadow();
		void cover();
		void uncover();

	signals:
		void toggled(bool covered);

	public slots:
		void setCovered(bool covered);

	protected slots:
		void setAnimationState(const QAbstractAnimation::State animationState, const QAbstractAnimation::State);
};

#endif // COVERMENU_H
