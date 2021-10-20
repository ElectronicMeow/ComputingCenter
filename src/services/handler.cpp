//
// Created by reverier on 2021/10/21.
//

#include <QJsonArray>
#include "handler.h"
#include "utils/crypto_utils.h"


MainHandler::MainHandler() {
//    this->router_.addRoute("GET", "^/generate-keys$", this, &MainHandler::handleGenerateKeys);
//    this->router_.addRoute("POST", "^/transform-to-same-pk$", this, &MainHandler::handleTransformToSamePk);
//    this->router_.addRoute("POST", "^/transform-back$", this, &MainHandler::handleTransformBack);
}

HttpPromise MainHandler::handle(HttpDataPtr data) {
    bool foundRoute;
    HttpPromise promise = this->router_.route(data, &foundRoute);
    if (foundRoute)
        return promise;

    qWarning("Request URI in this method is not found.");

    QJsonObject object;
    object["message"] = "INVALID URI or Method.";

    data->response->setStatus(HttpStatus::Forbidden, QJsonDocument(object));
    return HttpPromise::resolve(data);
}


//HttpPromise MainHandler::handleTransformBack(HttpDataPtr data) {
//    if (data->request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0) {
//        qWarning("Incorrect data type in handleTransformToSamePk.");
//        throw HttpException(HttpStatus::BadRequest, "Request body content type must be application/json");
//    }
//    QJsonDocument jsonDocument = data->request->parseJsonBody();
//    if (jsonDocument.isNull()) {
//        qWarning("Incorrect data format in handleTransformToSamePk.");
//        throw HttpException(HttpStatus::BadRequest, "Invalid JSON body");
//    }
//
//    MeowCryptoUtils::EncryptedPair inp;
//    inp.setPublicKey(this->prod_pk_);
//    inp.setPublicN(this->publicParameters_.N());
//    inp.setA(jsonDocument.object()["encrypted_pair_a"].toString());
//    inp.setB(jsonDocument.object()["encrypted_pair_b"].toString());
//
//    QJsonArray ans;
//
//    for (auto &pk : this->pk_reverse_map_.keys()) {
//        auto tag = this->pk_reverse_map_[pk];
//        auto r = MeowCryptoUtils::masterTransform(this->publicParameters_, this->sk_, pk, inp);
//
//        QJsonArray record;
//        record.append(tag); // tag
//        record.append(r.A()); // encrypted pair A
//        record.append(r.B()); // encrypted pair B
//        ans.append(record);
//    }
//
//    QJsonObject resp;
//    resp["data"] = ans;
//    resp["message"] = "ok";
//
//    data->response->setStatus(HttpStatus::Ok, QJsonDocument(resp));
//
//    return HttpPromise::resolve(data);
//}
