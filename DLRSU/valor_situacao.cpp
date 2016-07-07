#include "valor_situacao.h"
#include "ui_valor_situacao.h"
#include "globais.h"
#include <QtSql>
#include <QHash>
#include <QDesktopWidget>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>

valor_situacao::valor_situacao(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::valor_situacao)
{
    ui->setupUi(this);

    atualizar();
}

valor_situacao::~valor_situacao()
{
    delete ui;
}

void valor_situacao::on_pbSalvar_clicked()
{

}

void valor_situacao::on_pbCancelar_clicked()
{
    close();
}

void valor_situacao::atualizar()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlTableModel *model = new QSqlTableModel(this, con);
    model->setTable("valor_situacao");
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Grupo"));
    model->setHeaderData(2, Qt::Horizontal, tr("Situação 1"));
    model->setHeaderData(3, Qt::Horizontal, tr("Situação 2"));
    model->setHeaderData(4, Qt::Horizontal, tr("Situação 3"));
    model->setHeaderData(5, Qt::Horizontal, tr("Situação 4"));
    model->setHeaderData(6, Qt::Horizontal, tr("Situação 5"));
    model->select();

    ui->tvConsulta->setModel(model);
    ui->tvConsulta->resizeColumnsToContents();
}

void valor_situacao::on_pbAtualizar_clicked()
{
    atualizar();
}

void valor_situacao::on_pbAjuda_clicked()
{
    QString s = qApp->applicationDirPath() + QDir::toNativeSeparators("/doc/valor_situacao.pdf");
    QDesktopServices::openUrl(QUrl::fromLocalFile(s));
}
