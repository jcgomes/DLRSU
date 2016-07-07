#include "municipios.h"
#include "ui_municipios.h"
#include "municipios_cad.h"
#include "globais.h"
#include <QtSql>
#include <QHash>
#include <QDesktopWidget>
#include <QMessageBox>

municipios::municipios(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::municipios)
{
    ui->setupUi(this);

    // Combobox dinâmico: http://www.qtcentre.org/threads/63124-Qcombobox-dinamically
    _cbCampo["ID"] = "municipios.id";
    _cbCampo["Município"] = "municipios.municipio";
    _cbCampo["UF"] = "municipios.uf";

    atualizar_db();
}

municipios::~municipios()
{
    delete ui;
}

void municipios::on_tvConsulta_doubleClicked(const QModelIndex &index)
{
    //http://www.qtcentre.org/threads/38184-QAbstractItemModel-double-click-on-current-row
    QItemSelectionModel *model = ui->tvConsulta->selectionModel();
    QModelIndex current = model->currentIndex().sibling(model->currentIndex().row(),0);
    QModelIndexList selected = model->selectedIndexes();

    _ID = current.data().toInt();

    municipios_cad cad;
    cad.setModal(true);
    cad.exec();
}

void municipios::on_leParametro_returnPressed()
{
    _ID = ui->tvConsulta->model()->index(0,0).data().toInt();

    municipios_cad cad;
    Qt::WindowFlags flags = windowFlags(); // http://stackoverflow.com/questions/19097323/setwindowflagsqtwindowstaysontophint-hides-qt-window
    cad.setWindowFlags(flags | Qt::WindowStaysOnTopHint);
    cad.setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, cad.size(), qApp->desktop()->availableGeometry())); // centraliza
    cad.setModal(true);
    cad.exec();
}

void municipios::on_leParametro_textChanged(const QString &arg1)
{
    buscar();
}

void municipios::on_pbAtualizar_clicked()
{
    atualizar_db();
}

void municipios::on_pbAdicionar_clicked()
{
    _ID = 0;
    municipios_cad cad;
    cad.setModal(true);
    cad.exec();
}

void municipios::on_pbSair_clicked()
{
    close();
}

void municipios::atualizar_db()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(QDir::toNativeSeparators(qApp->applicationDirPath()+"/db.db"));
    con.open();

    ui->cbCampo->clear();

    QHashIterator<QString, QString> i(_cbCampo);
    while (i.hasNext()) {
        i.next();
        ui->cbCampo->addItem(i.key());
    }

    ui->cbCampo->setCurrentText("Município");

    QSqlTableModel *model = new QSqlTableModel(this, con);
    model->setTable("municipios");
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Município"));
    model->setHeaderData(2, Qt::Horizontal, tr("UF"));
    model->select();

    ui->tvConsulta->setModel(model);
    ui->tvConsulta->resizeColumnsToContents();
    ui->leParametro->clear();
    ui->leParametro->setFocus();
}

void municipios::buscar()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QString filtro;
    filtro.append(""+_cbCampo.value(ui->cbCampo->currentText())+" like '"+ui->leParametro->text()+"%'");

    QSqlTableModel *model = new QSqlTableModel(this, con);
    model->setTable("municipios");
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Município"));
    model->setHeaderData(2, Qt::Horizontal, tr("UF"));
    model->setFilter(filtro);
    model->setSort(2, Qt::DescendingOrder);
    model->select();

    ui->tvConsulta->setModel(model);
}
