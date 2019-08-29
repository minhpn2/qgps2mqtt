#include <QCoreApplication>
#include "mainmodel.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    mainModel model;
    model.start();

    return app.exec();
}
