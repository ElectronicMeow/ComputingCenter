#ifndef HTTP_SERVER_HTTP_COOKIE_H
#define HTTP_SERVER_HTTP_COOKIE_H

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QUrl>
#include <utility>

#include "util.h"


struct HTTPSERVER_EXPORT HttpCookie {
    QString name;
    QString value;
    int ageSeconds{};
    QDateTime expiration;
    QString domain;
    QString path;
    bool secure{};
    bool httpOnly{};

    HttpCookie() = default;

    HttpCookie(QString name, QString value, int ageSeconds = -1, QDateTime expiration = QDateTime(),
               QString domain = "",
               QString path = "/", bool secure = false, bool httpOnly = false) : name(std::move(name)), value(std::move(value)),
                                                                                 ageSeconds(ageSeconds),
                                                                                 expiration(std::move(expiration)), domain(std::move(domain)),
                                                                                 path(std::move(path)), secure(secure),
                                                                                 httpOnly(httpOnly) { }

    [[nodiscard]] QByteArray toByteArray() const {
        QByteArray buf;

        buf += name.toLatin1();
        buf += '=';
        buf += QUrl::toPercentEncoding(value);

        if (expiration.isValid())
            buf += ("; Expires=" + expiration.toString(Qt::RFC2822Date)).toLatin1();

        if (ageSeconds > 0)
            buf += ("; Max-Age=" + QString::number(ageSeconds)).toLatin1();

        if (!domain.isEmpty())
            buf += ("; Domain=" + domain).toLatin1();

        if (!path.isEmpty())
            buf += "; Path=" + QUrl::toPercentEncoding(path);

        if (secure)
            buf += "; Secure";

        if (httpOnly)
            buf += "; HttpOnly";

        return buf;
    }
};

#endif // HTTP_SERVER_HTTP_COOKIE_H
