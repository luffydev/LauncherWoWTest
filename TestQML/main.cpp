#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "Handler.hpp"
#include "SDK/Config/Config.hpp"

int main(int argc, char *argv[])
{
   QGuiApplication app(argc, argv);

   // Check if our base config exist 
   Config::instance().checkExist();

   //Read our base config
   Config::instance().readConfig();

   QQmlApplicationEngine engine;
   engine.addImportPath("qml"); // Permet de charger les composants

   QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

   Handler handler;

   engine.rootContext()->setContextProperty("handler", &handler);
   qmlRegisterSingletonInstance("Launcher.Config", 1, 0, "Config", &Config::instance());


   engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

   if (engine.rootObjects().isEmpty())
       return -1;

   QObject* rootObject = engine.rootObjects().first();
   handler.setQmlObject(rootObject);

   handler.onCheckForUpdate();

   return app.exec();
}