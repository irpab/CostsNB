#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStringList>
#include "costs_nb_qml_proxy.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    Costs_nb_qml_proxy costs_nb_qml_proxy(engine.rootContext());
    engine.rootContext()->setContextProperty("costs_nb_qml_proxy", &costs_nb_qml_proxy);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
