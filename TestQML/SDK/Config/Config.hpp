#ifndef __CONFIG__
#define __CONFIG__

#include "../Singleton/Singleton.hpp"

#include <QtCore>
#include <QJsonDocument>

class Config 
{
DECLARE_SINGLETON(Config)

public:
	void loadConfig(QString pName);

private:
	QJsonDocument* mJsonConfig;


};

#endif
