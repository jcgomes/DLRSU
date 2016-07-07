#include "lixoes_cen.h"
#include "ui_lixoes_cen.h"
#include "globais.h"
#include <QtSql>
#include <QMessageBox>

# include <iostream>
using namespace std;

QString cenario = "";
QString acoes_propostas = "";
QString acoes_complementares = "";
QString nivel_impacto = "";
QString atividade_lixao = "";
QString presenca_catadores = "";
QString isolamento_fisico = "";
QString impactos_ambientais = "";

lixoes_cen::lixoes_cen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lixoes_cen)
{
    ui->setupUi(this);
    this->adjustSize();
    ui->pbStatus->setVisible(false);

    ui->frame_1->setEnabled(false);
    ui->frame_2->setEnabled(false);
    ui->frame_3->setEnabled(false);
    ui->frame_4->setEnabled(false);
    ui->frame_5->setEnabled(false);
    ui->frame_6->setEnabled(false);

    limpar_variaveis();
    obter_cenario();
}

lixoes_cen::~lixoes_cen()
{
    delete ui;
}

void lixoes_cen::limpar_variaveis()
{
    cenario = "";
    acoes_propostas = "";
    acoes_complementares = "";
    nivel_impacto = "";
    atividade_lixao = "";
    presenca_catadores = "";
    isolamento_fisico = "";
    impactos_ambientais = "";
}

void lixoes_cen::obter_cenario()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM lixoes WHERE id=?");
    query.bindValue(0,_ID);
    query.exec();
    query.first();

    if (query.value("nivel_impacto") == "Baixo")
    {
        nivel_impacto = "<span style='background-color:#AAD800;'>Baixo. Não há necessidade de intervenção no lixão</span>";
        cenario = "Monitoramento";
    }
    else if (query.value("nivel_impacto") == "Médio")
    {
        nivel_impacto = "<span style='background-color:#FEFB18;'>Médio. Há necessidade de intervenção no lixão</span>";
        ui->frame_1->setEnabled(true);
    }
    else if (query.value("nivel_impacto") == "Alto")
    {
        nivel_impacto = "<span style='background-color:#FF2800;'>Alto. Há necessidade de intervenção no lixão</span>";
        ui->frame_1->setEnabled(true);
    }


    if (query.value("g1_atividade") == 0)
        atividade_lixao = "Lixão fechado há mais de 20 anos";
    else if (query.value("g1_atividade") == 1)
        atividade_lixao = "Lixão fechado num tempo entre 10 e 20 anos";
    else if (query.value("g1_atividade") == 2)
        atividade_lixao = "Lixão fechado há menos de 10 anos";
    else if (query.value("g1_atividade") == 3)
    {
        atividade_lixao = "Ainda em atividade";
        acoes_complementares = "Desativação do lixão;";
    }


    if (query.value("g4_catadores") == 0)
        presenca_catadores = "Não";
    else if (query.value("g4_catadores") == 1)
    {
        presenca_catadores = "Sim";
        acoes_complementares = acoes_complementares + "Fim das atividades dos catadores;";
    }


    if (query.value("g4_isolamento_fisico_lixao") == 0)
    {
        isolamento_fisico = "Zona isolada com barreira de proteção e vigiada";
    }
    else if (query.value("g4_isolamento_fisico_lixao") == 1)
    {
        isolamento_fisico = "Zona isolada com barreira de proteção mas não vigiada";
        acoes_complementares = acoes_complementares + "Delimitação da área, controle do acesso, instalação de placas de sinalização "
                                                      "e interdição quanto à entrada;";
    }
    else if (query.value("g4_isolamento_fisico_lixao") == 2)
    {
        isolamento_fisico = "Zona não isolada e não vigiada";
        acoes_complementares = acoes_complementares + "Delimitação da área, controle do acesso, instalação de placas de sinalização "
                                                      "e interdição quanto à entrada;";
    }

    ui->lbIntervencao->setText("<html><head/><body><p align='justify'>"
                               "<b>Nível de impacto:</b> " + nivel_impacto + "<br/>"
                               "<b>Atividades lixão:</b> " + atividade_lixao + "<br/>"
                               "<b>Acesso de pessoas e animais:</b> " + isolamento_fisico + "<br/>"
                               "<b>Impactos ambientais:</b> " + impactos_ambientais + "<br/>"
                               "<b>Cenário:</b> " + cenario + "<br/>"
                               "<b>Ações complementares:</b> " + acoes_complementares +
                               "</p></body></html>");

}

void lixoes_cen::on_pbSalvar_clicked()
{

}

void lixoes_cen::on_pbCancelar_clicked()
{
    close();
}

void lixoes_cen::on_rbAreaProtegida_0_clicked()
{
    ui->frame_2->setEnabled(false);

    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM lixoes WHERE id=?");
    query.bindValue(0,_ID);
    query.exec();
    query.first();

    limpar_variaveis();

    /* Se ocorreu ao menos um impacto ambiental ou um aspecto que esteja diretamente relacionado a um impacto, exemplo:
     * No formulario de campo:
     * Questão 1.3: respostas a, b, c, d (se sim)
     * Questão 2.2: respostas b, c
     * Questão 2.10: respostas b,c
     * Questão 2.11: respostas b,c
     * Questão 3.8: resposta b
     * Questão 6.1: respostas b,c
     * Questão 6.2: respostas b,c,d
     * Questão 6.3: respostas b,c
     * Questão 6.4: respostas b,c
     */

    if ((query.value("g1_adensamento") == 1) ||
            (query.value("g1_deslizamento") == 1) ||
            (query.value("g1_erosao") == 1) ||
            (query.value("g1_outros") == 1) ||
            (query.value("g2_presenca_lixiviado") == 1) ||
            (query.value("g2_presenca_lixiviado") == 2) ||
            (query.value("g2_contaminacao_solo") == 1) ||
            (query.value("g2_contaminacao_solo") == 2) ||
            (query.value("g2_contaminacao_agua") == 1) ||
            (query.value("g2_contaminacao_agua") == 2) ||
            (query.value("g3_poluicao_agua") == 1) ||
            (query.value("g6_odores") == 1) ||
            (query.value("g6_odores") == 2) ||
            (query.value("g6_explosoes") == 1) ||
            (query.value("g6_explosoes") == 2) ||
            (query.value("g6_explosoes") == 3) ||
            (query.value("g6_queima_residuos") == 1) ||
            (query.value("g6_queima_residuos") == 2) ||
            (query.value("g6_bolsoes_migracao_biogas") == 1) ||
            (query.value("g6_bolsoes_migracao_biogas") == 2))
    {
        impactos_ambientais = "Ocorreu ao menos um impacto ambiental ou um aspecto que esteja diretamente relacionado a um impacto";
        ui->frame_3->setEnabled(true);
    }
    else
    {
        impactos_ambientais = "Não ocorreu ao menos um impacto ambiental ou um aspecto que esteja diretamente relacionado a um impacto";
        cenario = "Monitoramento";
    }
    obter_cenario();
}

void lixoes_cen::on_rbAreaProtegida_1_clicked()
{
    ui->frame_2->setEnabled(true);
    ui->frame_3->setEnabled(false);
    ui->frame_4->setEnabled(false);
    ui->frame_5->setEnabled(false);
    ui->frame_6->setEnabled(false);

    ui->rbRetiradaResiduosCFB_0->setAutoExclusive(false);
    ui->rbRetiradaResiduosCFB_1->setAutoExclusive(false);
    ui->rbDisposicaoResiduos_0->setAutoExclusive(false);
    ui->rbDisposicaoResiduos_1->setAutoExclusive(false);
    ui->rbRetiradaResiduosNBR_0->setAutoExclusive(false);
    ui->rbRetiradaResiduosNBR_1->setAutoExclusive(false);
    ui->rbHotspots_0->setAutoExclusive(false);
    ui->rbHotspots_1->setAutoExclusive(false);
    ui->rbRetiradaHotspots_0->setAutoExclusive(false);
    ui->rbRetiradaHotspots_1->setAutoExclusive(false);

    ui->rbRetiradaResiduosCFB_0->setChecked(false);
    ui->rbRetiradaResiduosCFB_1->setChecked(false);
    ui->rbDisposicaoResiduos_0->setChecked(false);
    ui->rbDisposicaoResiduos_1->setChecked(false);
    ui->rbRetiradaResiduosNBR_0->setChecked(false);
    ui->rbRetiradaResiduosNBR_1->setChecked(false);
    ui->rbHotspots_0->setChecked(false);
    ui->rbHotspots_1->setChecked(false);
    ui->rbRetiradaHotspots_0->setChecked(false);
    ui->rbRetiradaHotspots_1->setChecked(false);

    ui->rbRetiradaResiduosCFB_0->setAutoExclusive(true);
    ui->rbRetiradaResiduosCFB_1->setAutoExclusive(true);
    ui->rbDisposicaoResiduos_0->setAutoExclusive(true);
    ui->rbDisposicaoResiduos_1->setAutoExclusive(true);
    ui->rbRetiradaResiduosNBR_0->setAutoExclusive(true);
    ui->rbRetiradaResiduosNBR_1->setAutoExclusive(true);
    ui->rbHotspots_0->setAutoExclusive(true);
    ui->rbHotspots_1->setAutoExclusive(true);
    ui->rbRetiradaHotspots_0->setAutoExclusive(true);
    ui->rbRetiradaHotspots_1->setAutoExclusive(true);

    limpar_variaveis();
    obter_cenario();
}
void lixoes_cen::on_rbRetiradaResiduosCFB_1_clicked()
{
    limpar_variaveis();
    cenario = "Retirada dos resíduos";
    obter_cenario();
}

void lixoes_cen::on_rbRetiradaResiduosCFB_0_clicked()
{
    // AÇÕES DE INTERVENÇÃO – Fluxograma 2
}
