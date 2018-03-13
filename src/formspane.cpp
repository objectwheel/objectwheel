#include <formspane.h>
#include <flatbutton.h>
#include <toolboxtree.h>
#include <saveutils.h>
#include <savebackend.h>
#include <projectbackend.h>
#include <designerscene.h>
#include <filemanager.h>
#include <css.h>
#include <fit.h>
#include <delayer.h>

#include <QLabel>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPalette>
#include <QListWidget>
#include <QScrollBar>
#include <QPainter>
#include <QStyledItemDelegate>

class FormListDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        FormListDelegate(QWidget* parent);
        void paint(QPainter* painter, const QStyleOptionViewItem &option,
          const QModelIndex &index) const override;
};

FormListDelegate::FormListDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

void FormListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    const QPen oldPen = painter->pen();
    painter->setPen("#10000000");
    painter->drawLine(QPointF(0.5, option.rect.bottom() + 0.5),
                      QPointF(option.rect.right() + 0.5, option.rect.bottom() + 0.5));
    painter->setPen(oldPen);

    QStyledItemDelegate::paint(painter, option, index);
}

FormsPane::FormsPane(DesignerScene* designerScene, QWidget* parent) : QWidget(parent)
  , m_designerScene(designerScene)
{
    _layout = new QVBoxLayout(this);
    _innerWidget = new QFrame;
    _innerLayout = new QVBoxLayout(_innerWidget);
    _header = new QLabel;
    _listWidget = new QListWidget;
    _buttonLayout = new QHBoxLayout;
    _addButton = new FlatButton;
    _removeButton = new FlatButton;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#E0E4E7");
    setAutoFillBackground(true);
    setPalette(p);

    QPalette p2(_listWidget->palette());
    p2.setColor(QPalette::All, QPalette::Base, QColor("#f5faff"));
    p2.setColor(QPalette::All, QPalette::Highlight, QColor("#c0d5eb"));
    p2.setColor(QPalette::All, QPalette::Text, QColor("#202427"));
    p2.setColor(QPalette::All, QPalette::HighlightedText, QColor("#202427"));
    _listWidget->setPalette(p2);

    _listWidget->setItemDelegate(new FormListDelegate(_listWidget));
    _listWidget->viewport()->installEventFilter(this);
    _listWidget->setStyleSheet("QListView { border: none; }");
    _listWidget->setFocusPolicy(Qt::NoFocus);
    _listWidget->setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _listWidget->setSelectionMode(QListWidget::SingleSelection);
    _listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    _listWidget->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTimer::singleShot(1000, [=] { //FIXME
        Delayer::delay([=]() -> bool { if (SaveBackend::instance()) return false; else return true;});
        connect(ProjectBackend::instance(), SIGNAL(started()), SLOT(handleDatabaseChange()));
        connect(SaveBackend::instance(), SIGNAL(databaseChanged()), SLOT(handleDatabaseChange()));
        connect(_listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(handleCurrentFormChange()));
    });

    _innerWidget->setObjectName("innerWidget");
    _innerWidget->setStyleSheet("#innerWidget { border: 1px solid #1a69bd; }");

    _header->setText("Forms");
    _header->setFixedHeight(fit::fx(23));
    _header->setStyleSheet(
        "color: white; font-weight: Medium; border:none; border-bottom: 1px solid #1a69bd;"
        "background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 #2784E3, stop:1 #1069C7);"
    );

    _addButton->settings().topColor = "#62A558";
    _addButton->settings().bottomColor = "#599750";
    _addButton->settings().borderRadius = fit::fx(10);
    _addButton->settings().textColor = Qt::white;
    _addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _addButton->setFixedSize(fit::fx(20), fit::fx(20));
    _addButton->setIconSize(QSize(fit::fx(12),fit::fx(12)));
    _addButton->setIcon(QIcon(":/resources/images/plus.png"));
    connect(_addButton, SIGNAL(clicked(bool)), SLOT(addButtonClicked()));

    _removeButton->settings().topColor = "#C2504B";
    _removeButton->settings().bottomColor = "#B34B46";
    _removeButton->settings().borderRadius = fit::fx(12);
    _removeButton->settings().textColor = Qt::white;
    _removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _removeButton->setFixedSize(fit::fx(20),fit::fx(20));
    _removeButton->setIconSize(QSize(fit::fx(12),fit::fx(12)));
    _removeButton->setIcon(QIcon(":/resources/images/minus.png"));
    connect(_removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonClicked()));

    _buttonLayout->addWidget(_addButton);
    _buttonLayout->addStretch();
    _buttonLayout->addWidget(_removeButton);

    _innerLayout->addWidget(_header);
    _innerLayout->addWidget(_listWidget);
    _innerLayout->setSpacing(0);
    _innerLayout->setContentsMargins(0, 0, 0, 0);

    _layout->addWidget(_innerWidget);
    _layout->addLayout(_buttonLayout);
    _layout->setSpacing(fit::fx(2));
    _layout->setContentsMargins(fit::fx(3), fit::fx(3), fit::fx(3), fit::fx(3));
}

bool FormsPane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _listWidget->viewport()) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(_listWidget->viewport());
            if (_listWidget->count() > 0) {
                const auto tli = _listWidget->item(0);
                const auto& tlir = _listWidget->visualItemRect(tli);
                const qreal ic = (
                    _listWidget->viewport()->height() +
                    qAbs(tlir.y())
                ) / (qreal) tlir.height();

                for (int i = 0; i < ic; i++) {
                    if (i % 2 == 0) {
                        painter.fillRect(
                            0,
                            tlir.y() + i * tlir.height(),
                            _listWidget->viewport()->width(),
                            tlir.height(),
                            QColor("#e8f1fa")
                        );
                    }
                }
            } else {
                const qreal hg = fit::fx(20.0);
                const qreal ic = _listWidget->viewport()->height() / hg;

                for (int i = 0; i < ic; i++) {
                    if (i % 2 == 0) {
                        painter.fillRect(
                            0, i * hg,
                            _listWidget->viewport()->width(),
                            hg, QColor("#e8f1fa")
                        );
                    } else if (i == int(ic / 2.0) || i == int(ic / 2.0) + 1) {
                        painter.setPen(QColor("#a5aab0"));
                        painter.drawText(0, i * hg, _listWidget->viewport()->width(),
                          hg, Qt::AlignCenter, "No forms to show");
                    }
                }
            }
        }

        return false;
    } else {
        return QWidget::eventFilter(watched, event);
    }
}

void FormsPane::removeButtonClicked()
{
    auto form = m_designerScene->mainForm();
    if (!form || !form->form() || form->main())
        return;
    SaveBackend::instance()->removeForm((Form*)form);
    m_designerScene->removeForm(form);
}

void FormsPane::addButtonClicked()
{
    auto tempPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    tempPath = tempPath + separator() + "Objectwheel";

    rm(tempPath);

    if (!mkdir(tempPath) || !cp(":/resources/qmls/form", tempPath, true, true))
        return;

    auto form = new Form(tempPath + separator() + DIR_THIS + separator() + "main.qml");
    m_designerScene->addForm(form);
    SaveBackend::instance()->addForm(form);
    rm(tempPath);
}

void FormsPane::handleDatabaseChange()
{
    int row = 0;
    QString id;
    if (_listWidget->currentItem())
        id = _listWidget->currentItem()->text();

    _listWidget->clear();

    for (auto path : SaveUtils::formPaths(ProjectBackend::instance()->dir())) {
        auto _id = SaveUtils::id(path);
        if (id == _id)
            row = _listWidget->count();

        auto item = new QListWidgetItem;
        item->setText(_id);
        if (SaveUtils::isMain(path))
            item->setIcon(QIcon(":/resources/images/mform.png"));
        else
            item->setIcon(QIcon(":/resources/images/form.png"));
        _listWidget->addItem(item);
    }
    _listWidget->setCurrentRow(row);
}

void FormsPane::handleCurrentFormChange()
{
    if (!_listWidget->currentItem())
        return;

    auto id = _listWidget->currentItem()->text();
    for (auto form : m_designerScene->forms())
        if (form->id() == id)
            m_designerScene->setMainForm(form);
    emit currentFormChanged();
}

void FormsPane::setCurrentForm(int index)
{
    _listWidget->setCurrentRow(index);
}

void FormsPane::clear()
{
    _listWidget->clear();
}

QSize FormsPane::sizeHint() const
{
    return fit::fx(QSizeF{215, 160}).toSize();
}

#include "formspane.moc"
