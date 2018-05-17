#ifndef TEXTUTILS_H
#define TEXTUTILS_H

class QTextDocument;

class TextUtils
{
public:
    static void trimCommentsAndStrings(QTextDocument* document);
};

#endif // TEXTUTILS_H