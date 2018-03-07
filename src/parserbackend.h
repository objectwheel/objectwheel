#ifndef PARSERBACKEND_H
#define PARSERBACKEND_H

#include <QString>

class ParserBackend
{
        Q_DISABLE_COPY(ParserBackend)

    public:
        static ParserBackend* instance();
        const QString& source() const;
        void init(const QString& fileName);
        void setProperty(const QString& property, const QString& value);

    private:
        ParserBackend() {}

    private:
        QString _source, _fileName;

};

#endif // PARSERBACKEND_H