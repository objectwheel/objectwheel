#include <outputbox.h>
#include <QSplitter>

class OutputBoxPrivate : public QObject
{
        Q_OBJECT
    public:
        OutputBoxPrivate(OutputBox *parent);

    public:
        OutputBox* parent;
};

OutputBoxPrivate::OutputBoxPrivate(OutputBox* parent)
    : QObject(parent)
    , parent(parent)
{

}


OutputBox::OutputBox(QSplitter* splitter, QWidget *parent)
    : QWidget(parent)
    , _splitter(splitter)
    , _d(new OutputBoxPrivate(this))
{

}

QSize OutputBox::sizeHint() const
{
    return QSize(200, 140);
}


#include "outputbox.moc"
