#include "lixoes.h"
#include "ui_lixoes.h"
#include "lixoes_cad.h"
#include "lixoes_cen.h"
#include "globais.h"
#include <QtSql>
#include <QDesktopWidget>
#include <QPainter>
#include <QMessageBox>

# include <iostream>
using namespace std;

lixoes::lixoes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lixoes)
{
    ui->setupUi(this);

    // Combobox dinâmico: http://www.qtcentre.org/threads/63124-Qcombobox-dinamically
    _cbCampo["ID"] = "lixoes.id";
    _cbCampo["Nome"] = "lixoes.nome";
    _cbCampo["Município"] = "municipio";
    _cbCampo["Pontuação"] = "lixoes.pontuacao";
    _cbCampo["Nível de impacto"] = "lixoes.nivel_impacto";

    ui->tvConsulta->setItemDelegate(new Delegate);

    atualizar_db();

    if (_FRM=="diag")
    {
        ui->pbAdicionar->setEnabled(true);
    }
    else if (_FRM=="cen")
    {
        ui->pbAdicionar->setEnabled(false); // Não é possível adicionar, pois o lixão deve estar diagnosticado.
    }
}

lixoes::~lixoes()
{
    delete ui;
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    if (index.column() == 97) // coluna que será pintada
    {
        //QVariant GetValueOfColumn =  index.model()->data(index.model()->index(index.row(),index.column()),Qt::DisplayRole);
        //http://www.qtcentre.org/threads/64391-SOLVED-Get-QModelIndex-data?p=284569#post284569
        QVariant GetValueOfColumn = index.data();

        if (GetValueOfColumn == "Baixo")
            painter->fillRect(option.rect, QColor(170, 216, 0));
        else if (GetValueOfColumn == "Médio")
            painter->fillRect(option.rect, QColor(254, 251, 24));
        else if (GetValueOfColumn == "Alto")
            painter->fillRect(option.rect, QColor(255, 40, 0));
    }

    return QItemDelegate::paint(painter, option, index);
}

void lixoes::on_tvConsulta_doubleClicked(const QModelIndex &index)
{
    //http://www.qtcentre.org/threads/38184-QAbstractItemModel-double-click-on-current-row
    QItemSelectionModel *model = ui->tvConsulta->selectionModel();
    QModelIndex current = model->currentIndex().sibling(model->currentIndex().row(),0);
    QModelIndexList selected = model->selectedIndexes();

    _ID = current.data().toInt();

    if (_FRM=="diag")
    {
        lixoes_cad cad;
        cad.setModal(true);
        cad.exec();
    }
    else if (_FRM=="cen")
    {
        QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
        con.setDatabaseName(_DB);
        con.open();

        QSqlQuery query;
        query.prepare("SELECT nivel_impacto FROM lixoes WHERE id=?");
        query.bindValue(0,_ID);
        query.exec();

        query.first();
        if ((query.value(0).toString() == NULL) || (query.value(0).toString() == ""))
        {
            QMessageBox msg;
            msg.setIcon(QMessageBox::Warning);
            msg.setWindowTitle("Atenção");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setText("O lixão precisa estar diagnosticado antes de usar a ferramenta cenários.");
            msg.exec();
        }
        else
        {
            lixoes_cen cen;
            //Qt::WindowFlags flags = windowFlags(); // http://stackoverflow.com/questions/19097323/setwindowflagsqtwindowstaysontophint-hides-qt-window
            //cen.setWindowFlags(flags | Qt::WindowStaysOnTopHint);
            //cen.setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, cen.size(), qApp->desktop()->availableGeometry())); // centraliza
            cen.setModal(true);
            cen.exec();
        }
    }


}

void lixoes::on_leParametro_returnPressed()
{
    _ID = ui->tvConsulta->model()->index(0,0).data().toInt();

    if (_FRM=="diag")
    {
        lixoes_cad cad;
        //Qt::WindowFlags flags = windowFlags(); // http://stackoverflow.com/questions/19097323/setwindowflagsqtwindowstaysontophint-hides-qt-window
        //cad.setWindowFlags(flags | Qt::WindowStaysOnTopHint);
        //cad.setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, cad.size(), qApp->desktop()->availableGeometry())); // centraliza
        cad.setModal(true);
        cad.exec();
    }
    else if (_FRM=="cen")
    {
        QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
        con.setDatabaseName(_DB);
        con.open();

        QSqlQuery query;
        query.prepare("SELECT nivel_impacto FROM lixoes WHERE id=?");
        query.bindValue(0,_ID);
        query.exec();

        query.first();
        if ((query.value(0).toString() == NULL) || (query.value(0).toString() == ""))
        {
            QMessageBox msg;
            msg.setIcon(QMessageBox::Warning);
            msg.setWindowTitle("Atenção");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setText("O lixão precisa estar diagnosticado antes de usar a ferramenta cenários.");
            msg.exec();
        }
        else
        {
            lixoes_cen cen;
            //Qt::WindowFlags flags = windowFlags(); // http://stackoverflow.com/questions/19097323/setwindowflagsqtwindowstaysontophint-hides-qt-window
            //cen.setWindowFlags(flags | Qt::WindowStaysOnTopHint);
            //cen.setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, cen.size(), qApp->desktop()->availableGeometry())); // centraliza
            cen.setModal(true);
            cen.exec();
        }
    }


}

void lixoes::on_leParametro_textChanged(const QString &arg1)
{
    buscar();
}

void lixoes::on_pbAtualizar_clicked()
{
    atualizar_db();
}

void lixoes::on_pbAdicionar_clicked()
{
    _ID = 0;
    lixoes_cad cad;
    cad.setModal(true);
    cad.exec();
}

void lixoes::on_pbSair_clicked()
{
    close();
}

void lixoes::atualizar_db()
{
    ui->cbCampo->clear();

    QHashIterator<QString, QString> i(_cbCampo);
    while (i.hasNext()) {
        i.next();
        ui->cbCampo->addItem(i.key());
    }

    ui->cbCampo->setCurrentText("Município");

    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    // http://doc.qt.io/qt-5/qsqlrelationaltablemodel.html
    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this, con);
    model->setTable("lixoes");
    model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    model->setRelation(5, QSqlRelation("municipios", "id", "municipio"));
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Nome"));
    model->setHeaderData(5, Qt::Horizontal, tr("Município"));
    model->setHeaderData(96, Qt::Horizontal, tr("Pontuação"));
    model->setHeaderData(97, Qt::Horizontal, tr("Nível de impacto"));
    model->select();

    ui->tvConsulta->setModel(model);
    ui->tvConsulta->resizeColumnsToContents();

    ui->tvConsulta->setColumnHidden(2,true);
    ui->tvConsulta->setColumnHidden(3,true);
    ui->tvConsulta->setColumnHidden(4,true);
    ui->tvConsulta->setColumnHidden(6,true);
    for (int j=6;j<=95;j++)
        ui->tvConsulta->setColumnHidden(j,true);
    ui->tvConsulta->setColumnHidden(98,true);

    ui->leParametro->clear();
    ui->leParametro->setFocus();
}

void lixoes::buscar()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QString filtro;
    filtro.append(""+_cbCampo.value(ui->cbCampo->currentText())+" like '"+ui->leParametro->text()+"%'");

    // http://doc.qt.io/qt-5/qsqlrelationaltablemodel.html
    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this, con);
    model->setTable("lixoes");
    model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    model->setRelation(5, QSqlRelation("municipios", "id", "municipio"));
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Nome"));
    model->setHeaderData(5, Qt::Horizontal, tr("Município"));
    model->setHeaderData(96, Qt::Horizontal, tr("Pontuação"));
    model->setHeaderData(97, Qt::Horizontal, tr("Nível de impacto"));
    model->setFilter(filtro);
    model->select();

    ui->tvConsulta->setModel(model);
    ui->tvConsulta->sortByColumn(97, Qt::DescendingOrder); // order by pontuacao desc
    ui->tvConsulta->resizeColumnsToContents();
    ui->tvConsulta->setColumnHidden(2,true);
    ui->tvConsulta->setColumnHidden(3,true);
    ui->tvConsulta->setColumnHidden(4,true);
    ui->tvConsulta->setColumnHidden(6,true);
    for (int j=6;j<=95;j++)
        ui->tvConsulta->setColumnHidden(j,true);
    ui->tvConsulta->setColumnHidden(98,true);
}
