//
// Created by Reverier-Xu on 2021/10/19.
//

#include <QCoreApplication>
#include <QMessageLogger>
#include "initializations/logger.h"
#include "initializations/server.h"


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    initLogger();

    qInfo("-*- Welcome to Electronic Meow Project! -*-");
    qWarning("This Server is Client Server. deploy it on Power Utility's database server.");
    qInfo("Booting Server...");

    /// Server Initializations
    auto server = initServer();

    server->listen();

    return QCoreApplication::exec();
}
