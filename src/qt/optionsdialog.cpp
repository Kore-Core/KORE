// Copyright (c) 2011-2013 The Bitcoin developers
// Copyright (c) 2017-2018 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "bitcoinunits.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "captchadialog.h"

#include "main.h" // for MAX_SCRIPTCHECK_THREADS
#include "netbase.h"
#include "txdb.h" // for -dbcache defaults

#ifdef ENABLE_WALLET
#include "wallet.h" // for CWallet::minTxFee
#endif

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <QDataWidgetMapper>
#include <QDebug>
#include <QDir>
#include <QIntValidator>
#include <QLocale>
#include <QMessageBox>
#include <QRegExp>
#include <QTimer>

namespace fs = boost::filesystem;

OptionsDialog::OptionsDialog(QWidget* parent, bool enableWallet) : QDialog(parent),
                                                                   ui(new Ui::OptionsDialog),
                                                                   model(0),
                                                                   mapper(0),
                                                                   fProxyIpValid(true),
                                                                   fRestartNoMoreQuestions(false)
{
    ui->setupUi(this);
    GUIUtil::restoreWindowGeometry("nOptionsDialogWindow", this->size(), this);

    /* Main elements init */
    ui->databaseCache->setMinimum(nMinDbCache);
    ui->databaseCache->setMaximum(nMaxDbCache);
    ui->threadsScriptVerif->setMinimum(-(int)boost::thread::hardware_concurrency());
    ui->threadsScriptVerif->setMaximum(MAX_SCRIPTCHECK_THREADS);

/* Network elements init */
#ifndef USE_UPNP
    ui->mapPortUpnp->setEnabled(false);
#endif

    ui->proxyIp->setEnabled(false);
    ui->proxyPort->setEnabled(false);
    ui->proxyPort->setValidator(new QIntValidator(1, 65535, this));


    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->proxyIp, SLOT(setEnabled(bool)));
    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->proxyPort, SLOT(setEnabled(bool)));

    ui->proxyIp->installEventFilter(this);
    ui->proxyPort->installEventFilter(this);

/* Window elements init */
#ifdef Q_OS_MAC
    /* remove Window tab on Mac */
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabWindow));
#endif

    /* remove Wallet tab in case of -disablewallet */
    if (!enableWallet) {
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabWallet));
    }

    /* Display elements init */

    /* Number of displayed decimal digits selector */
    QString digits;
    for (int index = 2; index <= 8; index++) {
        digits.setNum(index);
        ui->digits->addItem(digits, digits);
    }

    /* Theme selector static themes */
    ui->theme->addItem(QString("Default"), QVariant("default"));

    /* Theme selector external themes */
    boost::filesystem::path pathAddr = GetDataDir() / "themes";
    QDir dir(pathAddr.string().c_str());
    dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        ui->theme->addItem(fileInfo.fileName(), QVariant(fileInfo.fileName()));
    }

    /* Language selector */
    QDir translations(":translations");
    ui->lang->addItem(QString("(") + tr("default") + QString(")"), QVariant(""));
    Q_FOREACH (const QString& langStr, translations.entryList()) {
        QLocale locale(langStr);

        /** check if the locale name consists of 2 parts (language_country) */
        if (langStr.contains("_")) {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language - native country (locale name)", e.g. "Deutsch - Deutschland (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" - ") + locale.nativeCountryName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language - country (locale name)", e.g. "German - Germany (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" - ") + QLocale::countryToString(locale.country()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        } else {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language (locale name)", e.g. "Deutsch (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language (locale name)", e.g. "German (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
    }
#if QT_VERSION >= 0x040700
    ui->thirdPartyTxUrls->setPlaceholderText("https://example.com/tx/%s");
#endif

    ui->unit->setModel(new BitcoinUnits(this));

    /* Widget-to-option mapper */
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);

    /* setup/change UI elements when proxy IP is invalid/valid */
    connect(this, SIGNAL(proxyIpChecks(QValidatedLineEdit*, QLineEdit*)), this, SLOT(doProxyIpChecks(QValidatedLineEdit*, QLineEdit*)));

    /* show current obfs4 bridges */
    obfs4EnabledCurrent = GetBoolArg("-obfs4", false);
    qDebug() << "obfs4EnabledCurrent : " << obfs4EnabledCurrent;
    ui->enableObfs4checkBox->setCheckState(obfs4EnabledCurrent ? Qt::Checked : Qt::Unchecked);
    ui->retrieveNewBridgespushButton->setVisible(obfs4EnabledCurrent);
    ui->bridgesGroup->setEnabled(obfs4EnabledCurrent);
    oldObfs4Bridges = GUIUtil::retrieveBridgesFromTorrc(torrcWithoutBridges);

    ui->currentBridgesTextEdit->setPlainText(oldObfs4Bridges.join("\n"));

}

OptionsDialog::~OptionsDialog()
{
    GUIUtil::saveWindowGeometry("nOptionsDialogWindow", this);
    delete ui;
}

void OptionsDialog::setModel(OptionsModel* model)
{
    this->model = model;

    if (model) {
        /* check if client restart is needed and show persistent message */
        if (model->isRestartRequired())
            showRestartWarning(true);

        QString strLabel = model->getOverriddenByCommandLine();
        if (strLabel.isEmpty())
            strLabel = tr("none");
        ui->overriddenByCommandLineLabel->setText(strLabel);

        mapper->setModel(model);
        setMapper();
        mapper->toFirst();
    }

    /* warn when one of the following settings changes by user action (placed here so init via mapper doesn't trigger them) */

    /* Main */
    connect(ui->databaseCache, SIGNAL(valueChanged(int)), this, SLOT(showRestartWarning()));
    connect(ui->threadsScriptVerif, SIGNAL(valueChanged(int)), this, SLOT(showRestartWarning()));
    /* Wallet */
    connect(ui->spendZeroConfChange, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    /* Network */
    connect(ui->allowIncoming, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    connect(ui->connectSocks, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning()));
    /* Display */
    connect(ui->digits, SIGNAL(valueChanged()), this, SLOT(showRestartWarning()));
    connect(ui->theme, SIGNAL(valueChanged()), this, SLOT(showRestartWarning()));
    connect(ui->lang, SIGNAL(valueChanged()), this, SLOT(showRestartWarning()));
    connect(ui->thirdPartyTxUrls, SIGNAL(textChanged(const QString&)), this, SLOT(showRestartWarning()));
}

bool OptionsDialog::isRestartRequired()
{
    return model && model->isRestartRequired();
}

void OptionsDialog::setMapper()
{
    /* Main */
    mapper->addMapping(ui->bitcoinAtStartup, OptionsModel::StartAtStartup);
    mapper->addMapping(ui->threadsScriptVerif, OptionsModel::ThreadsScriptVerif);
    mapper->addMapping(ui->databaseCache, OptionsModel::DatabaseCache);

    /* Wallet */
    mapper->addMapping(ui->spendZeroConfChange, OptionsModel::SpendZeroConfChange);
    mapper->addMapping(ui->coinControlFeatures, OptionsModel::CoinControlFeatures);

    /* Network */
    mapper->addMapping(ui->mapPortUpnp, OptionsModel::MapPortUPnP);
    mapper->addMapping(ui->allowIncoming, OptionsModel::Listen);

    mapper->addMapping(ui->connectSocks, OptionsModel::ProxyUse);
    mapper->addMapping(ui->proxyIp, OptionsModel::ProxyIP);
    mapper->addMapping(ui->proxyPort, OptionsModel::ProxyPort);

    /* Window */
#ifndef Q_OS_MAC
    mapper->addMapping(ui->minimizeToTray, OptionsModel::MinimizeToTray);
    mapper->addMapping(ui->minimizeOnClose, OptionsModel::MinimizeOnClose);
#endif

    /* Display */
    mapper->addMapping(ui->digits, OptionsModel::Digits);
    mapper->addMapping(ui->theme, OptionsModel::Theme);
    mapper->addMapping(ui->theme, OptionsModel::Theme);
    mapper->addMapping(ui->lang, OptionsModel::Language);
    mapper->addMapping(ui->unit, OptionsModel::DisplayUnit);
    mapper->addMapping(ui->thirdPartyTxUrls, OptionsModel::ThirdPartyTxUrls);
}

void OptionsDialog::enableOkButton()
{
    /* prevent enabling of the OK button when data modified, if there is an invalid proxy address present */
    if (fProxyIpValid)
        setOkButtonState(true);
}

void OptionsDialog::disableOkButton()
{
    setOkButtonState(false);
}

void OptionsDialog::setOkButtonState(bool fState)
{
    ui->okButton->setEnabled(fState);
}

void OptionsDialog::on_resetButton_clicked()
{
    if (model) {
        // confirmation dialog
        QMessageBox::StandardButton btnRetVal = QMessageBox::question(this, tr("Confirm options reset"),
            tr("Client restart required to activate changes.") + "<br><br>" + tr("Client will be restarted, do you want to proceed?"),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

        if (btnRetVal == QMessageBox::Cancel)
            return;

        /* reset all options and close GUI */
        model->Reset();
        model->setRestartRequired(true);
        fRestartNoMoreQuestions = true;
        accept();
    }
}

void OptionsDialog::on_okButton_clicked()
{
    bool obfs4EnabledNew = ui->enableObfs4checkBox->isChecked();
    mapper->submit();  

    if (obfs4EnabledCurrent != obfs4EnabledNew)
        UpdateConfigFileKeyBool( "obfs4", obfs4EnabledNew );

    // if the obfs4 bridges were changed, we need to update the torrc file
     if (oldObfs4Bridges != newObfs4Bridges) {
        GUIUtil::saveBridges2Torrc(torrcWithoutBridges, newObfs4Bridges);
     }
    // just close the options, the user will restart the gui later.
    accept();
}

void OptionsDialog::on_cancelButton_clicked()
{
    model->setRestartRequired(false);
    reject();
}

void OptionsDialog::on_retrieveNewBridgespushButton_clicked()
{
    if (ui->enableObfs4checkBox->isChecked()) {
        // cool we are now checked, so let's retrieve the bridges
        CaptchaDialog dlg(this);
        int code = dlg.exec();
        if (code == QDialog::Accepted) {
            const QStringList& bridges = dlg.getBridges();
            if (bridges.size() > 0) {
                // we have new bridges, so we can enable the obfs4
                ui->bridgesGroup->setEnabled(true);
                if (bridges != oldObfs4Bridges) {
                    newObfs4Bridges = bridges;
                    ui->newBridgesTextEdit->setPlainText(bridges.join("\n"));
                    this->model->setRestartRequired(true);
                    showRestartWarning(true);
                } else {
                    // torproject is still returning the same bridges
                    QMessageBox::warning(this, windowTitle(), tr("Tor has returned the same bridges yet, please wait more time, before changing the bridges."),
                        QMessageBox::Ok, QMessageBox::Ok);
                }
            }
        } else { 
            // user canceled the captcha dialog
            // need to be disabled
            ui->enableObfs4checkBox->setCheckState(Qt::Unchecked);
        }
    } else {
        // disabled the obfs4
        // it is required to restart if it was enabled and now it is not anymore.
        if (obfs4EnabledCurrent)
            this->model->setRestartRequired(true);
        ui->bridgesGroup->setEnabled(false);
    }  
}

void OptionsDialog::on_enableObfs4checkBox_clicked()
{
    on_retrieveNewBridgespushButton_clicked();
}

void OptionsDialog::showRestartWarning(bool fPersistent)
{
    ui->statusLabel->setStyleSheet("QLabel { color: red; }");

    if (fPersistent) {
        ui->statusLabel->setText(tr("Client restart required to activate changes."));
    } else {
        ui->statusLabel->setText(tr("This change would require a client restart."));
        // clear non-persistent status label after 10 seconds
        // Todo: should perhaps be a class attribute, if we extend the use of statusLabel
        QTimer::singleShot(10000, this, SLOT(clearStatusLabel()));
    }
}

void OptionsDialog::clearStatusLabel()
{
    ui->statusLabel->clear();
}

void OptionsDialog::doProxyIpChecks(QValidatedLineEdit* pUiProxyIp, QLineEdit* pUiProxyPort)
{
    const std::string strAddrProxy = pUiProxyIp->text().toStdString();
    CService addrProxy;

    // Check proxy port
    if (!pUiProxyPort->hasAcceptableInput()){
        disableOkButton();
        ui->statusLabel->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel->setText(tr("The supplied proxy port is invalid."));
        return;
    }

    proxyType checkProxy = proxyType(addrProxy);
    if (!checkProxy.IsValid()) {
        disableOkButton();
        ui->statusLabel->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel->setText(tr("The supplied proxy settings are invalid."));
        return;
    }

    enableOkButton();
    ui->statusLabel->clear();
}

bool OptionsDialog::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::FocusOut) {
        if (object == ui->proxyIp || object == ui->proxyPort) {
            emit proxyIpChecks(ui->proxyIp, ui->proxyPort);
        }
    }
    return QDialog::eventFilter(object, event);
}
