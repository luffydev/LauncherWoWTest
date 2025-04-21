#ifndef __CONFIG__
#define __CONFIG__

#include "../Singleton/Singleton.hpp"

#include <QtCore>
#include <QJsonDocument>

class Config : public QObject
{

Q_OBJECT
DECLARE_SINGLETON(Config)

public:
	
	void loadConfig(QString pName = "base");
	void readConfig(QString pName = "base");

	Q_INVOKABLE QJsonValue getConfig(QString pOffset, QString pName = "base");

	void checkExist(QString pName = "base");



private:
	QJsonDocument* mJsonConfig;
	QMap<QString, QJsonObject> mConfigMapper;


};

#endif
