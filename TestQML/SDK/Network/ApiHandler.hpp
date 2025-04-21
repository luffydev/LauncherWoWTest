#ifndef __API_HANDLER_H__
#define __API_HANDLER_H__

#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

#include "../Cryptographic/AES/AES.hpp"


class APIHandler : public QObject
{
	Q_OBJECT

public:
	explicit APIHandler(QObject* parent = nullptr) : QObject(parent) {}

	static void GET(const QString& url, const QMap<QString, QString>& params, const std::function<void(QJsonDocument, bool, QString)>& pCallback);
	static void POST(const QString& url, const QMap<QString, QString>& params, const std::function<void(QJsonDocument)>& callback);
	//static void PUT(const QString& url, const QJsonObject& params, const std::function<void>& callback);

private:
};

#endif