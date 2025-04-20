#ifndef HANDLER_H  
#define HANDLER_H  

#include <QtCore>
#include <QRandomGenerator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

#include "SDK/Network/ApiHandler.hpp"
#include "SDK/Cryptographic/AES/AES.hpp"

#define BASE_URL "http://192.168.179.81"

#define UPDATE_INFO_URL	BASE_URL  ##  "/LauncherUpdate/file_info.php"
#define LOGIN_API_URL BASE_URL  ##  "LauncherUpdate/api_login.php"


class Handler : public QObject {  
   Q_OBJECT  
public:  
   explicit Handler(QObject* parent = nullptr) : QObject(parent), mNetworkManager(new QNetworkAccessManager()) {}

   Q_INVOKABLE void onConnectionClicked(QString pUsername, QString pPassword);  
   Q_INVOKABLE void onCheckForUpdate();  

   void setQmlObject(QObject* pObject);  

private:
	void getUpdateFileInfo();
	void showErrorMessage(QString pTitle, QString pMessage, QString pType = "critical");

private slots:
	void downloadUpdateProgress(qint64 pReceived, qint64 pTotal);
	void readyRead();
	void downloadFinished(QNetworkReply* data);

private:  
   QObject* mQMLObject, *mMainContainer, *mProgressBar, *mDownloadSpeed, *mSpinnerText;
   QNetworkAccessManager* mNetworkManager;
   qint64 mLastReceived = 0;
   QElapsedTimer mTimer;
   QString mUpdateName;
   QFile *mDestinationFile;
};  

#endif // HANDLER_H