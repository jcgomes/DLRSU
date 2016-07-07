#include "niveis_impacto.h"
#include "ui_niveis_impacto.h"
#include "globais.h"
#include <QMessageBox>
#include <QtSql>

niveis_impacto::niveis_impacto(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::niveis_impacto)
{
    ui->setupUi(this);

    formatar_niveis();

    recuperar_registro();
}

niveis_impacto::~niveis_impacto()
{
    delete ui;
}

void niveis_impacto::on_pbSalvar_clicked()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("UPDATE niveis_impacto set baixo=?, medio=?, alto=? WHERE id=1");
    query.bindValue(0,ui->leBaixo->text());
    query.bindValue(1,ui->leMedio->text());
    query.bindValue(2,ui->leAlto->text());
    query.exec();

    con.close();

    QMessageBox msgBox;
    msgBox.setText("Arquivo atualizado com sucesso");
    msgBox.exec();

    close();
}

void niveis_impacto::on_pbCancelar_clicked()
{
    close();
}

void niveis_impacto::formatar_niveis()
{
    /*
     * http://doc.qt.io/qt-5/qregexp.html
     * As expressões regulares abaixo assumem que os niveis de impactos serão numeros inteiros entre 1 e 100
     * O formato dos níveis de impactos são:
     * baixo: <=00
     * médio: >=00<=00
     * alto:  >=00
     */

    QRegExp rxBaixo("(<=)(\\d+)(?:\\s*)");
    int posB = rxBaixo.indexIn(ui->leBaixo->text());
    if (posB > -1) {
        QString unit = rxBaixo.cap(1); // ex:"<="
        QString value = rxBaixo.cap(2);  // ex:"20"
    }

    QRegExp rxMedio("(>=)(\\d+)(?:\\s*)(<=)(\\d+)(?:\\s*)");
    int posM = rxMedio.indexIn(ui->leMedio->text());
    if (posM > -1) {
        QString unit1 = rxMedio.cap(1); // ex:">="
        QString value1 = rxMedio.cap(2);  // ex:"21"
        QString unit2 = rxMedio.cap(3); // ex:"<="
        QString value2 = rxMedio.cap(4);  // ex:"70"
    }

    QRegExp rxAlto("(>=)(\\d+)(?:\\s*)");
    int posA = rxAlto.indexIn(ui->leAlto->text());
    if (posA > -1) {
        QString unit1 = rxAlto.cap(1); // ex:">="
        QString value1 = rxAlto.cap(2);  // ex:"71"
    }

    QValidator *vBaixo = new QRegExpValidator(rxBaixo, this);
    QValidator *vMedio = new QRegExpValidator(rxMedio, this);
    QValidator *vAlto = new QRegExpValidator(rxAlto, this);

    ui->leBaixo->setValidator(vBaixo);
    ui->leMedio->setValidator(vMedio);
    ui->leAlto->setValidator(vAlto);
}

void niveis_impacto::recuperar_registro()
{

    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM niveis_impacto where id = 1");
    query.exec();

    while (query.next())
    {
        ui->leBaixo->setText(query.value(1).toString());
        ui->leMedio->setText(query.value(2).toString());
        ui->leAlto->setText(query.value(3).toString());
    }

    con.close();
}
