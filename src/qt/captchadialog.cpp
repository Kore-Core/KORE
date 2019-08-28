#include "captchadialog.h"
#include "ui_captchadialog.h"
#include "ui_interface.h"


#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>


CaptchaDialog::CaptchaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaptchaDialog), manager2(NULL)
{
    ui->setupUi(this);

    ui->refreshCaptchaButton->setIcon(QIcon(":/icons/refresh").pixmap(16, 16));

    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(managerFinished(QNetworkReply*)));
    connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
        this, SLOT(reportSslErrors(QNetworkReply*, const QList<QSslError>&)));


    //QPushButton *okButton = ui->okButton;
    //connect(okButton , SIGNAL(accepted()), this, SLOT(on_OkButton_clicked()));
    connect( ui->okButton, SIGNAL(clicked(bool)), this, SLOT(on_OkButton_clicked()));


    requestCaptcha();
}

CaptchaDialog::~CaptchaDialog()
{
    delete ui;
    if (manager != NULL)
        delete manager;
    if (manager2 != NULL)
        delete manager2;
}

void CaptchaDialog::managerFinished(QNetworkReply* reply)
{    
    this->setCursor(Qt::ArrowCursor);
    captchaChallengeId.clear();
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QLabel *captchaLabel = ui->captchaLabel;

    QString answer = reply->readAll();

    QRegExp captchaChallenge("id=\"captcha_challenge_field\"\n[ ]+value=\"([^\"]+)\"");
    if (captchaChallenge.indexIn(answer,0) != -1) {
        captchaChallengeId = captchaChallenge.cap(1);         
    }

    QRegExp regExpr("src=\"data:image/jpeg;base64,([^\"]+)\"");
    int pos=0;
    if (regExpr.indexIn(answer,pos) != -1) {
        // found the captcha
        QString imgStr = regExpr.cap(1);        
        QPixmap  pixmap;
        if (pixmap.loadFromData(QByteArray::fromBase64(imgStr.toUtf8()), "JPEG")) {
            captchaLabel->setPixmap(pixmap); 
            captchaLabel->setAlignment(Qt::AlignHCenter);
            captchaLabel->setScaledContents(true);
        } else {
            captchaLabel->setText("Could not read the captcha image");
        }

    } else {
        captchaLabel->setText("Could not find the captcha image");
    }
}

void CaptchaDialog::getBridges(QString & resp)
{
    QRegExp bridgesExpr("(obfs4[^\\<]+)");
    int pos = bridgesExpr.indexIn(resp);
    if (pos == -1) {
        // wrong captcha. request a new one
        requestCaptcha();        
    } else {
        bridges = bridgesExpr.capturedTexts();
        for ( auto & bridge : bridges) {
            bridge.insert(0, "bridge ");
        }

        // cool the user solved the captcha and we have the bridges
        // we can close this windows
        accept();
    }
}

void CaptchaDialog::manager2Finished(QNetworkReply* reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QLabel *captchaLabel = ui->captchaLabel;

    QString answer = reply->readAll();
    getBridges(answer);
}

void CaptchaDialog::reportSslErrors(QNetworkReply* reply, const QList<QSslError>& errs)
{
    this->setCursor(Qt::ArrowCursor);
    Q_UNUSED(reply);

    QString errString;
    Q_FOREACH (const QSslError& err, errs) {
        errString += err.errorString() + "\n";
    }
    emit message(tr("Network request error"), errString, CClientUIInterface::MSG_ERROR);
}

void CaptchaDialog::on_okButton_clicked()
{
    QString captchaTyped = ui->captchaEdit->text();

    if (captchaTyped.isEmpty()) {
        //this->setCursor(Qt::ArrowCursor);
        QMessageBox::warning(this, windowTitle(),
                tr("Please enter the captcha"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    this->setCursor(Qt::WaitCursor);

    // Lets configure a second manager
    manager2 = new QNetworkAccessManager(this);
    connect(manager2, SIGNAL(finished(QNetworkReply*)), this, SLOT(manager2Finished(QNetworkReply*)));
    connect(manager2, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
        this, SLOT(reportSslErrors(QNetworkReply*, const QList<QSslError>&)));

    // lets do the put request
    QNetworkRequest request;
    QUrl url(torUrl);
    QUrlQuery query;
    query.addQueryItem("transport","obfs4");
    query.addQueryItem("captcha_challenge_field",captchaChallengeId);
    query.addQueryItem("captcha_response_field",captchaTyped);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);

    manager2->post(request, query.toString(QUrl::FullyEncoded).toUtf8());        
}

void CaptchaDialog::on_cancelButton_clicked()
{
    reject();
}


void CaptchaDialog::requestCaptcha()
{
    QNetworkRequest request;

    // lets do the get request
    this->setCursor(Qt::WaitCursor);
    ui->captchaEdit->clear();
    request.setUrl(QUrl(torUrl));
    manager->get(request);
}

void CaptchaDialog::on_refreshCaptchaButton_clicked()
{
    requestCaptcha();
}

void CaptchaDialog::on_refreshCaptcha_clicked()
{
    requestCaptcha();
}
