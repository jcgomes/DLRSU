#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "municipios.h"
#include "niveis_impacto.h"
#include "lixoes.h"
#include "valor_situacao.h"
#include "sobre.h"
#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>

// Variáveis Globais
int _ID = 0;
QString _DB;
QHash<QString,QString> _cbCampo;
QString _FRM; // diag=diagnóstico, cen=cenário

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setStyleSheet("QMainWindow {background: 'white';}");
    this->showMaximized();

    _DB = QDir::toNativeSeparators(qApp->applicationDirPath()+"/db.db"); // caminho do bando de dados
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_municipios_triggered()
{
    municipios mun;
    mun.setModal(true);
    mun.exec();
}


void MainWindow::on_action_niveis_impacto_triggered()
{
    niveis_impacto ni;
    ni.setModal(true);
    ni.exec();
}


void MainWindow::on_action_valores_situacoes_triggered()
{
    valor_situacao vs;
    vs.setModal(true);
    vs.exec();
}

void MainWindow::on_action_sobre_sistema_triggered()
{
    sobre s;
    s.setModal(true);
    s.exec();
}

void MainWindow::on_action_sair_triggered()
{
    close();
}

void MainWindow::on_action_formCampo_triggered()
{
    QString s = qApp->applicationDirPath() + QDir::toNativeSeparators("/doc/frmCampo.pdf");
    QDesktopServices::openUrl(QUrl::fromLocalFile(s));
}

void MainWindow::on_action_manual_sistema_triggered()
{
    QString s = qApp->applicationDirPath() + QDir::toNativeSeparators("/doc/manual.pdf");
    QDesktopServices::openUrl(QUrl::fromLocalFile(s));
}

void MainWindow::on_action_cenarios_triggered()
{
    _FRM = "cen";
    lixoes li;
    li.setModal(true);
    li.exec();
}

void MainWindow::on_action_diagnostico_triggered()
{
    _FRM = "diag";
    lixoes li;
    li.setModal(true);
    li.exec();
}
