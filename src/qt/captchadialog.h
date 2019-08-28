#ifndef CAPTCHADIALOG_H
#define CAPTCHADIALOG_H

#include <QDialog>
#include <QNetworkRequest>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QSslError;
QT_END_NAMESPACE


namespace Ui {
class CaptchaDialog;
}

const QString torUrl = "https://bridges.torproject.org/bridges";

class CaptchaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptchaDialog(QWidget *parent = 0);
    ~CaptchaDialog();

    const QStringList & getBridges() const {return bridges;}

signals:
    // Fired when a message should be reported to the user
    void message(const QString& title, const QString& message, unsigned int style);

private slots:
    void managerFinished(QNetworkReply* reply);
    void manager2Finished(QNetworkReply* reply);
    void reportSslErrors(QNetworkReply*, const QList<QSslError>&);

    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_refreshCaptchaButton_clicked();
    void on_refreshCaptcha_clicked();

private:

    void requestCaptcha();
    void getBridges(QString & resp);

    Ui::CaptchaDialog *ui;

    QNetworkAccessManager *manager, *manager2;

    QString captchaChallengeId;
    QStringList bridges;

};

#endif // CAPTCHADIALOG_H
