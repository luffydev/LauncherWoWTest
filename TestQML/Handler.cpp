#include "Handler.hpp"

void Handler::onConnectionClicked(QString pUsername, QString pPassword) {
	
	QMetaObject::invokeMethod(mQMLObject, "resetLoginError");

	QJsonObject userData;
	userData["username"] = pUsername;
	userData["password"] = pPassword;

	// Ajouter l'empreinte et convertir en chaîne JSON
	QString jsonData = QString("__ENCRYPTED_DATA__") + QJsonDocument(userData).toJson();

	// Mot de passe secret pour le chiffrement (doit être identique côté PHP)
	QByteArray secretKey = AES_ENCRYPT_KEY;

	// Chiffrer les données avec PBKDF2 et AES-GCM
	QByteArray encryptedData = AESGCM::encryptForPHP(jsonData.toUtf8(), secretKey);

	APIHandler::POST("http://192.168.179.81/LauncherUpdate/api_login.php", {
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

void Handler::showErrorMessage(QString pTitle, QString pMessage) {
	QMetaObject::invokeMethod(mQMLObject, "displayMessage", QVariant(pTitle), QVariant(pMessage));
}

void Handler::onCheckForUpdate() {
	qDebug() << "Check for update !!!";

	if (mQMLObject) {

		getUpdateFileInfo();

		QMetaObject::invokeMethod(mQMLObject, "onCheckUpdateFinished");

		mMainContainer = mQMLObject->findChild<QObject*>("mainContainer");
		mProgressBar = mQMLObject->findChild<QObject*>("loadingBar");
		mDownloadSpeed = mQMLObject->findChild<QObject*>("downloadSpeed");
		mMainContainer->setProperty("state", QVariant("updating"));
	}
}

void Handler::getUpdateFileInfo() {

	qDebug() << "Get update file info !!!";

	const QUrl lUrl("http://192.168.179.81/LauncherUpdate/file_info.php");
	QNetworkRequest lRequest(lUrl);

	lRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");
	lRequest.setRawHeader("Accept", "application/json");

	QNetworkReply* lReply = mNetworkManager->get(lRequest);

	connect(lReply, &QNetworkReply::finished, this, [this, lReply]() {

		if (lReply->error() == QNetworkReply::NoError)
		{
			QByteArray lData = lReply->readAll();
			QJsonParseError lError;

			QJsonDocument lDocument = QJsonDocument::fromJson(lData);

			qDebug() << " Recv Update file size : " << lDocument["file_size"].toInteger();

			mUpdateName = lDocument["file_name"].toString();
			QNetworkRequest lRequest(lDocument["file_url"].toString());


			mMainContainer->setProperty("state", QVariant("updating"));

			QNetworkReply* lReply = mNetworkManager->get(lRequest);

			QString lFile = QDir::currentPath() + "/update/" + mUpdateName;

			if (QFile::exists(lFile)) {
				QFile::remove(lFile);
			}

			mDestinationFile = new QFile(QDir::currentPath() + "/update/" + mUpdateName);

			if (!mDestinationFile->open(QIODevice::WriteOnly)) {
				showErrorMessage("Error", "Unable to open local update file for writing, error : " + mDestinationFile->errorString());
				return;
			}

			connect(lReply, &QNetworkReply::downloadProgress, this, &Handler::downloadUpdateProgress);
			connect(lReply, &QNetworkReply::readyRead, this, &Handler::readyRead);
			connect(mNetworkManager, &QNetworkAccessManager::finished, this, &Handler::downloadFinished);
		}
		else
			showErrorMessage("Error", "Unable to connect to dist server, error : " + lReply->errorString());

		lReply->deleteLater(); // Clean up the reply object
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

void Handler::setQmlObject(QObject* pObject) {
	mQMLObject = pObject;
}