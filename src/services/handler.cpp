//
// Created by reverier on 2021/10/21.
//

#include <QJsonArray>
#include "handler.h"
#include "utils/crypto_utils.h"
#include "sync_request.h"


MainHandler::MainHandler() {
//    this->router_.addRoute("GET", "^/generate-keys$", this, &MainHandler::handleGenerateKeys);
//    this->router_.addRoute("POST", "^/transform-to-same-pk$", this, &MainHandler::handleTransformToSamePk);
    this->router_.addRoute("POST", "^/computing-request$", this, &MainHandler::handleComputingRequest);
    this->router_.addRoute("POST", "^/register$", this, &MainHandler::handleRegister);
    this->router_.addRoute("POST", "^/register-master$", this, &MainHandler::handleRegisterMaster);
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

HttpPromise MainHandler::handleRegister(HttpDataPtr data) {
    if (data->request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0) {
        qWarning("Incorrect data type in handleRegister.");
        throw HttpException(HttpStatus::BadRequest, "Request body content type must be application/json");
    }
    QJsonDocument jsonDocument = data->request->parseJsonBody();
    if (jsonDocument.isNull()) {
        qWarning("Incorrect data format in handleRegister.");
        throw HttpException(HttpStatus::BadRequest, "Invalid JSON body");
    }
    try {
        auto address = jsonDocument.object()["data"].toObject()["url"].toString();
        auto tag = jsonDocument.object()["data"].toObject()["tag"].toString();
        qInfo("Server %s at %s registered.", tag.toLocal8Bit().data(), address.toLocal8Bit().data());
        this->clients_address_[tag] = (address);
    } catch (std::exception &e) {
        qInfo("Server register failed because of bad data: %s", jsonDocument.toJson().data());
        throw HttpException(HttpStatus::BadRequest, "Bad data");
    }

    return HttpPromise::resolve(data);
}

HttpPromise MainHandler::handleRegisterMaster(HttpDataPtr data) {
    if (data->request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0) {
        qWarning("Incorrect data type in handleRegisterMaster.");
        throw HttpException(HttpStatus::BadRequest, "Request body content type must be application/json");
    }
    QJsonDocument jsonDocument = data->request->parseJsonBody();
    if (jsonDocument.isNull()) {
        qWarning("Incorrect data format in handleRegisterMaster.");
        throw HttpException(HttpStatus::BadRequest, "Invalid JSON body");
    }

    try {
        auto address = jsonDocument.object()["data"].toObject()["url"].toString();
        this->master_address_ = address;
    } catch (std::exception &e) {
        throw HttpException(HttpStatus::BadRequest, "Bad data");
    }

    return HttpPromise::resolve(data);
}

HttpPromise MainHandler::handleComputingRequest(HttpDataPtr data) {

    clearData();

    if (data->request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0) {
        qWarning("Incorrect data type in handleComputingRequest.");
        throw HttpException(HttpStatus::BadRequest, "Request body content type must be application/json");
    }
    QJsonDocument jsonDocument = data->request->parseJsonBody();
    if (jsonDocument.isNull()) {
        qWarning("Incorrect data format in handleComputingRequest.");
        throw HttpException(HttpStatus::BadRequest, "Invalid JSON body");
    }

    /// MAIN COMPUTING CONTROL FUNCTION
    qWarning("==> SYNC COMPUTING STARTED <==");
    QNetworkAccessManager mgr;

    auto filter_data = jsonDocument.object().value("data").toObject();
    auto public_param_d = sync_get(&mgr, this->master_address_ + "/generate-keys").object().value("data").toObject();

    MeowCryptoUtils::PublicParameters public_param;
    public_param.setN(public_param_d["N"].toString());
    public_param.setK(public_param_d["k"].toString());
    public_param.setG(public_param_d["g"].toString());

    qInfo("Got Master's PublicParam: N=%s, k=%s, g=%s", public_param_d["N"].toString().toStdString().c_str(),
          public_param_d["k"].toString().toStdString().c_str(), public_param_d["g"].toString().toStdString().c_str());

    auto client_data_request_data = filter_data;
    client_data_request_data["N"] = public_param.N();
    client_data_request_data["k"] = public_param.k();
    client_data_request_data["g"] = public_param.g();

    QMap<QString, MeowCryptoUtils::EncryptedPair> all_clients_data_map;

    for (auto &client_tag: this->clients_address_.keys()) {
        auto resp = sync_post(&mgr, this->clients_address_[client_tag] + "/data-request",
                              QJsonDocument(client_data_request_data)).object()["data"].toObject();
        auto pk = resp["public_key"].toString();
        this->pk_map_[client_tag] = pk;

        MeowCryptoUtils::EncryptedPair enc;
        enc.setPublicN(public_param.N());
        enc.setPublicKey(pk);
        enc.setA(resp["encrypted_pair_a"].toString());
        enc.setB(resp["encrypted_pair_b"].toString());
        all_clients_data_map[client_tag] = enc;

        qInfo("Got Client[%s]'s data: a=%s, b=%s, pk=%s",
              client_tag.toStdString().c_str(),
              resp["encrypted_pair_a"].toString().toStdString().c_str(),
              resp["encrypted_pair_b"].toString().toStdString().c_str(),
              pk.toStdString().c_str());
    }

    auto cc_prod_pk = MeowCryptoUtils::getProdKey(public_param, this->pk_map_.values());
    qInfo("ProdPk=%s", cc_prod_pk.toStdString().c_str());

    QMap<QString, MeowCryptoUtils::EncryptedPair> all_clients_data_obfuscated_map;
    QMap<QString, QString> obfuscated_random_snips;

    qInfo("Requesting Master to transform ciphers...");

    QJsonArray master_transform_request_data_arr;

    for (auto &client_tag: all_clients_data_map.keys()) {
        MeowCryptoUtils::EncryptedPair obfuscated_cipher;
        QString obfuscated_r;
        bool ok = MeowCryptoUtils::ccTransform(public_param, all_clients_data_map[client_tag], obfuscated_cipher,
                                               obfuscated_r);
        if (!ok)
            qCritical("Error occurred when Computing Center obfuscating client data!");
        obfuscated_random_snips[client_tag] = obfuscated_r;
        all_clients_data_obfuscated_map[client_tag] = obfuscated_cipher;

        QJsonArray arr;
        arr.append(client_tag);
        arr.append(obfuscated_cipher.A());
        arr.append(obfuscated_cipher.B());
        arr.append(obfuscated_cipher.publicKey());

        master_transform_request_data_arr.append(arr);
    }

    QJsonObject master_transform_request_data;
    master_transform_request_data["data"] = master_transform_request_data_arr;

    auto master_transform_result_data = sync_post(&mgr, this->master_address_ + "/transform-to-same-pk",
                                                  QJsonDocument(
                                                          master_transform_request_data)).object()["data"].toArray();

    QMap<QString, MeowCryptoUtils::EncryptedPair> master_transformed_map;

    qInfo("Master transform is over");

    for (auto i: master_transform_request_data) {
        auto arr = i.toArray();
        MeowCryptoUtils::EncryptedPair enc;
        enc.setPublicN(public_param.N());
        enc.setPublicKey(cc_prod_pk);
        enc.setA(arr[1].toString());
        enc.setB(arr[2].toString());
        master_transformed_map[arr[0].toString()] = enc;
    }

    /// COMPUTING
    auto computed_res = MeowCryptoUtils::ccComputing(public_param, cc_prod_pk, master_transformed_map,
                                                     obfuscated_random_snips);

    MeowCryptoUtils::EncryptedPair obfuscated_computed_res;

    QString obfuscated_random_result_snip;

    bool ok = MeowCryptoUtils::ccTransform(public_param, computed_res, obfuscated_computed_res,
                                           obfuscated_random_result_snip);

    if (!ok)
        qCritical("Error occurred when Computing Center obfuscating client data!");

    QJsonObject master_transform_back_request_data;
    master_transform_back_request_data["encrypted_pair_a"] = obfuscated_computed_res.A();
    master_transform_back_request_data["encrypted_pair_b"] = obfuscated_computed_res.B();
    QJsonObject master_transform_back_request_data_r;
    master_transform_back_request_data_r["data"] = master_transform_back_request_data;

    auto master_transform_back_data = sync_post(&mgr, this->master_address_ + "/transform-back",
                                                QJsonDocument(
                                                        master_transform_back_request_data_r)).object()["data"].toArray();

    QMap<QString, MeowCryptoUtils::EncryptedPair> master_transform_back_ciphertext;

    for (auto i: master_transform_back_data) {
        auto arr = i.toArray();
        MeowCryptoUtils::EncryptedPair enc;
        enc.setPublicN(public_param.N());
        enc.setPublicKey(this->pk_map_[arr[0].toString()]);
        enc.setA(arr[1].toString());
        enc.setB(arr[2].toString());
        enc = enc + MeowCryptoUtils::encrypt(public_param, this->pk_map_[arr[0].toString()],
                                             "-" + obfuscated_random_result_snip);

        master_transform_back_ciphertext[arr[0].toString()] = enc;
    }

    for (auto &i: this->pk_map_.keys()) {
        QJsonObject final_result;
        final_result["encrypted_pair_a"] = master_transform_back_ciphertext[i].A();
        final_result["encrypted_pair_b"] = master_transform_back_ciphertext[i].B();
        qInfo("Computing Client[%s] Result: a=%s, b=%s",
              i.toStdString().c_str(),
              master_transform_back_ciphertext[i].A().toStdString().c_str(),
              master_transform_back_ciphertext[i].B().toStdString().c_str());
        QJsonObject final_result_r;
        final_result_r["data"] = final_result;
        sync_post(&mgr, this->clients_address_[i] + "/set-result", QJsonDocument(final_result_r));
    }

    qWarning("==> SYNC COMPUTING ENDED <==");

    return HttpPromise::resolve(data);
}

void MainHandler::clearData() {
    this->pk_map_.clear();
}
