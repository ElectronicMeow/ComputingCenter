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

public:
    MainHandler();

    HttpPromise handle(HttpDataPtr data) override;
};


#endif //MEOWCOMPUTINGCENTER_HANDLER_H
