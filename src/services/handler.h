//
// Created by reverier on 2021/10/21.
//

#ifndef MEOWCOMPUTINGCENTER_HANDLER_H
#define MEOWCOMPUTINGCENTER_HANDLER_H

#include "http/httpRequestRouter.h"
#include "http/httpRequestHandler.h"
#include "utils/crypto_utils.h"

class MainHandler : public HttpRequestHandler {
private:
    HttpRequestRouter router_;
    QString master_address_{};
    QMap<QString, QString> clients_address_{};
    QMap<QString, QString> pk_map_{};

public:
    MainHandler();

    HttpPromise handle(HttpDataPtr data) override;

    HttpPromise handleRegister(HttpDataPtr data);

    HttpPromise handleRegisterMaster(HttpDataPtr data);

    HttpPromise handleComputingRequest(HttpDataPtr data);

    void clearData();
};


#endif //MEOWCOMPUTINGCENTER_HANDLER_H
