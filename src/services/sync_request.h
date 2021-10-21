//
// Created by reverier on 2021/10/22.
//

#ifndef MEOWCOMPUTINGCENTER_REQUEST_H
#define MEOWCOMPUTINGCENTER_REQUEST_H

#include <QNetworkAccessManager>

QJsonDocument sync_get(QNetworkAccessManager *mgr, const QString &strUrl);

QJsonDocument sync_post(QNetworkAccessManager *mgr, const QString &strUrl, const QJsonDocument &doc);

#endif //MEOWCOMPUTINGCENTER_REQUEST_H
