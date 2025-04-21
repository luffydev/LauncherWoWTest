#include "ApiHandler.hpp"


void APIHandler::GET(const QString& url, const QMap<QString, QString>& params, const std::function<void(QJsonDocument, bool, QString)>& pCallback)
{
   QUrl lUrl(url);
   QUrlQuery lQuery;

   QNetworkAccessManager* lNetworkManager = new QNetworkAccessManager();

   for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
       lQuery.addQueryItem(it.key(), it.value());
   }

   lUrl.setQuery(lQuery.query());
   QNetworkRequest lRequest(lUrl);

   lRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");
   lRequest.setRawHeader("Accept", "application/json");

   QNetworkReply* lReply = lNetworkManager->get(lRequest);

   connect(lReply, &QNetworkReply::finished, [lReply, pCallback]() {

	   if (lReply->error() == QNetworkReply::NoError) {

		   QByteArray lData = lReply->readAll();
		   QJsonParseError lError;
		   QJsonDocument lDocument = QJsonDocument::fromJson(lData, &lError);

		   if (lError.error == QJsonParseError::NoError) {
			   pCallback(lDocument, false, "");
		   }
		   else {

			   pCallback(QJsonDocument(), true, lError.errorString());
			   qDebug() << "JSON Parse Error: " << lError.errorString();
		   }
	   }
	   else {

		   pCallback(QJsonDocument(), true, lReply->errorString());
		   qDebug() << "Network Error: " << lReply->errorString();

	   }

	   lReply->deleteLater(); // Clean up the reply object

	   });
}

void APIHandler::POST(const QString& url, const QMap<QString, QString>& params, const std::function<void(QJsonDocument)>& pCallback) {

	QUrl lUrl(url);
	QUrlQuery lParams;
	QNetworkAccessManager* lNetworkManager = new QNetworkAccessManager();
	QNetworkRequest lRequest(lUrl);

	lRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");
	lRequest.setRawHeader("Accept", "application/json");
	lRequest.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

	for (auto it = params.constBegin(); it != params.constEnd(); ++it) {	
		lParams.addQueryItem(it.key(), it.value());
	}

	lNetworkManager->post(lRequest, lParams.query(QUrl::FullyEncoded).toUtf8());

	connect(lNetworkManager, &QNetworkAccessManager::finished, [=](QNetworkReply* lReply) {

		if (lReply->error() == QNetworkReply::NoError) {

			QByteArray lData = lReply->readAll();
			QJsonParseError lError;
			QJsonDocument lDocument = QJsonDocument::fromJson(lData, &lError);

			if (lError.error == QJsonParseError::NoError) {
				pCallback(lDocument);
			}
			else {
				qDebug() << "JSON Parse Error: " << lError.errorString();
			}
		}
		else 
			qDebug() << "Network Error: " << lReply->errorString();

		lReply->deleteLater(); // Clean up the reply object

	});
}