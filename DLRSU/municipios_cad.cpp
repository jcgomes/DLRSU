#include "municipios_cad.h"
#include "ui_municipios_cad.h"
#include "globais.h"
#include <QHash>
#include <QMessageBox>
#include <QtSql>

# include <iostream>
using namespace std;

QMap<QString,int> _cbUF; // QMap ordena a lista por key. http://doc.qt.io/qt-5/qmap.html

municipios_cad::municipios_cad(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::municipios_cad)
{
    ui->setupUi(this);

    _cbUF.insert("AC", 0);
    _cbUF.insert("AL", 1);
    _cbUF.insert("AM", 2);
    _cbUF.insert("AP", 3);
    _cbUF.insert("BA", 4);
    _cbUF.insert("CE", 5);
    _cbUF.insert("DF", 6);
    _cbUF.insert("ES", 7);
    _cbUF.insert("GO", 8);
    _cbUF.insert("MA", 9);
    _cbUF.insert("MG", 10);
    _cbUF.insert("MS", 11);
    _cbUF.insert("MT", 12);
    _cbUF.insert("PA", 13);
    _cbUF.insert("PB", 14);
    _cbUF.insert("PE", 15);
    _cbUF.insert("PI", 16);
    _cbUF.insert("PR", 17);
    _cbUF.insert("RJ", 18);
    _cbUF.insert("RN", 19);
    _cbUF.insert("RO", 20);
    _cbUF.insert("RR", 21);
    _cbUF.insert("RS", 22);
    _cbUF.insert("SC", 23);
    _cbUF.insert("SE", 24);
    _cbUF.insert("SP", 25);
    _cbUF.insert("TO", 26);

    QMapIterator<QString, int> i(_cbUF);
    while (i.hasNext()) {
        i.next();
        ui->cbUF->addItem(i.key());
    }

    if (_ID == 0)
        ui->leMunicipio->setFocus();
    else
        recuperar_registro();
}

municipios_cad::~municipios_cad()
{
    delete ui;
}

void municipios_cad::on_pbSalvar_clicked()
{
    if (_ID == 0)
        inserir();
    else
        editar();
}

void municipios_cad::on_pbCancelar_clicked()
{
    close();
}

void municipios_cad::recuperar_registro()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM municipios where id = ? ");
    query.bindValue(0,_ID);
    query.exec();

    while (query.next())
    {
        ui->leMunicipio->setText(query.value(1).toString());

        QMapIterator<QString, int> i(_cbUF);
        while (i.hasNext()) {
            i.next();
            if (query.value(2).toString() == i.key())
                ui->cbUF->setCurrentIndex(i.value());
        }
    }

    con.close();
}

void municipios_cad::inserir()
{

    // insere
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlTableModel *model = new QSqlTableModel(this, con);
    model->setTable("municipios");
    model->select();

    QSqlRecord rec = model->record();
    rec.setValue("municipio", ui->leMunicipio->text());
    rec.setValue("uf", ui->cbUF->currentText());
    model->insertRecord(-1,rec);

    con.close();

    QMessageBox msgBox;
    msgBox.setText("Arquivo inserido com sucesso");
    msgBox.exec();

    close();

}

void municipios_cad::editar()
{
    // atualiza
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("UPDATE municipios set municipio=?, uf=? WHERE id=?");
    query.bindValue(0,ui->leMunicipio->text());
    query.bindValue(1,ui->cbUF->currentText());
    query.bindValue(2,_ID);
    query.exec();

    con.close();

    QMessageBox msgBox;
    msgBox.setText("Arquivo atualizado com sucesso");
    msgBox.exec();

    close();
}
