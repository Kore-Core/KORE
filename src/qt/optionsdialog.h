// Copyright (c) 2011-2013 The Bitcoin developers
// Copyright (c) 2017-2018 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_OPTIONSDIALOG_H
#define BITCOIN_QT_OPTIONSDIALOG_H

#include <QDialog>
//#include <QNetworkRequest>

class OptionsModel;
class QValidatedLineEdit;
class QLineEdit;

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui
{
class OptionsDialog;
}

/** Preferences dialog. */
class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget* parent, bool enableWallet);
    ~OptionsDialog();

    void setModel(OptionsModel* model);
    void setMapper();
    bool isRestartRequired();
    bool restartNoQuestions() {return fRestartNoMoreQuestions;}

protected:
    bool eventFilter(QObject* object, QEvent* event);

private slots:
    /* enable OK button */
    void enableOkButton();
    /* disable OK button */
    void disableOkButton();
    /* set OK button state (enabled / disabled) */
    void setOkButtonState(bool fState);
    void on_resetButton_clicked();
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_retrieveNewBridgespushButton_clicked();
    void on_enableObfs4checkBox_clicked();

    void showRestartWarning(bool fPersistent = false);
    void clearStatusLabel();
    void doProxyIpChecks(QValidatedLineEdit* pUiProxyIp, QLineEdit* pUiProxyPort);    

signals:
    void proxyIpChecks(QValidatedLineEdit* pUiProxyIp, QLineEdit* pUiProxyPort);

private:

    Ui::OptionsDialog* ui;
    OptionsModel* model;
    QDataWidgetMapper* mapper;
    bool fProxyIpValid;
    bool fRestartNoMoreQuestions;
    bool obfs4EnabledCurrent;
    QStringList newObfs4Bridges;
    QStringList oldObfs4Bridges;
    QByteArray torrcWithoutBridges;

};

#endif // BITCOIN_QT_OPTIONSDIALOG_H
