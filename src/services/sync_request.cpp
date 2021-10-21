//
// Created by reverier on 2021/10/22.
//

#include "sync_request.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QJsonDocument sync_get(QNetworkAccessManager *mgr, const QString &strUrl) {
    assert(!strUrl.isEmpty());

    const QUrl url = QUrl::fromUserInput(strUrl);
    assert(url.isValid());

    QNetworkRequest qnr(url);
    QNetworkReply *reply = mgr->get(qnr);

    QEventLoop eventLoop;
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray replyDataRaw = reply->readAll();
    QJsonDocument replyData = QJsonDocument::fromJson(replyDataRaw);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVariant redirectAttr = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()
        || 300 == statusCode
        || !redirectAttr.isNull()) {
        QString errString = reply->error() ? reply->errorString() : QString(
                "redirect found (%1) which is not allowed").arg(statusCode);
        replyData = QJsonDocument::fromJson(QString(R"({ "message":"%1", "error": true })").arg(errString).toLocal8Bit());
    }

    reply->deleteLater();

    return replyData;
}


QJsonDocument sync_post(QNetworkAccessManager *mgr, const QString &strUrl, const QJsonDocument &doc) {
    assert(!strUrl.isEmpty());

    const QUrl url = QUrl::fromUserInput(strUrl);
    assert(url.isValid());

    QNetworkRequest qnr(url);
    QNetworkReply *reply = mgr->post(qnr, doc.toJson());

    QEventLoop eventLoop;
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray replyDataRaw = reply->readAll();
    QJsonDocument replyData = QJsonDocument::fromJson(replyDataRaw);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVariant redirectAttr = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()
        || 300 == statusCode
        || !redirectAttr.isNull()) {
        QString errString = reply->error() ? reply->errorString() : QString(
                "redirect found (%1) which is not allowed").arg(statusCode);
        replyData = QJsonDocument::fromJson(
                QString(R"({ "message":"%1", "error": true })").arg(errString).toLocal8Bit());
    }

    reply->deleteLater();

    return replyData;
}
