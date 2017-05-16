#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QWidget>

class ComboBoxPrivate;

class ComboBox : public QWidget
{
		Q_OBJECT

	public:
		explicit ComboBox(QWidget *parent = 0);
		void clear();
		void setColor(const QColor& color);
		void setIcon(const QIcon& icon);
		void setPlaceHolderText(const QString& text);
		void addItem(const QString& item);
		const QString item(const int index) const;
		const QString currentItem() const;
		void setCurrentItem(const QString& text);
		const QStringList items() const;
		~ComboBox();

	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);

	public slots:
		void showPopup();
		void hidePopup();

	signals:
		void currentTextChanged(const QString& text) const;
		void popupShowed() const;
		void popupHided() const;

	private:
		ComboBoxPrivate* m_d;
};

#endif // COMBOBOX_H
