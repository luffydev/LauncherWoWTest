#include "Config.hpp"

void Config::loadConfig(QString pName) {

}

void Config::readConfig(QString pName) {
	
	QString lAppDirectory = QCoreApplication::applicationDirPath();
	QFile lConfFile(lAppDirectory + "/config/" + pName + ".json");

	if (!lConfFile.exists() || !lConfFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

		qDebug() << "unable to open " << pName << ".json";
		return;
	}

	QByteArray lJson = lConfFile.readAll();

	QJsonParseError lError;
	QJsonDocument lDocument = QJsonDocument::fromJson(lJson, &lError);

	if (lError.error == QJsonParseError::NoError) {

		QJsonObject lObject = lDocument.object();

		mConfigMapper.insert(pName, lObject);
		return;
	}

	qDebug() << "Error while parsing JSON config " << pName << ".json " << lError.errorString();

}

void Config::checkExist(QString pName) {

	// check if config directory exist
	QString lAppDirectory = QCoreApplication::applicationDirPath();
	QDir lPath(lAppDirectory + "/config/");

	if (!lPath.exists()){
		lPath.mkpath(".");
	}

	QFile lFile(lAppDirectory + "/config/" + pName + ".json");

	if (!lFile.exists()) {
		QFile lRawFile = QFile(":/config/"+ pName +".json");

		if (lRawFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QByteArray lConfFile = lRawFile.readAll();
			QFile lDistFile = QFile(lAppDirectory + "/config/" + pName + ".json");

			lDistFile.open(QIODevice::WriteOnly);
			lDistFile.write(lConfFile);

			qDebug() << "[Config] -> conf file " << lAppDirectory << "/config/" << pName << ".json created successfully";
		}
	}

}


QJsonValue Config::getConfig(QString pOffset, QString pName) {

	if (!mConfigMapper.contains(pName)){

		qDebug() << "Can't find " << pName << " in mapper, did you parse file before ?";
		return QJsonValue();
	}

	QJsonObject lObject = mConfigMapper[pName];

	if (!lObject.contains(pOffset)) {
		qDebug() << "Unable to fin offset " << pOffset << " in JSON : " << pName;
		return QJsonValue();
	}

	return lObject[pOffset];

	
}