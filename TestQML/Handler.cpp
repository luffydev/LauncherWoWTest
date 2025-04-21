#include "Handler.hpp"
#include "SDK/Config/Config.hpp"


void Handler::showErrorMessage(QString pTitle, QString pMessage, QString pType) {
	QMetaObject::invokeMethod(mQMLObject, "displayMessage", QVariant(pTitle), QVariant(pMessage), QVariant(pType));
}

void Handler::onCheckForUpdate() {

	qDebug() << "Check for update !!!";

	if (mQMLObject) {

		getUpdateFileInfo();

		QMetaObject::invokeMethod(mQMLObject, "onCheckUpdateFinished");

		mMainContainer = mQMLObject->findChild<QObject*>("mainContainer");
		mProgressBar = mQMLObject->findChild<QObject*>("loadingBar");
		mDownloadSpeed = mQMLObject->findChild<QObject*>("downloadSpeed");
		mSpinnerText = mQMLObject->findChild<QObject*>("spinnerText");
		mMainContainer->setProperty("state", QVariant("updating"));
	}

}

void Handler::getUpdateFileInfo() {

	qDebug() << "Get update file info !!!";

	QJsonObject lApi = Config::instance().getConfig("api").toObject();

	APIHandler::GET(lApi["url"].toString() + lApi["update_endpoint"].toString(), {
	{"version", Config::instance().getConfig("current_version").toString()}}, 
	[this, lApi](QJsonDocument pDocument, bool pHasError, QString pErrorString) 
	{

		if (pHasError) {
			showErrorMessage("Error", "Unable to connect to dist server, error : " + pErrorString);
			return;
		}

		mSpinnerText->setProperty("text", "Downloading update...");
		mUpdateName = pDocument["file_name"].toString();

		QNetworkRequest lRequest(pDocument["file_url"].toString());
		mMainContainer->setProperty("state", QVariant("updating"));

		QNetworkReply* lReply = mNetworkManager->get(lRequest);
		QString lFile = QDir::currentPath() + "/update/" + mUpdateName;

		if (QFile::exists(lFile)) 
			QFile::remove(lFile);

		mDestinationFile = new QFile(QDir::currentPath() + "/update/" + mUpdateName);

		if (!mDestinationFile->open(QIODevice::WriteOnly)) {
			showErrorMessage("Error", "Unable to open local update file for writing, error : " + mDestinationFile->errorString());
			return;
		}

		connect(lReply, &QNetworkReply::downloadProgress, this, &Handler::downloadUpdateProgress);
		connect(lReply, &QNetworkReply::readyRead, this, &Handler::readyRead);
		connect(mNetworkManager, &QNetworkAccessManager::finished, this, &Handler::downloadFinished);

	});
}

void Handler::downloadUpdateProgress(qint64 pReceived, qint64 pTotal) {

	qDebug() << "RECEIVED : " << pReceived << " TOTAL : " << pTotal;

	double lPercent = round(((double)pReceived / (double)pTotal) * 100.0);

	if (!mTimer.isValid()) {
		mTimer.start(); // Démarre le timer la première fois
	}

	qint64 lElapsed = mTimer.elapsed();
	qint64 lDeltaBytes = pReceived - mLastReceived;

	double lSpeedBytes = 0.0;

	if (lElapsed > 0)
		lSpeedBytes = (double)lDeltaBytes / ((double)lElapsed / 1000.0);

	mLastReceived = pReceived;
	mTimer.restart(); // Redémarre le timer

	QString lSpeedStr;

	if (lSpeedBytes > 1024 * 1024)
		lSpeedStr = QString::number(lSpeedBytes / (1024.0 * 1024.0), 'f', 2) + " Mo/s";
	else if (lSpeedBytes > 1024.0) {
		lSpeedStr = QString::number(lSpeedBytes / 1024.0, 'f', 2) + " Ko/s";
	}
	else {
		lSpeedStr = QString::number(lSpeedBytes, 'f', 2) + " o/s";
	}


	mProgressBar->setProperty("progress", QVariant(lPercent));
	mDownloadSpeed->setProperty("text", QVariant(lSpeedStr));

	qDebug() << "RECEIVED:" << pReceived << "/" << pTotal
		<< "| Percent:" << lPercent << "%"
		<< "| Speed:" << lSpeedStr;

}

void Handler::readyRead() {
	QNetworkReply* lReply = qobject_cast<QNetworkReply*>(sender());
	QByteArray lData = lReply->readAll();

	mDestinationFile->write(lData);
}

void Handler::downloadFinished(QNetworkReply* data) {

	mDestinationFile->close();
	mDestinationFile->deleteLater();
	if (data->error() != QNetworkReply::NoError) {
		showErrorMessage("Error while downloading", "An error occured while downloading update, error : " + data->errorString());
		return;
	}
	qDebug() << "Download finished successfully!";

	mMainContainer->setProperty("state", QVariant("idle"));
}

void Handler::onConnectionClicked(QString pUsername, QString pPassword) {

	QMetaObject::invokeMethod(mQMLObject, "resetLoginError");

	QJsonObject userData;
	userData["username"] = pUsername;
	userData["password"] = pPassword;

	// Ajouter l'empreinte et convertir en chaîne JSON
	QString jsonData = QString("__ENCRYPTED_DATA__") + QJsonDocument(userData).toJson();

	QJsonObject lApi = Config::instance().getConfig("api").toObject();

	// Mot de passe secret pour le chiffrement (doit être identique côté PHP)
	QByteArray secretKey = AES_ENCRYPT_KEY;

	// Chiffrer les données avec PBKDF2 et AES-GCM
	QByteArray encryptedData = AESGCM::encryptForPHP(jsonData.toUtf8(), secretKey);

	APIHandler::POST(lApi["url"].toString() + lApi["login_endpoint"].toString(), {
		{"data", encryptedData}
		}, [this](QJsonDocument pDocument) {

			if (pDocument.isNull()) {
				qDebug() << "Error: Invalid JSON document.";

				mMainContainer->setProperty("state", QVariant("idle"));
				QMetaObject::invokeMethod(mQMLObject, "showLoginError", QVariant("Internal Error \r\n Please try again later"));

				return;
			}

			qDebug() << "Login response: " << pDocument.toJson();

			if (pDocument["status"].toString() == "success") {
				qDebug() << "Login successful!";
				mQMLObject->setProperty("loginStatus", QVariant("success"));

				QMetaObject::invokeMethod(mQMLObject, "setLoginSucceed");

			}
			else {
				qDebug() << "Login failed!";
				mQMLObject->setProperty("loginStatus", QVariant("failed"));

				mMainContainer->setProperty("state", QVariant("idle"));
				QMetaObject::invokeMethod(mQMLObject, "showLoginError", QVariant("Invalid Credential !"));
			}

		});

}

void Handler::setQmlObject(QObject* pObject) {
	mQMLObject = pObject;
}