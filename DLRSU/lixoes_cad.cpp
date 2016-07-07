#include "lixoes_cad.h"
#include "ui_lixoes_cad.h"
#include "globais.h"
#include <QMessageBox>
#include <QtSql>
#include <QProgressDialog>

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QTextCursor>
#include <QTextImageFormat>

#include <typeinfo>
# include <iostream>
using namespace std;

int _Diagnostico = 0; //0=Não, 1=Sim
int _Preenchimento = 0; //0=Não, 1=Sim

lixoes_cad::lixoes_cad(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lixoes_cad)
{
    ui->setupUi(this);
    ui->pbStatus->setVisible(false);
    ui->lbStatus->setText("<html><head/><body><p><span style=' color:#ff0000;'>* </span><span style=' color:#000000;'>Preenchimento obrigatório para fins de cadastro. Para diagnosticar o lixão, </span><span style=' font-weight:600; color:#000000;'>todos</span><span style=' color:#000000;'> os campos devem ser preenchidos</span></p></body></html>");
    this->adjustSize(); // Qdialog com domensões mínimas.

    ui->leCEP->setInputMask("99999-999;_");
    ui->leDataVisita->setInputMask("99/99/9999;_");
    ui->leDataInicioAtividades->setInputMask("99/99/9999;_");
    ui->leDataFimAtividades->setInputMask("99/99/9999;_");

    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM municipios");
    query.exec();
    while (query.next())
    {
        QString s = query.value(1).toString();
        ui->cbMunicipios->addItem(s);
    }
    con.close();

    if (_ID == 0)
    {
        ui->leNome->setFocus();
        ui->pbDiagnosticar->setEnabled(false);
    }
    else
    {
        recuperar_registro();
        selecionarFigura();
        verificar_diagnostico(); // para fins de habilitar ou não o botão diagnóstico
    }
}

lixoes_cad::~lixoes_cad()
{
    delete ui;
}

void lixoes_cad::on_pbSalvar_clicked()
{
    if (_ID == 0)
        inserir();
    else
        editar();


    verificar_diagnostico();

    if (_Preenchimento==0 && _Diagnostico==0) // se o preenchimento da tabela não foi feito corretamente, e o lixão não foi diagnosticado
    {
        QMessageBox msg1;
        msg1.setIcon(QMessageBox::Information);
        msg1.setWindowTitle("Atenção");
        msg1.setTextFormat(Qt::RichText);
        msg1.setText("<b>O lixão ainda não foi diagnosticado e existem campos à preencher.<\b>");
        msg1.setInformativeText("Para fazer o diagnóstico será necessário antes preencher \n"
                                "todos os campos do formulário e salvar o registro novamente. \n"
                                "Então o diagnóstico poderá ser feito na aba 'Diagnóstico', \n"
                                "clicando no botão 'Diagnosticar Lixão'.");
        msg1.setStandardButtons(QMessageBox::Ok);
        msg1.exec();
    }
    else if (_Preenchimento==0 && _Diagnostico==1) // se o preenchimento da tabela não foi feito corretamente, mas o lixão foi diagnosticado
    {
        QMessageBox msg2;
        msg2.setIcon(QMessageBox::Information);
        msg2.setWindowTitle("Atenção");
        msg2.setTextFormat(Qt::RichText);
        msg2.setText("<b>O lixão já foi diagnosticado, porém existem campos à preencher.<\b>");
        msg2.setInformativeText("Sugere-se que seja revisto o preenchimento dos campos, e\n"
                                "e então refeito o diagnóstico do lixão.");
        msg2.setStandardButtons(QMessageBox::Ok);
        msg2.exec();
    }
    else if (_Preenchimento==1 && _Diagnostico==0) // se o preenchimento da tabela foi feito corretamente, mas o lixão não foi diagnosticado
    {
        int msg3 = QMessageBox::question(this, "Atenção!", "<b>O lixão ainda não foi diagnosticado.</b> Gostaria de faze-lo agora?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (msg3 == QMessageBox::Yes)
            diagnosticar();

    }
    else if (_Preenchimento==1 && _Diagnostico==1) // se o preenchimento da tabela foi feito corretamente e o lixão foi diagnosticado
    {
        QMessageBox msg4;
        msg4.setIcon(QMessageBox::Warning);
        msg4.setWindowTitle("Atenção");
        msg4.setTextFormat(Qt::RichText);
        msg4.setText("<b>O lixão já está diagnosticado, mas o botão salvar foi pressionado.</b>");
        msg4.setInformativeText("Caso houve alteração em algum campo que influencia no cálculo da \n"
                                  "pontuação, gostaria de refazer o diagnóstico?");
        msg4.setDetailedText("Questões que influenciam no cálculo da pontuação: \n"

                               "\nDa aba 'Caracterização da área', as seguintes questões:\n"
                               "- Ocorre em algum tipo de área conforme Lei 9985/2000? \n"
                               "- Área urbana ou rural? \n"
                               "- Ocorre em talvegue com curso d’água intermitente? \n"
                               "- Ocorre em região de várzea? \n"
                               "- Quantidade de corpos hídricos num raio de até 200 m? \n"
                               "- Área industrial? \n"

                               "\nTodos os campos da aba 'Caracterização do lixão', exceto: \n"
                               "- Campo 'Quais' da questão 'Acidentes e eventos importantes...' \n"
                               "- Campo 'Quais' da questão 'Natureza dos resíduos' \n"

                               "\nTodos os campos da aba 'Solo e águas subterrâneas', exceto: \n"
                               "- Questão 'Se nível piezométrico inferior a 1m' \n"


                               "\nTodos os campos da aba 'Águas superficiais' \n"

                               "\nTodos os campos da aba 'Social' com exceção dos campos \n"
                               "de texto para detalhar 'Outras aves' e 'Outros animais' \n"

                               "\nTodos os campos da aba 'Meio natural e paisagens', \n"
                               "com exceção do campo 'Desconhecida', da questão  \n"
                               "'Distância de elemento cultural...' \n"

                              "\nTodos os campos da aba 'Ar'");
        msg4.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msg4.setDefaultButton(QMessageBox::No);
        int ret = msg4.exec();

        switch (ret)
        {
        case QMessageBox::Yes:
            diagnosticar();
            break;
        case QMessageBox::No:
            break;
        }
    }
}

void lixoes_cad::on_pbCancelar_clicked()
{
    close();
}

void lixoes_cad::on_rbOutrosAcidentes_0_clicked()
{
    ui->lbOutrosAcidentesDetalhes->setEnabled(false);
    ui->leOutrosAcidentesDetalhes->setEnabled(false);
    ui->leOutrosAcidentesDetalhes->clear();
}

void lixoes_cad::on_rbOutrosAcidentes_1_clicked()
{
    ui->lbOutrosAcidentesDetalhes->setEnabled(true);
    ui->leOutrosAcidentesDetalhes->setEnabled(true);
    ui->leOutrosAcidentesDetalhes->setFocus();
}

void lixoes_cad::on_cbNaturezaResiduos_2_clicked()
{
    if (ui->cbNaturezaResiduos_2->checkState())
    {
        ui->lbNaturezaResiduosDetalhes->setEnabled(true);
        ui->leNaturezaResiduosDetalhes->setEnabled(true);
        ui->leNaturezaResiduosDetalhes->setFocus();
    }
    else
    {
        ui->lbNaturezaResiduosDetalhes->setEnabled(false);
        ui->leNaturezaResiduosDetalhes->setEnabled(false);
        ui->leNaturezaResiduosDetalhes->clear();
    }
}

void lixoes_cad::on_rbNivelPiezometrico_0_clicked()
{
    ui->lbDistanciaResiduosAgua->setEnabled(false);
    ui->rbDistanciaResiduosAgua_0->setEnabled(false);
    ui->rbDistanciaResiduosAgua_1->setEnabled(false);
    ui->rbDistanciaResiduosAgua_2->setEnabled(false);
}

void lixoes_cad::on_rbNivelPiezometrico_1_clicked()
{
    ui->lbDistanciaResiduosAgua->setEnabled(false);
    ui->rbDistanciaResiduosAgua_0->setEnabled(false);
    ui->rbDistanciaResiduosAgua_1->setEnabled(false);
    ui->rbDistanciaResiduosAgua_2->setEnabled(false);
}

void lixoes_cad::on_rbNivelPiezometrico_2_clicked()
{
    ui->lbDistanciaResiduosAgua->setEnabled(true);
    ui->rbDistanciaResiduosAgua_0->setEnabled(true);
    ui->rbDistanciaResiduosAgua_1->setEnabled(true);
    ui->rbDistanciaResiduosAgua_2->setEnabled(true);
}

void lixoes_cad::on_cbOutrasAves_clicked()
{
    if (ui->cbOutrasAves->checkState())
    {
        ui->leOutrasAves->setEnabled(true);
        ui->leOutrasAves->setEnabled(true);
        ui->leOutrasAves->setFocus();
    }
    else
    {
        ui->leOutrasAves->setEnabled(false);
        ui->leOutrasAves->setEnabled(false);
        ui->leOutrasAves->clear();
    }
}

void lixoes_cad::on_cbOutrosAnimais_clicked()
{
    if (ui->cbOutrosAnimais->checkState())
    {
        ui->leOutrosAnimais->setEnabled(true);
        ui->leOutrosAnimais->setEnabled(true);
        ui->leOutrosAnimais->setFocus();
    }
    else
    {
        ui->leOutrosAnimais->setEnabled(false);
        ui->leOutrosAnimais->setEnabled(false);
        ui->leOutrosAnimais->clear();
    }
}

void lixoes_cad::on_rbDistanciaElemento_3_clicked()
{
    ui->leDistanciaElementoDetalhes->setEnabled(true);
    ui->leDistanciaElementoDetalhes->setEnabled(true);
    ui->leDistanciaElementoDetalhes->setFocus();
}

void lixoes_cad::on_rbDistanciaElemento_0_clicked()
{
    ui->leDistanciaElementoDetalhes->setEnabled(false);
    ui->leDistanciaElementoDetalhes->setEnabled(false);
    ui->leDistanciaElementoDetalhes->clear();
}

void lixoes_cad::on_rbDistanciaElemento_1_clicked()
{
    ui->leDistanciaElementoDetalhes->setEnabled(false);
    ui->leDistanciaElementoDetalhes->setEnabled(false);
    ui->leDistanciaElementoDetalhes->clear();
}

void lixoes_cad::on_rbDistanciaElemento_2_clicked()
{
    ui->leDistanciaElementoDetalhes->setEnabled(false);
    ui->leDistanciaElementoDetalhes->setEnabled(false);
    ui->leDistanciaElementoDetalhes->clear();
}

void lixoes_cad::on_pbDiagnosticar_clicked()
{
    if (_Preenchimento==0 && _Diagnostico==0) // se o preenchimento da tabela não foi feito corretamente, e o lixão não foi diagnosticado
    {
        QMessageBox msg1;
        msg1.setIcon(QMessageBox::Information);
        msg1.setWindowTitle("Atenção");
        msg1.setTextFormat(Qt::RichText);
        msg1.setText("<b>O lixão ainda não foi diagnosticado e existem campos à preencher.<\b>");
        msg1.setInformativeText("Para fazer o diagnóstico será necessário antes preencher \n"
                                "todos os campos do formulário e salvar o registro novamente. \n"
                                "Então o diagnóstico poderá ser feito na aba 'Diagnóstico', \n"
                                "clicando no botão 'Diagnosticar Lixão'.");
        msg1.setStandardButtons(QMessageBox::Ok);
        msg1.exec();
    }
    else if (_Preenchimento==0 && _Diagnostico==1) // se o preenchimento da tabela não foi feito corretamente, mas o lixão foi diagnosticado
    {
        QMessageBox msg2;
        msg2.setIcon(QMessageBox::Information);
        msg2.setWindowTitle("Atenção");
        msg2.setTextFormat(Qt::RichText);
        msg2.setText("<b>O lixão já foi diagnosticado, porém existem campos à preencher.<\b>");
        msg2.setInformativeText("Sugere-se que seja revisto o preenchimento dos campos, e\n"
                                "e então refeito o diagnóstico do lixão.");
        msg2.setStandardButtons(QMessageBox::Ok);
        msg2.exec();
    }
    else if (_Preenchimento==1 && _Diagnostico==0) // se o preenchimento da tabela foi feito corretamente, mas o lixão não foi diagnosticado
    {
        int msg3 = QMessageBox::question(this, "Atenção!", "<b>O lixão ainda não foi diagnosticado.</b> Gostaria de faze-lo agora?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (msg3 == QMessageBox::Yes)
            diagnosticar();

    }
    else if (_Preenchimento==1 && _Diagnostico==1) // se o preenchimento da tabela foi feito corretamente e o lixão foi diagnosticado
    {
        int msg4 = QMessageBox::warning(this, "Atenção!", "<b>O lixão já está diagnosticado.</b> Gostaria de refazer o diagnóstico?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (msg4 == QMessageBox::Yes)
            diagnosticar();
    }
}

void lixoes_cad::recuperar_registro()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM lixoes where id = ? ");
    query.bindValue(0,_ID);
    query.exec();

    while (query.next())
    {
        ui->leNome->setText(query.value("nome").toString());
        ui->leEndereco->setText(query.value("endereco").toString());
        ui->leBairro->setText(query.value("bairro").toString());
        ui->leCEP->setText(query.value("cep").toString());

        // http://www.qtcentre.org/threads/64393-better-way-to-set-data-in-a-QCombobox?p=284574#post284574
        QSqlQuery query2;
        query2.prepare("SELECT * FROM municipios where id = ? ");
        query2.bindValue(0,query.value("id_municipio").toInt());
        query2.exec();
        while (query2.next())
        {
            int index = ui->cbMunicipios->findText(query2.value(1).toString());
            if( index >= 0 )
                ui->cbMunicipios->setCurrentIndex(index);
        }
        // Caracterização da área [início]
        ui->leLatitude->setText(query.value("latitude").toString());
        ui->leLongitude->setText(query.value("longitude").toString());
        ui->leFolhaTopografica->setText(query.value("folha_topografica").toString());
        ui->leUTM->setText(query.value("coordenadas_utm").toString());
        ui->leDatum->setText(query.value("datum_mc").toString());
        ui->leBaciaHidrografica->setText(query.value("bacia_hidrografica").toString());
        ui->leDataVisita->setText(query.value("data_visita").toString());
        ui->leDataInicioAtividades->setText(query.value("inicio_atividades").toString());
        ui->leDataFimAtividades->setText(query.value("fim_atividades").toString());
        ui->leEnquadramentoPlanoDiretor->setText(query.value("enquadramento_plano_diretor").toString());
        ui->leUsoFuturo->setText(query.value("uso_futuro").toString());
        ui->leProprietario->setText(query.value("proprietario_terreno").toString());
        ui->leInformantes->setText(query.value("informantes").toString());
        ui->leAcompanhantes->setText(query.value("acompanhantes").toString());

        if (query.value("condicao_tempo") == 0)
            ui->rbCondicaoTempo_0->setChecked(true);
        else if (query.value("condicao_tempo") == 1)
            ui->rbCondicaoTempo_1->setChecked(true);
        else if (query.value("condicao_tempo") == 2)
            ui->rbCondicaoTempo_2->setChecked(true);
        else if (query.value("condicao_tempo") == 3)
            ui->rbCondicaoTempo_3->setChecked(true);
        else if (query.value("condicao_tempo") == 4)
            ui->rbCondicaoTempo_4->setChecked(true);ui->lbDistanciaResiduosAgua->setEnabled(false);
        ui->rbDistanciaResiduosAgua_0->setChecked(false);
        ui->rbDistanciaResiduosAgua_1->setChecked(false);
        ui->rbDistanciaResiduosAgua_2->setChecked(false);

        ui->leCondicaoVento->setText(query.value("condicao_vento").toString());
        ui->leAcumuladoChuva->setText(query.value("acumulado_chuva").toString());

        if (query.value("lei_9985") == 0)
            ui->rbLei9985_0->setChecked(true);
        else if (query.value("lei_9985") == 1)
            ui->rbLei9985_1->setChecked(true);

        if (query.value("area_urbana_rural") == 0)
            ui->rbAreaUrbanaRural_0->setChecked(true);
        else if (query.value("area_urbana_rural") == 1)
            ui->rbAreaUrbanaRural_1->setChecked(true);

        if (query.value("ocorrencia_talvegue") == 0)
            ui->rbOcorrenciaTalvegue_0->setChecked(true);
        else if (query.value("ocorrencia_talvegue") == 1)
            ui->rbOcorrenciaTalvegue_1->setChecked(true);

        if (query.value("ocorrencia_varzea") == 0)
            ui->rbOcorrenciaVarzea_0->setChecked(true);
        else if (query.value("ocorrencia_varzea") == 1)
            ui->rbOcorrenciaVarzea_1->setChecked(true);

        ui->sbQuantidadeCorposHidricos->setValue(query.value("ocorrencia_corpos_hidricos").toInt());

        if (query.value("area_industrial") == 0)
            ui->rbAreaIndustrial_0->setChecked(true);
        else if (query.value("area_industrial") == 1)
            ui->rbAreaIndustrial_1->setChecked(true);
        // Caracterização da área [fim]

        // 1. Caracterização do lixão [início]
        if (query.value("g1_area") == 0)
            ui->rbAreaLixao_0->setChecked(true);
        else if (query.value("g1_area") == 1)
            ui->rbAreaLixao_1->setChecked(true);
        else if (query.value("g1_area") == 2)
            ui->rbAreaLixao_2->setChecked(true);
        else if (query.value("g1_area") == 3)
            ui->rbAreaLixao_3->setChecked(true);

        if (query.value("g1_atividade") == 0)
            ui->rbAtividadeLixao_0->setChecked(true);
        else if (query.value("g1_atividade") == 1)
            ui->rbAtividadeLixao_1->setChecked(true);
        else if (query.value("g1_atividade") == 2)
            ui->rbAtividadeLixao_3->setChecked(true);
        else if (query.value("g1_atividade") == 3)
            ui->rbAtividadeLixao_3->setChecked(true);

        if (query.value("g1_adensamento") == 0)
            ui->rbAdensamento_0->setChecked(true);
        else if (query.value("g1_adensamento") == 1)
            ui->rbAdensamento_1->setChecked(true);

        if (query.value("g1_deslizamento") == 0)
            ui->rbDeslizamento_0->setChecked(true);
        else if (query.value("g1_deslizamento") == 1)
            ui->rbDeslizamento_1->setChecked(true);

        if (query.value("g1_erosao") == 0)
            ui->rbErosao_0->setChecked(true);
        else if (query.value("g1_erosao") == 1)
            ui->rbErosao_1->setChecked(true);

        if (query.value("g1_outros") == 0)
        {
            ui->rbOutrosAcidentes_0->setChecked(true);
        }
        else if (query.value("g1_outros") == 1)
        {
            ui->rbOutrosAcidentes_1->setChecked(true);
            ui->lbOutrosAcidentesDetalhes->setEnabled(true);
            ui->leOutrosAcidentesDetalhes->setEnabled(true);
            ui->leOutrosAcidentesDetalhes->setText(query.value("g1_outros_detalhes").toString());
        }

        if (query.value("g1_espessura") == 0)
            ui->rbEspessuraCamada_0->setChecked(true);
        else if (query.value("g1_espessura") == 1)
            ui->rbEspessuraCamada_1->setChecked(true);
        else if (query.value("g1_espessura") == 2)
            ui->rbEspessuraCamada_2->setChecked(true);
        else if (query.value("g1_espessura") == 3)
            ui->rbEspessuraCamada_3->setChecked(true);

        if (query.value("g1_residuos_classe_IIB") == 0)
            ui->cbNaturezaResiduos_0->setChecked(false);
        else if (query.value("g1_residuos_classe_IIB") == 1)
            ui->cbNaturezaResiduos_0->setChecked(true);

        if (query.value("g1_residuos_classe_IIA") == 0)
            ui->cbNaturezaResiduos_1->setChecked(false);
        else if (query.value("g1_residuos_classe_IIA") == 1)
            ui->cbNaturezaResiduos_1->setChecked(true);

        if (query.value("g1_residuos_classe_I") == 0)
        {
            ui->cbNaturezaResiduos_2->setChecked(false);
        }
        else if (query.value("g1_residuos_classe_I") == 1)
        {
            ui->cbNaturezaResiduos_2->setChecked(true);
            ui->lbNaturezaResiduosDetalhes->setEnabled(true);
            ui->leNaturezaResiduosDetalhes->setEnabled(true);
            ui->leNaturezaResiduosDetalhes->setText(query.value("g1_residuos_classe_I_detalhes").toString());
        }

        if (query.value("g1_impermeabilizacao_superior") == 0)
            ui->rbImpermeabilizacaoSuperior_0->setChecked(true);
        else if (query.value("g1_impermeabilizacao_superior") == 1)
            ui->rbImpermeabilizacaoSuperior_1->setChecked(true);
        else if (query.value("g1_impermeabilizacao_superior") == 2)
            ui->rbImpermeabilizacaoSuperior_2->setChecked(true);

        if (query.value("g1_pluviometria") == 0)
            ui->rbPluviometria_0->setChecked(true);
        else if (query.value("g1_pluviometria") == 1)
            ui->rbPluviometria_1->setChecked(true);
        else if (query.value("g1_pluviometria") == 2)
            ui->rbPluviometria_2->setChecked(true);
        else if (query.value("g1_pluviometria") == 3)
            ui->rbPluviometria_3->setChecked(true);

        if (query.value("g1_declividade") == 0)
            ui->rbDeclividadeTerreno_0->setChecked(true);
        else if (query.value("g1_declividade") == 1)
            ui->rbDeclividadeTerreno_1->setChecked(true);
        else if (query.value("g1_declividade") == 2)
            ui->rbDeclividadeTerreno_2->setChecked(true);
        else if (query.value("g1_declividade") == 3)
            ui->rbDeclividadeTerreno_3->setChecked(true);
        else if (query.value("g1_declividade") == 4)
            ui->rbDeclividadeTerreno_4->setChecked(true);
        else if (query.value("g1_declividade") == 5)
            ui->rbDeclividadeTerreno_5->setChecked(true);
        // 1. Caracterização do lixão [fim]

        // 2. Solo e águas subterrâneas [início]
        if (query.value("g2_impermeabilizacao_inferior") == 0)
            ui->rbImpermeabilizacaoInferior_0->setChecked(true);
        else if (query.value("g2_impermeabilizacao_inferior") == 1)
            ui->rbImpermeabilizacaoInferior_1->setChecked(true);
        else if (query.value("g2_impermeabilizacao_inferior") == 2)
            ui->rbImpermeabilizacaoInferior_2->setChecked(true);
        else if (query.value("g2_impermeabilizacao_inferior") == 3)
            ui->rbImpermeabilizacaoInferior_3->setChecked(true);

        if (query.value("g2_presenca_lixiviado") == 0)
            ui->rbPresencaLixiviado_0->setChecked(true);
        else if (query.value("g2_presenca_lixiviado") == 1)
            ui->rbPresencaLixiviado_1->setChecked(true);
        else if (query.value("g2_presenca_lixiviado") == 2)
            ui->rbPresencaLixiviado_2->setChecked(true);

        if (query.value("g2_coleta_lixiviado") == 0)
            ui->rbColetaLixiviado_0->setChecked(true);
        else if (query.value("g2_coleta_lixiviado") == 1)
            ui->rbColetaLixiviado_1->setChecked(true);
        else if (query.value("g2_coleta_lixiviado") == 2)
            ui->rbColetaLixiviado_2->setChecked(true);

        if (query.value("g2_tratamento_lixiviado") == 0)
            ui->rbTratamentoLixiviado_0->setChecked(true);
        else if (query.value("g2_tratamento_lixiviado") == 1)
            ui->rbTratamentoLixiviado_1->setChecked(true);
        else if (query.value("g2_tratamento_lixiviado") == 2)
            ui->rbTratamentoLixiviado_2->setChecked(true);

        if (query.value("g2_natureza_solo") == 0)
            ui->rbNaturezaSolo_0->setChecked(true);
        else if (query.value("g2_natureza_solo") == 1)
            ui->rbNaturezaSolo_1->setChecked(true);
        else if (query.value("g2_natureza_solo") == 2)
            ui->rbNaturezaSolo_2->setChecked(true);

        if (query.value("g2_permeabilidade_solo") == 0)
            ui->rbPermeabilidadeSolo_0->setChecked(true);
        else if (query.value("g2_permeabilidade_solo") == 1)
            ui->rbPermeabilidadeSolo_1->setChecked(true);
        else if (query.value("g2_permeabilidade_solo") == 2)
            ui->rbPermeabilidadeSolo_2->setChecked(true);

        if (query.value("g2_nivel_piezometrico") == 0)
        {
            ui->rbNivelPiezometrico_0->setChecked(true);
        }
        else if (query.value("g2_nivel_piezometrico") == 1)
        {
            ui->rbNivelPiezometrico_1->setChecked(true);
        }
        else if (query.value("g2_nivel_piezometrico") == 2)
        {
            ui->rbNivelPiezometrico_2->setChecked(true);
            ui->lbDistanciaResiduosAgua->setEnabled(true);

            if (query.value("g2_distancia_residuos_agua") == 0)
            {
                ui->rbDistanciaResiduosAgua_0->setEnabled(true);
                ui->rbDistanciaResiduosAgua_0->setChecked(true);
                ui->rbDistanciaResiduosAgua_1->setEnabled(true);
                ui->rbDistanciaResiduosAgua_2->setEnabled(true);
            }

            else if (query.value("g2_distancia_residuos_agua") == 1)
            {
                ui->rbDistanciaResiduosAgua_0->setEnabled(true);
                ui->rbDistanciaResiduosAgua_1->setEnabled(true);
                ui->rbDistanciaResiduosAgua_1->setChecked(true);
                ui->rbDistanciaResiduosAgua_2->setEnabled(true);
            }

            else if (query.value("g2_distancia_residuos_agua") == 2)
            {
                ui->rbDistanciaResiduosAgua_0->setEnabled(true);
                ui->rbDistanciaResiduosAgua_1->setEnabled(true);
                ui->rbDistanciaResiduosAgua_2->setEnabled(true);
                ui->rbDistanciaResiduosAgua_2->setChecked(true);
            }

        }

        if (query.value("g2_descontinuidade_terreno") == 0)
            ui->rbDescontinuidadeTerreno_0->setChecked(true);
        else if (query.value("g2_descontinuidade_terreno") == 1)
            ui->rbDescontinuidadeTerreno_1->setChecked(true);
        else if (query.value("g2_descontinuidade_terreno") == 2)
                ui->rbDescontinuidadeTerreno_2->setChecked(true);

        if (query.value("g2_contaminacao_solo") == 0)
            ui->rbContaminacaoSolo_0->setChecked(true);
        else if (query.value("g2_contaminacao_solo") == 1)
            ui->rbContaminacaoSolo_1->setChecked(true);
        else if (query.value("g2_contaminacao_solo") == 2)
            ui->rbContaminacaoSolo_2->setChecked(true);

        if (query.value("g2_contaminacao_agua") == 0)
            ui->rbContaminacaoAgua_0->setChecked(true);
        else if (query.value("g2_contaminacao_agua") == 1)
            ui->rbContaminacaoAgua_1->setChecked(true);
        else if (query.value("g2_contaminacao_agua") == 2)
            ui->rbContaminacaoAgua_2->setChecked(true);

        if (query.value("g2_distancia_abast_domestico") == 0)
            ui->rbDistanciaAbastDomesticoSub_0->setChecked(true);
        else if (query.value("g2_distancia_abast_domestico") == 1)
            ui->rbDistanciaAbastDomesticoSub_1->setChecked(true);
        else if (query.value("g2_distancia_abast_domestico") == 2)
            ui->rbDistanciaAbastDomesticoSub_2->setChecked(true);
        else if (query.value("g2_distancia_abast_domestico") == 3)
            ui->rbDistanciaAbastDomesticoSub_3->setChecked(true);

        if (query.value("g2_distancia_abast_publico") == 0)
            ui->rbDistanciaAbastPublicoSub_0->setChecked(true);
        else if (query.value("g2_distancia_abast_publico") == 1)
            ui->rbDistanciaAbastPublicoSub_1->setChecked(true);
        else if (query.value("g2_distancia_abast_publico") == 2)
            ui->rbDistanciaAbastPublicoSub_2->setChecked(true);

        if (query.value("g2_uso_preponderante_agua") == 0)
            ui->rbUsoPreponderanteAguaSub_0->setChecked(true);
        else if (query.value("g2_uso_preponderante_agua") == 1)
            ui->rbUsoPreponderanteAguaSub_1->setChecked(true);
        else if (query.value("g2_uso_preponderante_agua") == 2)
            ui->rbUsoPreponderanteAguaSub_2->setChecked(true);
        // 2. Solo e águas subterrâneas [fim]

        // 3. Águas superficiais [início]
        if (query.value("g3_distancia_abast_domestico") == 0)
            ui->rbDistanciaAbastDomesticoSup_0->setChecked(true);
        else if (query.value("g3_distancia_abast_domestico") == 1)
            ui->rbDistanciaAbastDomesticoSup_1->setChecked(true);
        else if (query.value("g3_distancia_abast_domestico") == 2)
            ui->rbDistanciaAbastDomesticoSup_2->setChecked(true);
        else if (query.value("g3_distancia_abast_domestico") == 3)
            ui->rbDistanciaAbastDomesticoSup_3->setChecked(true);

        if (query.value("g3_distancia_abast_publico") == 0)
            ui->rbDistanciaAbastPublicoSup_0->setChecked(true);
        else if (query.value("g3_distancia_abast_publico") == 1)
            ui->rbDistanciaAbastPublicoSup_1->setChecked(true);
        else if (query.value("g3_distancia_abast_publico") == 2)
            ui->rbDistanciaAbastPublicoSup_2->setChecked(true);

        if (query.value("g3_classe_aguas") == 0)
            ui->rbClasseAgua_0->setChecked(true);
        else if (query.value("g3_classe_aguas") == 1)
            ui->rbClasseAgua_1->setChecked(true);
        else if (query.value("g3_classe_aguas") == 2)
            ui->rbClasseAgua_2->setChecked(true);
        else if (query.value("g3_classe_aguas") == 3)
            ui->rbClasseAgua_3->setChecked(true);
        else if (query.value("g3_classe_aguas") == 4)
            ui->rbClasseAgua_4->setChecked(true);

        if (query.value("g3_uso_preponderante_agua") == 0)
            ui->rbUsoPreponderanteAguaSup_0->setChecked(true);
        else if (query.value("g3_uso_preponderante_agua") == 1)
            ui->rbUsoPreponderanteAguaSup_1->setChecked(true);
        else if (query.value("g3_uso_preponderante_agua") == 2)
            ui->rbUsoPreponderanteAguaSup_2->setChecked(true);

        if (query.value("g3_distancia_zona_balneavel") == 0)
            ui->rbDistanciaZonaBalneavel_0->setChecked(true);
        else if (query.value("g3_distancia_zona_balneavel") == 1)
            ui->rbDistanciaZonaBalneavel_1->setChecked(true);
        else if (query.value("g3_distancia_zona_balneavel") == 2)
            ui->rbDistanciaZonaBalneavel_2->setChecked(true);

        if (query.value("g3_distancia_corpo_hidrico") == 0)
            ui->rbDistanciaCorpoHidrico_0->setChecked(true);
        else if (query.value("g3_distancia_corpo_hidrico") == 1)
            ui->rbDistanciaCorpoHidrico_1->setChecked(true);
        else if (query.value("g3_distancia_corpo_hidrico") == 2)
            ui->rbDistanciaCorpoHidrico_2->setChecked(true);

        if (query.value("g3_distancia_nascente") == 0)
            ui->rbDistanciaNascente_0->setChecked(true);
        else if (query.value("g3_distancia_nascente") == 1)
            ui->rbDistanciaNascente_1->setChecked(true);
        else if (query.value("g3_distancia_nascente") == 2)
            ui->rbDistanciaNascente_2->setChecked(true);

        if (query.value("g3_poluicao_agua") == 0)
            ui->rbPoluicaoAgua_0->setChecked(true);
        else if (query.value("g3_poluicao_agua") == 1)
            ui->rbPoluicaoAgua_1->setChecked(true);
        // 3. Águas superficiais [fim]

        // 4. Social [início]
        if (query.value("g4_densidade_populacional") == 0)
            ui->rbDensidadePopulacional_0->setChecked(true);
        else if (query.value("g4_densidade_populacional") == 1)
            ui->rbDensidadePopulacional_1->setChecked(true);
        else if (query.value("g4_densidade_populacional") == 2)
            ui->rbDensidadePopulacional_2->setChecked(true);

        if (query.value("g4_hospital_creche_escola_asilo") == 0)
            ui->rbHospitalCrecheEscolaAsilo_0->setChecked(true);
        else if (query.value("g4_hospital_creche_escola_asilo") == 1)
            ui->rbHospitalCrecheEscolaAsilo_1->setChecked(true);

        if (query.value("g4_distancia_populacao") == 0)
            ui->rbDistanciaPopulacao_0->setChecked(true);
        else if (query.value("g4_distancia_populacao") == 1)
            ui->rbDistanciaPopulacao_1->setChecked(true);
        else if (query.value("g4_distancia_populacao") == 2)
            ui->rbDistanciaPopulacao_2->setChecked(true);

        if (query.value("g4_atividades_agropecuarias") == 0)
            ui->rbAtividadesAgropecuarias_0->setChecked(true);
        else if (query.value("g4_atividades_agropecuarias") == 1)
            ui->rbAtividadesAgropecuarias_1->setChecked(true);
        else if (query.value("g4_atividades_agropecuarias") == 2)
            ui->rbAtividadesAgropecuarias_2->setChecked(true);

        if (query.value("g4_atividades_lazer") == 0)
            ui->rbAtividadesLazer_0->setChecked(true);
        else if (query.value("g4_atividades_lazer") == 1)
            ui->rbAtividadesLazer_1->setChecked(true);
        else if (query.value("g4_atividades_lazer") == 2)
            ui->rbAtividadesLazer_2->setChecked(true);

        if (query.value("g4_isolamento_fisico_lixao") == 0)
            ui->rbIsolamentoFisicoLixao_0->setChecked(true);
        else if (query.value("g4_isolamento_fisico_lixao") == 1)
            ui->rbIsolamentoFisicoLixao_1->setChecked(true);
        else if (query.value("g4_isolamento_fisico_lixao") == 2)
            ui->rbIsolamentoFisicoLixao_2->setChecked(true);

        if (query.value("g4_insetos") == 0)
            ui->cbInsetos->setChecked(false);
        else if (query.value("g4_insetos") == 1)
            ui->cbInsetos->setChecked(true);

        if (query.value("g4_roedores") == 0)
            ui->cbRoedores->setChecked(false);
        else if (query.value("g4_roedores") == 1)
            ui->cbRoedores->setChecked(true);

        if (query.value("g4_escorpioes") == 0)
            ui->cbEscorpioes->setChecked(false);
        else if (query.value("g4_escorpioes") == 1)
            ui->cbEscorpioes->setChecked(true);

        if (query.value("g4_urubus") == 0)
            ui->cbUrubus->setChecked(false);
        else if (query.value("g4_urubus") == 1)
            ui->cbUrubus->setChecked(true);

        if (query.value("g4_outras_aves") == 0)
        {
            ui->cbOutrasAves->setChecked(false);
        }
        else if (query.value("g4_outras_aves") == 1)
        {
            ui->cbOutrasAves->setChecked(true);
            ui->leOutrasAves->setEnabled(true);
            ui->leOutrasAves->setText(query.value("g4_outras_aves_detalhes").toString());
        }

        if (query.value("g4_outros_animais") == 0)
        {
            ui->cbOutrosAnimais->setChecked(false);
        }
        else if (query.value("g4_outros_animais") == 1)
        {
            ui->cbOutrosAnimais->setChecked(true);
            ui->leOutrosAnimais->setEnabled(true);
            ui->leOutrosAnimais->setText(query.value("g4_outros_animais_detalhes").toString());
        }

        if (query.value("g4_danos_saude_populacao") == 0)
            ui->rbDanoSaudePopulacao_0->setChecked(true);
        else if (query.value("g4_danos_saude_populacao") == 1)
            ui->rbDanoSaudePopulacao_1->setChecked(true);
        else if (query.value("g4_danos_saude_populacao") == 2)
            ui->rbDanoSaudePopulacao_2->setChecked(true);
        else if (query.value("g4_danos_saude_populacao") == 3)
            ui->rbDanoSaudePopulacao_3->setChecked(true);

        if (query.value("g4_danos_materiais_populacao") == 0)
            ui->rbDanoMaterialPopulacao_0->setChecked(true);
        else if (query.value("g4_danos_materiais_populacao") == 1)
            ui->rbDanoMaterialPopulacao_1->setChecked(true);
        else if (query.value("g4_danos_materiais_populacao") == 2)
            ui->rbDanoMaterialPopulacao_2->setChecked(true);
        else if (query.value("g4_danos_materiais_populacao") == 3)
            ui->rbDanoMaterialPopulacao_3->setChecked(true);

        if (query.value("g4_catadores") == 0)
            ui->rbCatadores_0->setChecked(true);
        else if (query.value("g4_catadores") == 1)
            ui->rbCatadores_1->setChecked(true);
        // 4. Social [fim]

        // 5.Meio natural e paisagem [início]        
        if (query.value("g5_largura_barreira_vegetal") == 0)
            ui->rbLarguraBarreiraVegetal_0->setChecked(true);
        else if (query.value("g5_largura_barreira_vegetal") == 1)
            ui->rbLarguraBarreiraVegetal_1->setChecked(true);
        else if (query.value("g5_largura_barreira_vegetal") == 2)
            ui->rbLarguraBarreiraVegetal_2->setChecked(true);

        if (query.value("g5_distancia_elemento") == 0)
            ui->rbDistanciaElemento_0->setChecked(true);
        else if (query.value("g5_distancia_elemento") == 1)
            ui->rbDistanciaElemento_1->setChecked(true);
        else if (query.value("g5_distancia_elemento") == 2)
            ui->rbDistanciaElemento_2->setChecked(true);
        else if (query.value("g5_distancia_elemento") == 3)
        {
            ui->rbDistanciaElemento_3->setChecked(true);
            ui->leDistanciaElementoDetalhes->setEnabled(true);
            ui->leDistanciaElementoDetalhes->setText(query.value("g5_distancia_elemento_detalhes").toString());
        }

        if (query.value("g5_desmatamento") == 0)
            ui->rbDesmatamento_0->setChecked(true);
        else if (query.value("g5_desmatamento") == 1)
            ui->rbDesmatamento_1->setChecked(true);
        else if (query.value("g5_desmatamento") == 2)
            ui->rbDesmatamento_2->setChecked(true);

        if (query.value("g5_dispersao_residuos") == 0)
            ui->rbDispersaoResiduos_0->setChecked(true);
        else if (query.value("g5_dispersao_residuos") == 1)
            ui->rbDispersaoResiduos_1->setChecked(true);

        if (query.value("g5_contaminacao_mangue_pantano") == 0)
            ui->rbContaminacaoManguePantano_0->setChecked(true);
        else if (query.value("g5_contaminacao_mangue_pantano") == 1)
            ui->rbContaminacaoManguePantano_1->setChecked(true);
        else if (query.value("g5_contaminacao_mangue_pantano") == 2)
            ui->rbContaminacaoManguePantano_2->setChecked(true);
        else if (query.value("g5_contaminacao_mangue_pantano") == 3)
            ui->rbContaminacaoManguePantano_3->setChecked(true);

        if (query.value("g5_danos_animais") == 0)
            ui->rbDanosAnimais_0->setChecked(true);
        else if (query.value("g5_danos_animais") == 1)
            ui->rbDanosAnimais_1->setChecked(true);
        else if (query.value("g5_danos_animais") == 2)
            ui->rbDanosAnimais_2->setChecked(true);
        // 5.Meio natural e paisagem [fim]

        // 6. Ar [início]
        if (query.value("g6_odores") == 0)
            ui->rbOdoresAr_0->setChecked(true);
        else if (query.value("g6_odores") == 1)
            ui->rbOdoresAr_1->setChecked(true);
        else if (query.value("g6_odores") == 2)
            ui->rbOdoresAr_2->setChecked(true);

        if (query.value("g6_explosoes") == 0)
            ui->rbExplosoes_0->setChecked(true);
        else if (query.value("g6_explosoes") == 1)
            ui->rbExplosoes_1->setChecked(true);
        else if (query.value("g6_explosoes") == 2)
            ui->rbExplosoes_2->setChecked(true);
        else if (query.value("g6_explosoes") == 3)
            ui->rbExplosoes_3->setChecked(true);

        if (query.value("g6_queima_residuos") == 0)
            ui->rbQueimaResiduos_0->setChecked(true);
        else if (query.value("g6_queima_residuos") == 1)
            ui->rbQueimaResiduos_1->setChecked(true);
        else if (query.value("g6_queima_residuos") == 2)
            ui->rbQueimaResiduos_2->setChecked(true);

        if (query.value("g6_bolsoes_migracao_biogas") == 0)
            ui->rbBolsoesMigracaoBiogas_0->setChecked(true);
        else if (query.value("g6_bolsoes_migracao_biogas") == 1)
            ui->rbBolsoesMigracaoBiogas_1->setChecked(true);
        else if (query.value("g6_bolsoes_migracao_biogas") == 2)
            ui->rbBolsoesMigracaoBiogas_2->setChecked(true);

        if (query.value("g6_coleta_gas") == 0)
            ui->rbColetaGas_0->setChecked(true);
        else if (query.value("g6_coleta_gas") == 1)
            ui->rbColetaGas_1->setChecked(true);
        else if (query.value("g6_coleta_gas") == 2)
            ui->rbColetaGas_2->setChecked(true);
        else if (query.value("g6_coleta_gas") == 3)
            ui->rbColetaGas_3->setChecked(true);

        if (query.value("g6_tratamento_gas") == 0)
            ui->rbTratamentoGas_0->setChecked(true);
        else if (query.value("g6_tratamento_gas") == 1)
            ui->rbTratamentoGas_1->setChecked(true);
        else if (query.value("g6_tratamento_gas") == 2)
            ui->rbTratamentoGas_2->setChecked(true);
        // 6. Ar [fim]
    }

    con.close();
}

void lixoes_cad::inserir()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    // http://www.qtcentre.org/threads/64448-placeholder-problem
    QSqlQuery query;
    query.prepare("INSERT INTO lixoes "
                  "(nome, "
                  "endereco, "
                  "bairro, "
                  "cep, "
                  "id_municipio, "
                  "latitude, "
                  "longitude, "
                  "folha_topografica, "
                  "coordenadas_utm, "
                  "datum_mc, "
                  "bacia_hidrografica, "
                  "data_visita, "
                  "inicio_atividades, "
                  "fim_atividades, "
                  "enquadramento_plano_diretor, "
                  "uso_futuro, "
                  "proprietario_terreno, "
                  "informantes, "
                  "acompanhantes, "
                  "condicao_tempo, "
                  "condicao_vento, "
                  "acumulado_chuva, "
                  "lei_9985, "
                  "area_urbana_rural, "
                  "ocorrencia_talvegue, "
                  "ocorrencia_varzea, "
                  "ocorrencia_corpos_hidricos, "
                  "area_industrial, "
                  "g1_area, "
                  "g1_atividade, "
                  "g1_adensamento, "
                  "g1_deslizamento, "
                  "g1_erosao, "
                  "g1_outros, "
                  "g1_outros_detalhes, "
                  "g1_espessura, "
                  "g1_residuos_classe_IIB, "
                  "g1_residuos_classe_IIA, "
                  "g1_residuos_classe_I, "
                  "g1_residuos_classe_I_detalhes, "
                  "g1_impermeabilizacao_superior, "
                  "g1_pluviometria, "
                  "g1_declividade, "
                  "g2_impermeabilizacao_inferior, "
                  "g2_presenca_lixiviado, "
                  "g2_coleta_lixiviado, "
                  "g2_tratamento_lixiviado, "
                  "g2_natureza_solo, "
                  "g2_permeabilidade_solo, "
                  "g2_nivel_piezometrico, "
                  "g2_distancia_residuos_agua, "
                  "g2_descontinuidade_terreno, "
                  "g2_contaminacao_solo, "
                  "g2_contaminacao_agua, "
                  "g2_distancia_abast_domestico, "
                  "g2_distancia_abast_publico, "
                  "g2_uso_preponderante_agua, "
                  "g3_distancia_abast_domestico, "
                  "g3_distancia_abast_publico, "
                  "g3_classe_aguas, "
                  "g3_uso_preponderante_agua, "
                  "g3_distancia_zona_balneavel, "
                  "g3_distancia_corpo_hidrico, "
                  "g3_distancia_nascente, "
                  "g3_poluicao_agua, "
                  "g4_densidade_populacional, "
                  "g4_hospital_creche_escola_asilo, "
                  "g4_distancia_populacao, "
                  "g4_atividades_agropecuarias, "
                  "g4_atividades_lazer, "
                  "g4_isolamento_fisico_lixao, "
                  "g4_insetos, "
                  "g4_roedores, "
                  "g4_escorpioes, "
                  "g4_urubus, "
                  "g4_outras_aves, "
                  "g4_outras_aves_detalhes, "
                  "g4_outros_animais, "
                  "g4_outros_animais_detalhes, "
                  "g4_danos_saude_populacao, "
                  "g4_danos_materiais_populacao, "
                  "g4_catadores, "
                  "g5_largura_barreira_vegetal, "
                  "g5_distancia_elemento, "
                  "g5_distancia_elemento_detalhes, "
                  "g5_desmatamento, "
                  "g5_dispersao_residuos, "
                  "g5_contaminacao_mangue_pantano, "
                  "g5_danos_animais, "
                  "g6_odores, "
                  "g6_explosoes, "
                  "g6_queima_residuos, "
                  "g6_bolsoes_migracao_biogas, "
                  "g6_coleta_gas, "
                  "g6_tratamento_gas) "
                  "VALUES "
                  "(:nomeLixao, "
                  ":enderecoLixao, "
                  ":bairroLixao, "
                  ":cepLixao, "
                  ":idMunicipioLixao, "
                  ":latitudeLixao, "
                  ":longitudeLixao, "
                  ":folhaTopograficaLixao, "
                  ":coordenadasUtmLixao, "
                  ":datumMcLixao, "
                  ":baciaHidrograficaLixao, "
                  ":dataVisitaLixao, "
                  ":inicioAtividadesLixao, "
                  ":fimAtividadesLixao, "
                  ":enquadramentoPlanoDiretorLixao, "
                  ":usoFuturoLixao, "
                  ":proprietarioTerrenoLixao, "
                  ":informantesLixao, "
                  ":acompanhantesLixao, "
                  ":condicaoTempoLixao, "
                  ":condicaoVentoLixao, "
                  ":acumuladoChuvaLixao, "
                  ":lei9985Lixao, "
                  ":areaUrbanaRuralLixao, "
                  ":ocorrenciaTalvegueLixao, "
                  ":ocorrenciaVarzeaLixao, "
                  ":ocorrenciaCorposHidricosLixao, "
                  ":areaIndustrialLixao, "
                  ":1AreaLixao, "
                  ":1AtividadeLixao, "
                  ":1AdensamentoLixao, "
                  ":1DeslizamentoLixao, "
                  ":1ErosaoLixao, "
                  ":1OutrosLixao, "
                  ":1OutrosDetalhesLixao, "
                  ":1EspessuraLixao, "
                  ":1ResiduosClasseIIBLixao, "
                  ":1ResiduosClasseIIALixao, "
                  ":1ResiduosClasseILixao, "
                  ":1ResiduosClasseIDetalhesLixao, "
                  ":1ImpermeabilizacaoSuperiorLixao, "
                  ":1PluviometriaLixao, "
                  ":1DeclividadeLixao, "
                  ":2ImpermeabilizacaoInferiorLixao, "
                  ":2PresencaLixiviadoLixao, "
                  ":2ColetaLixiviadoLixao, "
                  ":2TratamentoLixiviadoLixao, "
                  ":2NaturezaSoloLixao, "
                  ":2PermeabilidadeSoloLixao, "
                  ":2NivelPiezometricoLixao, "
                  ":2DistanciaResiduosAguaLixao, "
                  ":2DescontinuidadeTerrenoLixao, "
                  ":2ContaminacaoSoloLixao, "
                  ":2ContaminacaoAguaLixao, "
                  ":2DistanciaAbastDomesticoLixao, "
                  ":2DistanciaAbastPublicoLixao, "
                  ":2UsoPreponderanteAguaLixao, "
                  ":3DistanciaAbastDomesticoLixao, "
                  ":3DistanciaAbastPublicoLixao, "
                  ":3ClasseAguasLixao, "
                  ":3UsoPreponderanteAguaLixao, "
                  ":3DistanciaZonaBalneavelLixao, "
                  ":3DistanciaCorpoHidricoLixao, "
                  ":3DistanciaNascenteLixao, "
                  ":3PoluicaoAguaLixao, "
                  ":4DensidadePopulacionalLixao, "
                  ":4HospitalCrecheEscolaAsiloLixao, "
                  ":4DistanciaPopulacaoLixao, "
                  ":4AtividadesAgropecuariasLixao, "
                  ":4AtividadesLazerLixao, "
                  ":4IsolamentoFisicoLixao, "
                  ":4InsetosLixao, "
                  ":4RoedoresLixao, "
                  ":4EscorpioesLixao, "
                  ":4UrubusLixao, "
                  ":4OutrasAvesLixao, "
                  ":4OutrasAvesDetalhesLixao, "
                  ":4OutrosAnimaisLixao, "
                  ":4OutrosAnimaisDetalhesLixao, "
                  ":4DanosSaudePopulacaoLixao, "
                  ":4DanosMateriaisPopulacaoLixao, "
                  ":4CatadoresLixao, "
                  ":5LarguraBarreiraVegetalLixao, "
                  ":5DistanciaElementoLixao, "
                  ":5DistanciaElementoDetalhesLixao, "
                  ":5DesmatamentoLixao, "
                  ":5DispersaoResiduosLixao, "
                  ":5ContaminacaoManguePantanoLixao, "
                  ":5DanosAnimaisLixao, "
                  ":6OdoresLixao, "
                  ":6ExplosoesLixao, "
                  ":6QueimaResiduosLixao, "
                  ":6BolsoesMigracaoBiogasLixao, "
                  ":6ColetaGasLixao, "
                  ":6TratamentoGasLixao) ");

    // 0. Caracterização da área [início]
    query.bindValue(":nomeLixao",ui->leNome->text());
    query.bindValue(":enderecoLixao",ui->leEndereco->text());
    query.bindValue(":bairroLixao",ui->leBairro->text());
    query.bindValue(":cepLixao",ui->leCEP->text());

    QSqlQuery query2;
    query2.prepare("SELECT * FROM municipios where municipio = :municipioLixao ");
    query2.bindValue(":municipioLixao",ui->cbMunicipios->currentText());
    query2.exec();
    while (query2.next())
    {
        query.bindValue(":idMunicipioLixao",query2.value(0).toInt());
    }

    query.bindValue(":latitudeLixao",ui->leLatitude->text());
    query.bindValue(":longitudeLixao",ui->leLongitude->text());
    query.bindValue(":folhaTopograficaLixao",ui->leFolhaTopografica->text());
    query.bindValue(":coordenadasUtmLixao",ui->leUTM->text());
    query.bindValue(":datumMcLixao",ui->leDatum->text());
    query.bindValue(":baciaHidrograficaLixao",ui->leBaciaHidrografica->text());
    query.bindValue(":dataVisitaLixao",ui->leDataVisita->text());
    query.bindValue(":inicioAtividadesLixao",ui->leDataInicioAtividades->text());
    query.bindValue(":fimAtividadesLixao",ui->leDataFimAtividades->text());
    query.bindValue(":enquadramentoPlanoDiretorLixao",ui->leEnquadramentoPlanoDiretor->text());
    query.bindValue(":usoFuturoLixao",ui->leUsoFuturo->text());
    query.bindValue(":proprietarioTerrenoLixao",ui->leProprietario->text());
    query.bindValue(":informantesLixao",ui->leInformantes->text());
    query.bindValue(":acompanhantesLixao",ui->leAcompanhantes->text());

    if (ui->rbCondicaoTempo_0->isChecked())
        query.bindValue(":condicaoTempoLixao", 0);
    else if (ui->rbCondicaoTempo_1->isChecked())
        query.bindValue(":condicaoTempoLixao", 1);
    else if (ui->rbCondicaoTempo_2->isChecked())
        query.bindValue(":condicaoTempoLixao", 2);
    else if (ui->rbCondicaoTempo_3->isChecked())
        query.bindValue(":condicaoTempoLixao", 3);
    else if (ui->rbCondicaoTempo_4->isChecked())
        query.bindValue(":condicaoTempoLixao", 4);

    query.bindValue(":condicaoVentoLixao",ui->leCondicaoVento->text());
    query.bindValue(":acumuladoChuvaLixao",ui->leAcumuladoChuva->text());

    if (ui->rbLei9985_0->isChecked())
        query.bindValue(":lei9985Lixao", 0);
    else if (ui->rbLei9985_1->isChecked())
        query.bindValue(":lei9985Lixao", 1);

    if (ui->rbAreaUrbanaRural_0->isChecked())
        query.bindValue(":areaUrbanaRuralLixao", 0);
    else if (ui->rbAreaUrbanaRural_1->isChecked())
        query.bindValue(":areaUrbanaRuralLixao", 1);

    if (ui->rbOcorrenciaTalvegue_0->isChecked())
        query.bindValue(":ocorrenciaTalvegueLixao", 0);
    else if (ui->rbOcorrenciaTalvegue_1->isChecked())
        query.bindValue(":ocorrenciaTalvegueLixao", 1);

    if (ui->rbOcorrenciaVarzea_0->isChecked())
        query.bindValue(":ocorrenciaVarzeaLixao", 0);
    else if (ui->rbOcorrenciaVarzea_1->isChecked())
        query.bindValue(":ocorrenciaVarzeaLixao", 1);

    query.bindValue(":ocorrenciaCorposHidricosLixao", ui->sbQuantidadeCorposHidricos->value());

    if (ui->rbAreaIndustrial_0->isChecked())
        query.bindValue(":areaIndustrialLixao", 0);
    if (ui->rbAreaIndustrial_1->isChecked())
        query.bindValue(":areaIndustrialLixao", 1);
    // 0. Caracterização da área [fim]

    // 1. Caracterização do lixão [inicio]
    if (ui->rbAreaLixao_0->isChecked())
        query.bindValue(":1AreaLixao", 0);
    else if (ui->rbAreaLixao_1->isChecked())
        query.bindValue(":1AreaLixao", 1);
    else if (ui->rbAreaLixao_2->isChecked())
        query.bindValue(":1AreaLixao", 2);
    else if (ui->rbAreaLixao_3->isChecked())
        query.bindValue(":1AreaLixao", 3);

    if (ui->rbAtividadeLixao_0->isChecked())
        query.bindValue(":1AtividadeLixao", 0);
    else if (ui->rbAtividadeLixao_1->isChecked())
        query.bindValue(":1AtividadeLixao", 1);
    else if (ui->rbAtividadeLixao_2->isChecked())
        query.bindValue(":1AtividadeLixao", 2);
    else if (ui->rbAtividadeLixao_3->isChecked())
        query.bindValue(":1AtividadeLixao", 3);

    if (ui->rbAdensamento_0->isChecked())
        query.bindValue(":1AdensamentoLixao", 0);
    else if (ui->rbAdensamento_1->isChecked())
        query.bindValue(":1AdensamentoLixao", 1);

    if (ui->rbDeslizamento_0->isChecked())
        query.bindValue(":1DeslizamentoLixao", 0);
    else if (ui->rbDeslizamento_1->isChecked())
        query.bindValue(":1DeslizamentoLixao", 1);

    if (ui->rbErosao_0->isChecked())
        query.bindValue(":1ErosaoLixao", 0);
    else if (ui->rbErosao_1->isChecked())
        query.bindValue(":1ErosaoLixao", 1);

    if (ui->rbOutrosAcidentes_0->isChecked())
        query.bindValue(":1OutrosLixao", 0);
    else if (ui->rbOutrosAcidentes_1->isChecked())
    {
        query.bindValue(":1OutrosLixao", 1);
        query.bindValue(":1OutrosDetalhesLixao", ui->leOutrosAcidentesDetalhes->text());
    }

    if (ui->rbEspessuraCamada_0->isChecked())
        query.bindValue(":1EspessuraLixao", 0);
    else if (ui->rbEspessuraCamada_1->isChecked())
        query.bindValue(":1EspessuraLixao", 1);
    else if (ui->rbEspessuraCamada_2->isChecked())
        query.bindValue(":1EspessuraLixao", 2);
    else if (ui->rbEspessuraCamada_3->isChecked())
        query.bindValue(":1EspessuraLixao", 3);

    if (ui->cbNaturezaResiduos_0->isChecked())
        query.bindValue(":1ResiduosClasseIIBLixao",1);
    else
        query.bindValue(":1ResiduosClasseIIBLixao",0);

    if (ui->cbNaturezaResiduos_1->isChecked())
        query.bindValue(":1ResiduosClasseIIALixao",1);
    else
        query.bindValue(":1ResiduosClasseIIALixao",0);

    if (ui->cbNaturezaResiduos_2->isChecked())
    {
        query.bindValue(":1ResiduosClasseILixao",1);
        query.bindValue(":1ResiduosClasseIDetalhesLixao",ui->leNaturezaResiduosDetalhes->text());
    }
    else
        query.bindValue(":1_residuos_classe_I",0);

    if (ui->rbImpermeabilizacaoSuperior_0->isChecked())
        query.bindValue(":1ImpermeabilizacaoSuperiorLixao", 0);
    else if (ui->rbImpermeabilizacaoSuperior_1->isChecked())
        query.bindValue(":1ImpermeabilizacaoSuperiorLixao", 1);
    else if (ui->rbImpermeabilizacaoSuperior_2->isChecked())
        query.bindValue(":1ImpermeabilizacaoSuperiorLixao", 2);

    if (ui->rbPluviometria_0->isChecked())
        query.bindValue(":1PluviometriaLixao", 0);
    else if (ui->rbPluviometria_1->isChecked())
        query.bindValue(":1PluviometriaLixao", 1);
    else if (ui->rbPluviometria_2->isChecked())
        query.bindValue(":1PluviometriaLixao", 2);
    else if (ui->rbPluviometria_3->isChecked())
        query.bindValue(":1PluviometriaLixao", 3);

    if (ui->rbDeclividadeTerreno_0->isChecked())
        query.bindValue(":1DeclividadeLixao", 0);
    else if (ui->rbDeclividadeTerreno_1->isChecked())
        query.bindValue(":1DeclividadeLixao", 1);
    else if (ui->rbDeclividadeTerreno_2->isChecked())
        query.bindValue(":1DeclividadeLixao", 2);
    else if (ui->rbDeclividadeTerreno_3->isChecked())
        query.bindValue(":1DeclividadeLixao", 3);
    else if (ui->rbDeclividadeTerreno_4->isChecked())
        query.bindValue(":1DeclividadeLixao", 4);
    else if (ui->rbDeclividadeTerreno_5->isChecked())
        query.bindValue(":1DeclividadeLixao", 5);
    // 1. Caracterização do lixão [fim]

    // 2. Solo e águas subterrâneas [inicio]
    if (ui->rbImpermeabilizacaoInferior_0->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 0);
    else if (ui->rbImpermeabilizacaoInferior_1->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 1);
    else if (ui->rbImpermeabilizacaoInferior_2->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 2);
    else if (ui->rbImpermeabilizacaoInferior_3->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 3);

    if (ui->rbPresencaLixiviado_0->isChecked())
        query.bindValue(":2PresencaLixiviadoLixao", 0);
    else if (ui->rbPresencaLixiviado_1->isChecked())
        query.bindValue(":2PresencaLixiviadoLixao", 1);
    else if (ui->rbPresencaLixiviado_2->isChecked())
        query.bindValue(":2PresencaLixiviadoLixao", 2);

    if (ui->rbColetaLixiviado_0->isChecked())
        query.bindValue(":2ColetaLixiviadoLixao", 0);
    else if (ui->rbColetaLixiviado_1->isChecked())
        query.bindValue(":2ColetaLixiviadoLixao", 1);
    else if (ui->rbColetaLixiviado_2->isChecked())
        query.bindValue(":2ColetaLixiviadoLixao", 2);

    if (ui->rbTratamentoLixiviado_0->isChecked())
        query.bindValue(":2TratamentoLixiviadoLixao", 0);
    else if (ui->rbTratamentoLixiviado_1->isChecked())
        query.bindValue(":2TratamentoLixiviadoLixao", 1);
    else if (ui->rbTratamentoLixiviado_2->isChecked())
        query.bindValue(":2TratamentoLixiviadoLixao", 2);

    if (ui->rbNaturezaSolo_0->isChecked())
        query.bindValue(":2NaturezaSoloLixao", 0);
    else if (ui->rbNaturezaSolo_1->isChecked())
        query.bindValue(":2NaturezaSoloLixao", 1);
    else if (ui->rbNaturezaSolo_2->isChecked())
        query.bindValue(":2NaturezaSoloLixao", 2);

    if (ui->rbPermeabilidadeSolo_0->isChecked())
        query.bindValue(":2PermeabilidadeSoloLixao", 0);
    else if (ui->rbPermeabilidadeSolo_1->isChecked())
        query.bindValue(":2PermeabilidadeSoloLixao", 1);
    else if (ui->rbPermeabilidadeSolo_2->isChecked())
        query.bindValue(":2PermeabilidadeSoloLixao", 2);

    if (ui->rbNivelPiezometrico_0->isChecked())
    {
        query.bindValue(":2NivelPiezometricoLixao", 0);
        query.bindValue(":2DistanciaResiduosAguaLixao", -1);
    }
    else if (ui->rbNivelPiezometrico_1->isChecked())
    {
        query.bindValue(":2NivelPiezometricoLixao", 1);
        query.bindValue(":2DistanciaResiduosAguaLixao", -1);
    }
    else if (ui->rbNivelPiezometrico_2->isChecked())
    {
        query.bindValue(":2NivelPiezometricoLixao", 2);

        if (ui->rbDistanciaResiduosAgua_0->isChecked())
            query.bindValue(":2DistanciaResiduosAguaLixao", 0);
        else if (ui->rbDistanciaResiduosAgua_1->isChecked())
            query.bindValue(":2DistanciaResiduosAguaLixao", 1);
        else if (ui->rbDistanciaResiduosAgua_2->isChecked())
            query.bindValue(":2DistanciaResiduosAguaLixao", 2);
    }
    else
        query.bindValue(":2DistanciaResiduosAguaLixao", -1);

    if (ui->rbDescontinuidadeTerreno_0->isChecked())
        query.bindValue(":2DescontinuidadeTerrenoLixao", 0);
    else if (ui->rbDescontinuidadeTerreno_1->isChecked())
        query.bindValue(":2DescontinuidadeTerrenoLixao", 1);
    else if (ui->rbDescontinuidadeTerreno_2->isChecked())
        query.bindValue(":2DescontinuidadeTerrenoLixao", 2);

    if (ui->rbContaminacaoSolo_0->isChecked())
        query.bindValue(":2ContaminacaoSoloLixao", 0);
    else if (ui->rbContaminacaoSolo_1->isChecked())
        query.bindValue(":2ContaminacaoSoloLixao", 1);
    else if (ui->rbContaminacaoSolo_2->isChecked())
        query.bindValue(":2ContaminacaoSoloLixao", 2);

    if (ui->rbContaminacaoAgua_0->isChecked())
        query.bindValue(":2ContaminacaoAguaLixao", 0);
    else if (ui->rbContaminacaoAgua_1->isChecked())
        query.bindValue(":2ContaminacaoAguaLixao", 1);
    else if (ui->rbContaminacaoAgua_2->isChecked())
        query.bindValue(":2ContaminacaoAguaLixao", 2);

    if (ui->rbDistanciaAbastDomesticoSub_0->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 0);
    else if (ui->rbDistanciaAbastDomesticoSub_1->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 1);
    else if (ui->rbDistanciaAbastDomesticoSub_2->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 2);
    else if (ui->rbDistanciaAbastDomesticoSub_3->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 3);

    if (ui->rbDistanciaAbastPublicoSub_0->isChecked())
        query.bindValue(":2DistanciaAbastPublicoLixao", 0);
    else if (ui->rbDistanciaAbastPublicoSub_1->isChecked())
        query.bindValue(":2DistanciaAbastPublicoLixao", 1);
    else if (ui->rbDistanciaAbastPublicoSub_2->isChecked())query.bindValue(":2DistanciaResiduosAguaLixao", 0);
        query.bindValue(":2DistanciaAbastPublicoLixao", 2);

    if (ui->rbUsoPreponderanteAguaSub_0->isChecked())
        query.bindValue(":2UsoPreponderanteAguaLixao", 0);
    else if (ui->rbUsoPreponderanteAguaSub_1->isChecked())
        query.bindValue(":2UsoPreponderanteAguaLixao", 1);
    else if (ui->rbUsoPreponderanteAguaSub_2->isChecked())
        query.bindValue(":2UsoPreponderanteAguaLixao", 2);
    // 2. Solo e águas subterrâneas [fim]


    // 3. Águas superficiais [início]
    if (ui->rbDistanciaAbastDomesticoSup_0->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 0);
    else if (ui->rbDistanciaAbastDomesticoSup_1->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 1);
    else if (ui->rbDistanciaAbastDomesticoSup_2->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 2);
    else if (ui->rbDistanciaAbastDomesticoSup_3->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 3);

    if (ui->rbDistanciaAbastPublicoSup_0->isChecked())
        query.bindValue(":3DistanciaAbastPublicoLixao", 0);
    else if (ui->rbDistanciaAbastPublicoSup_1->isChecked())
        query.bindValue(":3DistanciaAbastPublicoLixao", 1);
    else if (ui->rbDistanciaAbastPublicoSup_2->isChecked())
        query.bindValue(":3DistanciaAbastPublicoLixao", 2);

    if (ui->rbClasseAgua_0->isChecked())
        query.bindValue(":3ClasseAguasLixao", 0);
    else if (ui->rbClasseAgua_1->isChecked())
        query.bindValue(":3ClasseAguasLixao", 1);
    else if (ui->rbClasseAgua_2->isChecked())
        query.bindValue(":3ClasseAguasLixao", 2);
    else if (ui->rbClasseAgua_3->isChecked())
        query.bindValue(":3ClasseAguasLixao", 3);
    else if (ui->rbClasseAgua_4->isChecked())
        query.bindValue(":3ClasseAguasLixao", 4);

    if (ui->rbUsoPreponderanteAguaSup_0->isChecked())
        query.bindValue(":3UsoPreponderanteAguaLixao", 0);
    else if (ui->rbUsoPreponderanteAguaSup_1->isChecked())
        query.bindValue(":3UsoPreponderanteAguaLixao", 1);
    else if (ui->rbUsoPreponderanteAguaSup_2->isChecked())
        query.bindValue(":3UsoPreponderanteAguaLixao", 2);

    if (ui->rbDistanciaZonaBalneavel_0->isChecked())
        query.bindValue(":3DistanciaZonaBalneavelLixao", 0);
    else if (ui->rbDistanciaZonaBalneavel_1->isChecked())
        query.bindValue(":3DistanciaZonaBalneavelLixao", 1);
    else if (ui->rbDistanciaZonaBalneavel_2->isChecked())
        query.bindValue(":3DistanciaZonaBalneavelLixao", 2);

    if (ui->rbDistanciaCorpoHidrico_0->isChecked())
        query.bindValue(":3DistanciaCorpoHidricoLixao", 0);
    else if (ui->rbDistanciaCorpoHidrico_1->isChecked())
        query.bindValue(":3DistanciaCorpoHidricoLixao", 1);
    else if (ui->rbDistanciaCorpoHidrico_2->isChecked())
        query.bindValue(":3DistanciaCorpoHidricoLixao", 2);

    if (ui->rbDistanciaNascente_0->isChecked())
        query.bindValue(":3DistanciaNascenteLixao", 0);
    else if (ui->rbDistanciaNascente_1->isChecked())
        query.bindValue(":3DistanciaNascenteLixao", 1);
    else if (ui->rbDistanciaNascente_2->isChecked())
        query.bindValue(":3DistanciaNascenteLixao", 2);

    if (ui->rbPoluicaoAgua_0->isChecked())
        query.bindValue(":3PoluicaoAguaLixao", 0);
    else if (ui->rbPoluicaoAgua_1->isChecked())
        query.bindValue(":3PoluicaoAguaLixao", 1);
    // 3. Águas superficiais [fim]

    // 4. Social [início]
    if (ui->rbDensidadePopulacional_0->isChecked())
        query.bindValue(":4DensidadePopulacionalLixao", 0);
    else if (ui->rbDensidadePopulacional_1->isChecked())
        query.bindValue(":4DensidadePopulacionalLixao", 1);
    else if (ui->rbDensidadePopulacional_2->isChecked())
        query.bindValue(":4DensidadePopulacionalLixao", 2);

    if (ui->rbHospitalCrecheEscolaAsilo_0->isChecked())
        query.bindValue(":4HospitalCrecheEscolaAsiloLixao", 0);
    else if (ui->rbHospitalCrecheEscolaAsilo_1->isChecked())
        query.bindValue(":4HospitalCrecheEscolaAsiloLixao", 1);

    if (ui->rbDistanciaPopulacao_0->isChecked())
        query.bindValue(":4DistanciaPopulacaoLixao", 0);
    else if (ui->rbDistanciaPopulacao_1->isChecked())
        query.bindValue(":4DistanciaPopulacaoLixao", 1);
    else if (ui->rbDistanciaPopulacao_2->isChecked())
        query.bindValue(":4DistanciaPopulacaoLixao", 2);

    if (ui->rbAtividadesAgropecuarias_0->isChecked())
        query.bindValue(":4AtividadesAgropecuariasLixao", 0);
    else if (ui->rbAtividadesAgropecuarias_1->isChecked())
        query.bindValue(":4AtividadesAgropecuariasLixao", 1);
    else if (ui->rbAtividadesAgropecuarias_2->isChecked())
        query.bindValue(":4AtividadesAgropecuariasLixao", 2);

    if (ui->rbAtividadesLazer_0->isChecked())
        query.bindValue(":4AtividadesLazerLixao", 0);
    else if (ui->rbAtividadesLazer_1->isChecked())
        query.bindValue(":4AtividadesLazerLixao", 1);
    else if (ui->rbAtividadesLazer_2->isChecked())
        query.bindValue(":4AtividadesLazerLixao", 2);

    if (ui->rbIsolamentoFisicoLixao_0->isChecked())
        query.bindValue(":4IsolamentoFisicoLixao", 0);
    else if (ui->rbIsolamentoFisicoLixao_1->isChecked())
        query.bindValue(":4IsolamentoFisicoLixao", 1);
    else if (ui->rbIsolamentoFisicoLixao_2->isChecked())
        query.bindValue(":4IsolamentoFisicoLixao", 2);

    if (ui->cbInsetos->isChecked())
        query.bindValue(":4InsetosLixao", 1);
    else
        query.bindValue(":4InsetosLixao", 0);

    if (ui->cbRoedores->isChecked())
        query.bindValue(":4RoedoresLixao", 1);
    else
        query.bindValue(":4RoedoresLixao", 0);

    if (ui->cbEscorpioes->isChecked())
        query.bindValue(":4EscorpioesLixao", 1);
    else
        query.bindValue(":4EscorpioesLixao", 0);

    if (ui->cbUrubus->isChecked())
        query.bindValue(":4UrubusLixao", 1);
    else
        query.bindValue(":4UrubusLixao", 0);

    if (ui->cbOutrasAves->isChecked())
    {
        query.bindValue(":4OutrasAvesLixao", 1);
        query.bindValue(":4OutrasAvesDetalhesLixao", ui->leOutrasAves->text());
    }
    else
        query.bindValue(":4OutrasAvesLixao", 0);

    if (ui->cbOutrosAnimais->isChecked())
    {
        query.bindValue(":4OutrosAnimaisLixao", 1);
        query.bindValue(":4OutrosAnimaisDetalhesLixao", ui->leOutrosAnimais->text());
    }
    else
        query.bindValue(":4OutrosAnimaisLixao", 0);

    if (ui->rbDanoSaudePopulacao_0->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 0);
    else if (ui->rbDanoSaudePopulacao_1->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 1);
    else if (ui->rbDanoSaudePopulacao_2->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 2);
    else if (ui->rbDanoSaudePopulacao_3->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 3);

    if (ui->rbDanoMaterialPopulacao_0->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 0);
    else if (ui->rbDanoMaterialPopulacao_1->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 1);
    else if (ui->rbDanoMaterialPopulacao_2->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 2);
    else if (ui->rbDanoMaterialPopulacao_3->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 3);

    if (ui->rbCatadores_0->isChecked())
        query.bindValue(":4CatadoresLixao", 0);
    else if (ui->rbCatadores_1->isChecked())
        query.bindValue(":4CatadoresLixao", 1);
    // 4. Social [fim]

    // 5. Meio natural e paisagens [início]
    if (ui->rbLarguraBarreiraVegetal_0->isChecked())
        query.bindValue(":5LarguraBarreiraVegetalLixao", 0);
    else if (ui->rbLarguraBarreiraVegetal_1->isChecked())
        query.bindValue(":5LarguraBarreiraVegetalLixao", 1);
    else if (ui->rbLarguraBarreiraVegetal_2->isChecked())
        query.bindValue(":5LarguraBarreiraVegetalLixao", 2);

    if (ui->rbDistanciaElemento_0->isChecked())
        query.bindValue(":5DistanciaElementoLixao", 0);
    else if (ui->rbDistanciaElemento_1->isChecked())
        query.bindValue(":5DistanciaElementoLixao", 1);
    else if (ui->rbDistanciaElemento_2->isChecked())
        query.bindValue(":5DistanciaElementoLixao", 2);
    else if (ui->rbDistanciaElemento_3->isChecked())
    {
        query.bindValue(":5DistanciaElementoLixao", 3);
        query.bindValue(":5DistanciaElementoDetalhesLixao", ui->leDistanciaElementoDetalhes->text());
    }

    if (ui->rbDesmatamento_0->isChecked())
        query.bindValue(":5DesmatamentoLixao", 0);
    else if (ui->rbDesmatamento_1->isChecked())
        query.bindValue(":5DesmatamentoLixao", 1);
    else if (ui->rbDesmatamento_2->isChecked())
        query.bindValue(":5DesmatamentoLixao", 2);

    if (ui->rbDispersaoResiduos_0->isChecked())
        query.bindValue(":5DispersaoResiduosLixao", 0);
    else if (ui->rbDispersaoResiduos_1->isChecked())
        query.bindValue(":5DispersaoResiduosLixao", 1);

    if (ui->rbContaminacaoManguePantano_0->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 0);
    else if (ui->rbContaminacaoManguePantano_1->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 1);
    else if (ui->rbContaminacaoManguePantano_2->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 2);
    else if (ui->rbContaminacaoManguePantano_3->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 3);

    if (ui->rbDanosAnimais_0->isChecked())
        query.bindValue(":5DanosAnimaisLixao", 0);
    else if (ui->rbDanosAnimais_1->isChecked())
        query.bindValue(":5DanosAnimaisLixao", 1);
    else if (ui->rbDanosAnimais_2->isChecked())
        query.bindValue(":5DanosAnimaisLixao", 2);
    // 5. Meio natural e paisagens [fim]

    // 6. Ar [início]
    if (ui->rbOdoresAr_0->isChecked())
        query.bindValue(":6OdoresLixao", 0);
    else if (ui->rbOdoresAr_1->isChecked())
        query.bindValue(":6OdoresLixao", 1);
    else if (ui->rbOdoresAr_2->isChecked())
        query.bindValue(":6OdoresLixao", 2);

    if (ui->rbExplosoes_0->isChecked())
        query.bindValue(":6ExplosoesLixao", 0);
    else if (ui->rbExplosoes_1->isChecked())
        query.bindValue(":6ExplosoesLixao", 1);
    else if (ui->rbExplosoes_2->isChecked())
        query.bindValue(":6ExplosoesLixao", 2);
    else if (ui->rbExplosoes_3->isChecked())
        query.bindValue(":6ExplosoesLixao", 3);

    if (ui->rbQueimaResiduos_0->isChecked())
        query.bindValue(":6QueimaResiduosLixao", 0);
    else if (ui->rbQueimaResiduos_1->isChecked())
        query.bindValue(":6QueimaResiduosLixao", 1);
    else if (ui->rbQueimaResiduos_2->isChecked())
        query.bindValue(":6QueimaResiduosLixao", 2);

    if (ui->rbBolsoesMigracaoBiogas_0->isChecked())
        query.bindValue(":6BolsoesMigracaoBiogasLixao", 0);
    else if (ui->rbBolsoesMigracaoBiogas_1->isChecked())
        query.bindValue(":6BolsoesMigracaoBiogasLixao", 1);
    else if (ui->rbBolsoesMigracaoBiogas_2->isChecked())
        query.bindValue(":6BolsoesMigracaoBiogasLixao", 2);

    if (ui->rbColetaGas_0->isChecked())
        query.bindValue(":6ColetaGasLixao", 0);
    else if (ui->rbColetaGas_1->isChecked())
        query.bindValue(":6ColetaGasLixao", 1);
    else if (ui->rbColetaGas_2->isChecked())
        query.bindValue(":6ColetaGasLixao", 2);
    else if (ui->rbColetaGas_3->isChecked())
        query.bindValue(":6ColetaGasLixao", 3);

    if (ui->rbTratamentoGas_0->isChecked())
        query.bindValue(":6TratamentoGasLixao", 0);
    else if (ui->rbTratamentoGas_1->isChecked())
        query.bindValue(":6TratamentoGasLixao", 1);
    else if (ui->rbTratamentoGas_2->isChecked())
        query.bindValue(":6TratamentoGasLixao", 2);
    // 6. Ar [fim]


    if (ui->leNome->text() == NULL)
    {
        QMessageBox msgBox;
        msgBox.setText("O campo Nome é de preenchimento obrigatório");
        msgBox.exec();

        ui->twCadastro->setCurrentIndex(0);
        ui->leNome->setFocus();
    }
    else if (ui->cbMunicipios->currentIndex() == -1)
    {
        QMessageBox msgBox;
        msgBox.setText("O campo Município é de preenchimento obrigatório");
        msgBox.exec();

        ui->twCadastro->setCurrentIndex(0);
    }
    else
    {
        query.exec();

        /*
         * Como o _ID=0, é necessário recuperar o ultimo registro inserido na tabela e atribuir seu id à variável _ID
         * Para que ao fazer o diagnstico do lixão (on_pbDiagnosticar_clicked()), o sistema recupere corretamente
         * o id do lixão após inserir um novo registro, e não _ID=0.
         */
        QSqlQuery querySEQUENCIA;
        querySEQUENCIA.prepare("SELECT seq FROM SQLITE_SEQUENCE WHERE name=?");
        querySEQUENCIA.bindValue(0,"lixoes");
        querySEQUENCIA.exec();

        while (querySEQUENCIA.next())
        {
            int _seq = querySEQUENCIA.value(0).toInt();
            _ID = _seq; //cout << "sequencia atual: " << _ID << " - próxima sequencia: " << _ID+1 << "\n";
        }

        con.close();

        QMessageBox msgBox;
        msgBox.setText("Arquivo inserido com sucesso");
        msgBox.exec();
    }
}

void lixoes_cad::editar()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    // http://www.qtcentre.org/threads/64448-placeholder-problem
    QSqlQuery query;
    query.prepare("UPDATE lixoes SET "
                  "nome=:nomeLixao, "
                  "endereco=:enderecoLixao, "
                  "bairro=:bairroLixao, "
                  "cep=:cepLixao, "
                  "id_municipio=:idMunicipioLixao, "
                  "latitude=:latitudeLixao, "
                  "longitude=:longitudeLixao, "
                  "folha_topografica=:folhaTopograficaLixao, "
                  "coordenadas_utm=:coordenadasUtmLixao, "
                  "datum_mc=:datumMcLixao, "
                  "bacia_hidrografica=:baciaHidrograficaLixao, "
                  "data_visita=:dataVisitaLixao, "
                  "inicio_atividades=:inicioAtividadesLixao, "
                  "fim_atividades=:fimAtividadesLixao, "
                  "enquadramento_plano_diretor=:enquadramentoPlanoDiretorLixao, "
                  "uso_futuro=:usoFuturoLixao, "
                  "proprietario_terreno=:proprietarioTerrenoLixao, "
                  "informantes=:informantesLixao, "
                  "acompanhantes=:acompanhantesLixao, "
                  "condicao_tempo=:condicaoTempoLixao, "
                  "condicao_vento=:condicaoVentoLixao, "
                  "acumulado_chuva=:acumuladoChuvaLixao, "
                  "lei_9985=:lei9985Lixao, "
                  "area_urbana_rural=:areaUrbanaRuralLixao, "
                  "ocorrencia_talvegue=:ocorrenciaTalvegueLixao, "
                  "ocorrencia_varzea=:ocorrenciaVarzeaLixao, "
                  "ocorrencia_corpos_hidricos=:ocorrenciaCorposHidricosLixao, "
                  "area_industrial=:areaIndustrialLixao, "
                  "g1_area=:1AreaLixao, "
                  "g1_atividade=:1AtividadeLixao, "
                  "g1_adensamento=:1AdensamentoLixao, "
                  "g1_deslizamento=:1DeslizamentoLixao, "
                  "g1_erosao=:1ErosaoLixao, "
                  "g1_outros=:1OutrosLixao, "
                  "g1_outros_detalhes=:1OutrosDetalhesLixao, "
                  "g1_espessura=:1EspessuraLixao, "
                  "g1_residuos_classe_IIB=:1ResiduosClasseIIBLixao, "
                  "g1_residuos_classe_IIA=:1ResiduosClasseIIALixao, "
                  "g1_residuos_classe_I=:1ResiduosClasseILixao, "
                  "g1_residuos_classe_I_detalhes=:1ResiduosClasseIDetalhesLixao, "
                  "g1_impermeabilizacao_superior=:1ImpermeabilizacaoSuperiorLixao, "
                  "g1_pluviometria=:1PluviometriaLixao, "
                  "g1_declividade=:1DeclividadeLixao, "
                  "g2_impermeabilizacao_inferior=:2ImpermeabilizacaoInferiorLixao, "
                  "g2_presenca_lixiviado=:2PresencaLixiviadoLixao, "
                  "g2_coleta_lixiviado=:2ColetaLixiviadoLixao, "
                  "g2_tratamento_lixiviado=:2TratamentoLixiviadoLixao, "
                  "g2_natureza_solo=:2NaturezaSoloLixao, "
                  "g2_permeabilidade_solo=:2PermeabilidadeSoloLixao, "
                  "g2_nivel_piezometrico=:2NivelPiezometricoLixao, "
                  "g2_distancia_residuos_agua=:2DistanciaResiduosAguaLixao, "
                  "g2_descontinuidade_terreno=:2DescontinuidadeTerrenoLixao, "
                  "g2_contaminacao_solo=:2ContaminacaoSoloLixao, "
                  "g2_contaminacao_agua=:2ContaminacaoAguaLixao, "
                  "g2_distancia_abast_domestico=:2DistanciaAbastDomesticoLixao, "
                  "g2_distancia_abast_publico=:2DistanciaAbastPublicoLixao, "
                  "g2_uso_preponderante_agua=:2UsoPreponderanteAguaLixao, "
                  "g3_distancia_abast_domestico=:3DistanciaAbastDomesticoLixao, "
                  "g3_distancia_abast_publico=:3DistanciaAbastPublicoLixao, "
                  "g3_classe_aguas=:3ClasseAguasLixao, "
                  "g3_uso_preponderante_agua=:3UsoPreponderanteAguaLixao, "
                  "g3_distancia_zona_balneavel=:3DistanciaZonaBalneavelLixao, "
                  "g3_distancia_corpo_hidrico=:3DistanciaCorpoHidricoLixao, "
                  "g3_distancia_nascente=:3DistanciaNascenteLixao, "
                  "g3_poluicao_agua=:3PoluicaoAguaLixao, "
                  "g4_densidade_populacional=:4DensidadePopulacionalLixao, "
                  "g4_hospital_creche_escola_asilo=:4HospitalCrecheEscolaAsiloLixao, "
                  "g4_distancia_populacao=:4DistanciaPopulacaoLixao, "
                  "g4_atividades_agropecuarias=:4AtividadesAgropecuariasLixao, "
                  "g4_atividades_lazer=:4AtividadesLazerLixao, "
                  "g4_isolamento_fisico_lixao=:4IsolamentoFisicoLixao, "
                  "g4_insetos=:4InsetosLixao, "
                  "g4_roedores=:4RoedoresLixao, "
                  "g4_escorpioes=:4EscorpioesLixao, "
                  "g4_urubus=:4UrubusLixao, "
                  "g4_outras_aves=:4OutrasAvesLixao, "
                  "g4_outras_aves_detalhes=:4OutrasAvesDetalhesLixao, "
                  "g4_outros_animais=:4OutrosAnimaisLixao, "
                  "g4_outros_animais_detalhes=:4OutrosAnimaisDetalhesLixao, "
                  "g4_danos_saude_populacao=:4DanosSaudePopulacaoLixao, "
                  "g4_danos_materiais_populacao=:4DanosMateriaisPopulacaoLixao, "
                  "g4_catadores=:4CatadoresLixao, "
                  "g5_largura_barreira_vegetal=:5LarguraBarreiraVegetalLixao, "
                  "g5_distancia_elemento=:5DistanciaElementoLixao, "
                  "g5_distancia_elemento_detalhes=:5DistanciaElementoDetalhesLixao, "
                  "g5_desmatamento=:5DesmatamentoLixao, "
                  "g5_dispersao_residuos=:5DispersaoResiduosLixao, "
                  "g5_contaminacao_mangue_pantano=:5ContaminacaoManguePantanoLixao, "
                  "g5_danos_animais=:5DanosAnimaisLixao, "
                  "g6_odores=:6OdoresLixao, "
                  "g6_explosoes=:6ExplosoesLixao, "
                  "g6_queima_residuos=:6QueimaResiduosLixao, "
                  "g6_bolsoes_migracao_biogas=:6BolsoesMigracaoBiogasLixao, "
                  "g6_coleta_gas=:6ColetaGasLixao, "
                  "g6_tratamento_gas=:6TratamentoGasLixao "
                  "WHERE id=:idLixao");

    // 0. Caracterização da área [início]
    query.bindValue(":nomeLixao",ui->leNome->text());
    query.bindValue(":enderecoLixao",ui->leEndereco->text());
    query.bindValue(":bairroLixao",ui->leBairro->text());
    query.bindValue(":cepLixao",ui->leCEP->text());

    QSqlQuery query2;
    query2.prepare("SELECT * FROM municipios where municipio = :municipioLixao ");
    query2.bindValue(":municipioLixao",ui->cbMunicipios->currentText());
    query2.exec();
    while (query2.next())
    {
        query.bindValue(":idMunicipioLixao",query2.value(0).toInt());
    }

    query.bindValue(":latitudeLixao",ui->leLatitude->text());
    query.bindValue(":longitudeLixao",ui->leLongitude->text());
    query.bindValue(":folhaTopograficaLixao",ui->leFolhaTopografica->text());
    query.bindValue(":coordenadasUtmLixao",ui->leUTM->text());
    query.bindValue(":datumMcLixao",ui->leDatum->text());
    query.bindValue(":baciaHidrograficaLixao",ui->leBaciaHidrografica->text());
    query.bindValue(":dataVisitaLixao",ui->leDataVisita->text());
    query.bindValue(":inicioAtividadesLixao",ui->leDataInicioAtividades->text());
    query.bindValue(":fimAtividadesLixao",ui->leDataFimAtividades->text());
    query.bindValue(":enquadramentoPlanoDiretorLixao",ui->leEnquadramentoPlanoDiretor->text());
    query.bindValue(":usoFuturoLixao",ui->leUsoFuturo->text());
    query.bindValue(":proprietarioTerrenoLixao",ui->leProprietario->text());
    query.bindValue(":informantesLixao",ui->leInformantes->text());
    query.bindValue(":acompanhantesLixao",ui->leAcompanhantes->text());

    if (ui->rbCondicaoTempo_0->isChecked())
        query.bindValue(":condicaoTempoLixao", 0);
    else if (ui->rbCondicaoTempo_1->isChecked())
        query.bindValue(":condicaoTempoLixao", 1);
    else if (ui->rbCondicaoTempo_2->isChecked())
        query.bindValue(":condicaoTempoLixao", 2);
    else if (ui->rbCondicaoTempo_3->isChecked())
        query.bindValue(":condicaoTempoLixao", 3);
    else if (ui->rbCondicaoTempo_4->isChecked())
        query.bindValue(":condicaoTempoLixao", 4);

    query.bindValue(":condicaoVentoLixao",ui->leCondicaoVento->text());
    query.bindValue(":acumuladoChuvaLixao",ui->leAcumuladoChuva->text());

    if (ui->rbLei9985_0->isChecked())
        query.bindValue(":lei9985Lixao", 0);
    else if (ui->rbLei9985_1->isChecked())
        query.bindValue(":lei9985Lixao", 1);

    if (ui->rbAreaUrbanaRural_0->isChecked())
        query.bindValue(":areaUrbanaRuralLixao", 0);
    else if (ui->rbAreaUrbanaRural_1->isChecked())
        query.bindValue(":areaUrbanaRuralLixao", 1);

    if (ui->rbOcorrenciaTalvegue_0->isChecked())
        query.bindValue(":ocorrenciaTalvegueLixao", 0);
    else if (ui->rbOcorrenciaTalvegue_1->isChecked())
        query.bindValue(":ocorrenciaTalvegueLixao", 1);

    if (ui->rbOcorrenciaVarzea_0->isChecked())
        query.bindValue(":ocorrenciaVarzeaLixao", 0);
    else if (ui->rbOcorrenciaVarzea_1->isChecked())
        query.bindValue(":ocorrenciaVarzeaLixao", 1);

    query.bindValue(":ocorrenciaCorposHidricosLixao", ui->sbQuantidadeCorposHidricos->value());

    if (ui->rbAreaIndustrial_0->isChecked())
        query.bindValue(":areaIndustrialLixao", 0);
    if (ui->rbAreaIndustrial_1->isChecked())
        query.bindValue(":areaIndustrialLixao", 1);
    // 0. Caracterização da área [fim]

    // 1. Caracterização do lixão [inicio]
    if (ui->rbAreaLixao_0->isChecked())
        query.bindValue(":1AreaLixao", 0);
    else if (ui->rbAreaLixao_1->isChecked())
        query.bindValue(":1AreaLixao", 1);
    else if (ui->rbAreaLixao_2->isChecked())
        query.bindValue(":1AreaLixao", 2);
    else if (ui->rbAreaLixao_3->isChecked())
        query.bindValue(":1AreaLixao", 3);

    if (ui->rbAtividadeLixao_0->isChecked())
        query.bindValue(":1AtividadeLixao", 0);
    else if (ui->rbAtividadeLixao_1->isChecked())
        query.bindValue(":1AtividadeLixao", 1);
    else if (ui->rbAtividadeLixao_2->isChecked())
        query.bindValue(":1AtividadeLixao", 2);
    else if (ui->rbAtividadeLixao_3->isChecked())
        query.bindValue(":1AtividadeLixao", 3);

    if (ui->rbAdensamento_0->isChecked())
        query.bindValue(":1AdensamentoLixao", 0);
    else if (ui->rbAdensamento_1->isChecked())
        query.bindValue(":1AdensamentoLixao", 1);

    if (ui->rbDeslizamento_0->isChecked())
        query.bindValue(":1DeslizamentoLixao", 0);
    else if (ui->rbDeslizamento_1->isChecked())
        query.bindValue(":1DeslizamentoLixao", 1);

    if (ui->rbErosao_0->isChecked())
        query.bindValue(":1ErosaoLixao", 0);
    else if (ui->rbErosao_1->isChecked())
        query.bindValue(":1ErosaoLixao", 1);

    if (ui->rbOutrosAcidentes_0->isChecked())
        query.bindValue(":1OutrosLixao", 0);
    else if (ui->rbOutrosAcidentes_1->isChecked())
    {
        query.bindValue(":1OutrosLixao", 1);
        query.bindValue(":1OutrosDetalhesLixao", ui->leOutrosAcidentesDetalhes->text());
    }

    if (ui->rbEspessuraCamada_0->isChecked())
        query.bindValue(":1EspessuraLixao", 0);
    else if (ui->rbEspessuraCamada_1->isChecked())
        query.bindValue(":1EspessuraLixao", 1);
    else if (ui->rbEspessuraCamada_2->isChecked())
        query.bindValue(":1EspessuraLixao", 2);
    else if (ui->rbEspessuraCamada_3->isChecked())
        query.bindValue(":1EspessuraLixao", 3);

    if (ui->cbNaturezaResiduos_0->isChecked())
        query.bindValue(":1ResiduosClasseIIBLixao",1);
    else
        query.bindValue(":1ResiduosClasseIIBLixao",0);

    if (ui->cbNaturezaResiduos_1->isChecked())
        query.bindValue(":1ResiduosClasseIIALixao",1);
    else
        query.bindValue(":1ResiduosClasseIIALixao",0);

    if (ui->cbNaturezaResiduos_2->isChecked())
    {
        query.bindValue(":1ResiduosClasseILixao",1);
        query.bindValue(":1ResiduosClasseIDetalhesLixao",ui->leNaturezaResiduosDetalhes->text());
    }
    else
        query.bindValue(":1ResiduosClasseILixao",0);

    if (ui->rbImpermeabilizacaoSuperior_0->isChecked())
        query.bindValue(":1ImpermeabilizacaoSuperiorLixao", 0);
    else if (ui->rbImpermeabilizacaoSuperior_1->isChecked())
        query.bindValue(":1ImpermeabilizacaoSuperiorLixao", 1);
    else if (ui->rbImpermeabilizacaoSuperior_2->isChecked())
        query.bindValue(":1ImpermeabilizacaoSuperiorLixao", 2);

    if (ui->rbPluviometria_0->isChecked())
        query.bindValue(":1PluviometriaLixao", 0);
    else if (ui->rbPluviometria_1->isChecked())
        query.bindValue(":1PluviometriaLixao", 1);
    else if (ui->rbPluviometria_2->isChecked())
        query.bindValue(":1PluviometriaLixao", 2);
    else if (ui->rbPluviometria_3->isChecked())
        query.bindValue(":1PluviometriaLixao", 3);

    if (ui->rbDeclividadeTerreno_0->isChecked())
        query.bindValue(":1DeclividadeLixao", 0);
    else if (ui->rbDeclividadeTerreno_1->isChecked())
        query.bindValue(":1DeclividadeLixao", 1);
    else if (ui->rbDeclividadeTerreno_2->isChecked())
        query.bindValue(":1DeclividadeLixao", 2);
    else if (ui->rbDeclividadeTerreno_3->isChecked())
        query.bindValue(":1DeclividadeLixao", 3);
    else if (ui->rbDeclividadeTerreno_4->isChecked())
        query.bindValue(":1DeclividadeLixao", 4);
    else if (ui->rbDeclividadeTerreno_5->isChecked())
        query.bindValue(":1DeclividadeLixao", 5);
    // 1. Caracterização do lixão [fim]

    // 2. Solo e águas subterrâneas [inicio]
    if (ui->rbImpermeabilizacaoInferior_0->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 0);
    else if (ui->rbImpermeabilizacaoInferior_1->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 1);
    else if (ui->rbImpermeabilizacaoInferior_2->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 2);
    else if (ui->rbImpermeabilizacaoInferior_3->isChecked())
        query.bindValue(":2ImpermeabilizacaoInferiorLixao", 3);

    if (ui->rbPresencaLixiviado_0->isChecked())
        query.bindValue(":2PresencaLixiviadoLixao", 0);
    else if (ui->rbPresencaLixiviado_1->isChecked())
        query.bindValue(":2PresencaLixiviadoLixao", 1);
    else if (ui->rbPresencaLixiviado_2->isChecked())
        query.bindValue(":2PresencaLixiviadoLixao", 2);

    if (ui->rbColetaLixiviado_0->isChecked())
        query.bindValue(":2ColetaLixiviadoLixao", 0);
    else if (ui->rbColetaLixiviado_1->isChecked())
        query.bindValue(":2ColetaLixiviadoLixao", 1);
    else if (ui->rbColetaLixiviado_2->isChecked())
        query.bindValue(":2ColetaLixiviadoLixao", 2);

    if (ui->rbTratamentoLixiviado_0->isChecked())
        query.bindValue(":2TratamentoLixiviadoLixao", 0);
    else if (ui->rbTratamentoLixiviado_1->isChecked())
        query.bindValue(":2TratamentoLixiviadoLixao", 1);
    else if (ui->rbTratamentoLixiviado_2->isChecked())
        query.bindValue(":2TratamentoLixiviadoLixao", 2);

    if (ui->rbNaturezaSolo_0->isChecked())
        query.bindValue(":2NaturezaSoloLixao", 0);
    else if (ui->rbNaturezaSolo_1->isChecked())
        query.bindValue(":2NaturezaSoloLixao", 1);
    else if (ui->rbNaturezaSolo_2->isChecked())
        query.bindValue(":2NaturezaSoloLixao", 2);

    if (ui->rbPermeabilidadeSolo_0->isChecked())
        query.bindValue(":2PermeabilidadeSoloLixao", 0);
    else if (ui->rbPermeabilidadeSolo_1->isChecked())
        query.bindValue(":2PermeabilidadeSoloLixao", 1);
    else if (ui->rbPermeabilidadeSolo_2->isChecked())
        query.bindValue(":2PermeabilidadeSoloLixao", 2);

    if (ui->rbNivelPiezometrico_0->isChecked())
    {
        query.bindValue(":2NivelPiezometricoLixao", 0);
        query.bindValue(":2DistanciaResiduosAguaLixao", -1);
    }
    else if (ui->rbNivelPiezometrico_1->isChecked())
    {
        query.bindValue(":2NivelPiezometricoLixao", 1);
        query.bindValue(":2DistanciaResiduosAguaLixao", -1);
    }
    else if (ui->rbNivelPiezometrico_2->isChecked())
    {
        query.bindValue(":2NivelPiezometricoLixao", 2);

        if (ui->rbDistanciaResiduosAgua_0->isChecked())
            query.bindValue(":2DistanciaResiduosAguaLixao", 0);
        else if (ui->rbDistanciaResiduosAgua_1->isChecked())
            query.bindValue(":2DistanciaResiduosAguaLixao", 1);
        else if (ui->rbDistanciaResiduosAgua_2->isChecked())
            query.bindValue(":2DistanciaResiduosAguaLixao", 2);
    }

    if (ui->rbDescontinuidadeTerreno_0->isChecked())
        query.bindValue(":2DescontinuidadeTerrenoLixao", 0);
    else if (ui->rbDescontinuidadeTerreno_1->isChecked())
        query.bindValue(":2DescontinuidadeTerrenoLixao", 1);
    else if (ui->rbDescontinuidadeTerreno_2->isChecked())
        query.bindValue(":2DescontinuidadeTerrenoLixao", 2);

    if (ui->rbContaminacaoSolo_0->isChecked())
        query.bindValue(":2ContaminacaoSoloLixao", 0);
    else if (ui->rbContaminacaoSolo_1->isChecked())
        query.bindValue(":2ContaminacaoSoloLixao", 1);
    else if (ui->rbContaminacaoSolo_2->isChecked())
        query.bindValue(":2ContaminacaoSoloLixao", 2);

    if (ui->rbContaminacaoAgua_0->isChecked())
        query.bindValue(":2ContaminacaoAguaLixao", 0);
    else if (ui->rbContaminacaoAgua_1->isChecked())
        query.bindValue(":2ContaminacaoAguaLixao", 1);
    else if (ui->rbContaminacaoAgua_2->isChecked())
        query.bindValue(":2ContaminacaoAguaLixao", 2);

    if (ui->rbDistanciaAbastDomesticoSub_0->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 0);
    else if (ui->rbDistanciaAbastDomesticoSub_1->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 1);
    else if (ui->rbDistanciaAbastDomesticoSub_2->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 2);
    else if (ui->rbDistanciaAbastDomesticoSub_3->isChecked())
        query.bindValue(":2DistanciaAbastDomesticoLixao", 3);

    if (ui->rbDistanciaAbastPublicoSub_0->isChecked())
        query.bindValue(":2DistanciaAbastPublicoLixao", 0);
    else if (ui->rbDistanciaAbastPublicoSub_1->isChecked())
        query.bindValue(":2DistanciaAbastPublicoLixao", 1);
    else if (ui->rbDistanciaAbastPublicoSub_2->isChecked())
        query.bindValue(":2DistanciaAbastPublicoLixao", 2);

    if (ui->rbUsoPreponderanteAguaSub_0->isChecked())
        query.bindValue(":2UsoPreponderanteAguaLixao", 0);
    else if (ui->rbUsoPreponderanteAguaSub_1->isChecked())
        query.bindValue(":2UsoPreponderanteAguaLixao", 1);
    else if (ui->rbUsoPreponderanteAguaSub_2->isChecked())
        query.bindValue(":2UsoPreponderanteAguaLixao", 2);
    // 2. Solo e águas subterrâneas [fim]


    // 3. Águas superficiais [início]
    if (ui->rbDistanciaAbastDomesticoSup_0->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 0);
    else if (ui->rbDistanciaAbastDomesticoSup_1->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 1);
    else if (ui->rbDistanciaAbastDomesticoSup_2->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 2);
    else if (ui->rbDistanciaAbastDomesticoSup_3->isChecked())
        query.bindValue(":3DistanciaAbastDomesticoLixao", 3);

    if (ui->rbDistanciaAbastPublicoSup_0->isChecked())
        query.bindValue(":3DistanciaAbastPublicoLixao", 0);
    else if (ui->rbDistanciaAbastPublicoSup_1->isChecked())
        query.bindValue(":3DistanciaAbastPublicoLixao", 1);
    else if (ui->rbDistanciaAbastPublicoSup_2->isChecked())
        query.bindValue(":3DistanciaAbastPublicoLixao", 2);

    if (ui->rbClasseAgua_0->isChecked())
        query.bindValue(":3ClasseAguasLixao", 0);
    else if (ui->rbClasseAgua_1->isChecked())
        query.bindValue(":3ClasseAguasLixao", 1);
    else if (ui->rbClasseAgua_2->isChecked())
        query.bindValue(":3ClasseAguasLixao", 2);
    else if (ui->rbClasseAgua_3->isChecked())
        query.bindValue(":3ClasseAguasLixao", 3);
    else if (ui->rbClasseAgua_4->isChecked())
        query.bindValue(":3ClasseAguasLixao", 4);

    if (ui->rbUsoPreponderanteAguaSup_0->isChecked())
        query.bindValue(":3UsoPreponderanteAguaLixao", 0);
    else if (ui->rbUsoPreponderanteAguaSup_1->isChecked())
        query.bindValue(":3UsoPreponderanteAguaLixao", 1);
    else if (ui->rbUsoPreponderanteAguaSup_2->isChecked())
        query.bindValue(":3UsoPreponderanteAguaLixao", 2);

    if (ui->rbDistanciaZonaBalneavel_0->isChecked())
        query.bindValue(":3DistanciaZonaBalneavelLixao", 0);
    else if (ui->rbDistanciaZonaBalneavel_1->isChecked())
        query.bindValue(":3DistanciaZonaBalneavelLixao", 1);
    else if (ui->rbDistanciaZonaBalneavel_2->isChecked())
        query.bindValue(":3DistanciaZonaBalneavelLixao", 2);

    if (ui->rbDistanciaCorpoHidrico_0->isChecked())
        query.bindValue(":3DistanciaCorpoHidricoLixao", 0);
    else if (ui->rbDistanciaCorpoHidrico_1->isChecked())
        query.bindValue(":3DistanciaCorpoHidricoLixao", 1);
    else if (ui->rbDistanciaCorpoHidrico_2->isChecked())
        query.bindValue(":3DistanciaCorpoHidricoLixao", 2);

    if (ui->rbDistanciaNascente_0->isChecked())
        query.bindValue(":3DistanciaNascenteLixao", 0);
    else if (ui->rbDistanciaNascente_1->isChecked())
        query.bindValue(":3DistanciaNascenteLixao", 1);
    else if (ui->rbDistanciaNascente_2->isChecked())
        query.bindValue(":3DistanciaNascenteLixao", 2);

    if (ui->rbPoluicaoAgua_0->isChecked())
        query.bindValue(":3PoluicaoAguaLixao", 0);
    else if (ui->rbPoluicaoAgua_1->isChecked())
        query.bindValue(":3PoluicaoAguaLixao", 1);
    // 3. Águas superficiais [fim]

    // 4. Social [início]
    if (ui->rbDensidadePopulacional_0->isChecked())
        query.bindValue(":4DensidadePopulacionalLixao", 0);
    else if (ui->rbDensidadePopulacional_1->isChecked())
        query.bindValue(":4DensidadePopulacionalLixao", 1);
    else if (ui->rbDensidadePopulacional_2->isChecked())
        query.bindValue(":4DensidadePopulacionalLixao", 2);

    if (ui->rbHospitalCrecheEscolaAsilo_0->isChecked())
        query.bindValue(":4HospitalCrecheEscolaAsiloLixao", 0);
    else if (ui->rbHospitalCrecheEscolaAsilo_1->isChecked())
        query.bindValue(":4HospitalCrecheEscolaAsiloLixao", 1);

    if (ui->rbDistanciaPopulacao_0->isChecked())
        query.bindValue(":4DistanciaPopulacaoLixao", 0);
    else if (ui->rbDistanciaPopulacao_1->isChecked())
        query.bindValue(":4DistanciaPopulacaoLixao", 1);
    else if (ui->rbDistanciaPopulacao_2->isChecked())
        query.bindValue(":4DistanciaPopulacaoLixao", 2);

    if (ui->rbAtividadesAgropecuarias_0->isChecked())
        query.bindValue(":4AtividadesAgropecuariasLixao", 0);
    else if (ui->rbAtividadesAgropecuarias_1->isChecked())
        query.bindValue(":4AtividadesAgropecuariasLixao", 1);
    else if (ui->rbAtividadesAgropecuarias_2->isChecked())
        query.bindValue(":4AtividadesAgropecuariasLixao", 2);

    if (ui->rbAtividadesLazer_0->isChecked())
        query.bindValue(":4AtividadesLazerLixao", 0);
    else if (ui->rbAtividadesLazer_1->isChecked())
        query.bindValue(":4AtividadesLazerLixao", 1);
    else if (ui->rbAtividadesLazer_2->isChecked())
        query.bindValue(":4AtividadesLazerLixao", 2);

    if (ui->rbIsolamentoFisicoLixao_0->isChecked())
        query.bindValue(":4IsolamentoFisicoLixao", 0);
    else if (ui->rbIsolamentoFisicoLixao_1->isChecked())
        query.bindValue(":4IsolamentoFisicoLixao", 1);
    else if (ui->rbIsolamentoFisicoLixao_2->isChecked())
        query.bindValue(":4IsolamentoFisicoLixao", 2);

    if (ui->cbInsetos->isChecked())
        query.bindValue(":4InsetosLixao", 1);
    else
        query.bindValue(":4InsetosLixao", 0);

    if (ui->cbRoedores->isChecked())
        query.bindValue(":4RoedoresLixao", 1);
    else
        query.bindValue(":4RoedoresLixao", 0);

    if (ui->cbEscorpioes->isChecked())
        query.bindValue(":4EscorpioesLixao", 1);
    else
        query.bindValue(":4EscorpioesLixao", 0);

    if (ui->cbUrubus->isChecked())
        query.bindValue(":4UrubusLixao", 1);
    else
        query.bindValue(":4UrubusLixao", 0);

    if (ui->cbOutrasAves->isChecked())
    {
        query.bindValue(":4OutrasAvesLixao", 1);
        query.bindValue(":4OutrasAvesDetalhesLixao", ui->leOutrasAves->text());
    }
    else
        query.bindValue(":4OutrasAvesLixao", 0);

    if (ui->cbOutrosAnimais->isChecked())
    {
        query.bindValue(":4OutrosAnimaisLixao", 1);
        query.bindValue(":4OutrosAnimaisDetalhesLixao", ui->leOutrosAnimais->text());
    }
    else
        query.bindValue(":4OutrosAnimaisLixao", 0);

    if (ui->rbDanoSaudePopulacao_0->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 0);
    else if (ui->rbDanoSaudePopulacao_1->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 1);
    else if (ui->rbDanoSaudePopulacao_2->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 2);
    else if (ui->rbDanoSaudePopulacao_3->isChecked())
        query.bindValue(":4DanosSaudePopulacaoLixao", 3);

    if (ui->rbDanoMaterialPopulacao_0->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 0);
    else if (ui->rbDanoMaterialPopulacao_1->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 1);
    else if (ui->rbDanoMaterialPopulacao_2->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 2);
    else if (ui->rbDanoMaterialPopulacao_3->isChecked())
        query.bindValue(":4DanosMateriaisPopulacaoLixao", 3);

    if (ui->rbCatadores_0->isChecked())
        query.bindValue(":4CatadoresLixao", 0);
    else if (ui->rbCatadores_1->isChecked())
        query.bindValue(":4CatadoresLixao", 1);
    // 4. Social [fim]

    // 5. Meio natural e paisagens [início]
    if (ui->rbLarguraBarreiraVegetal_0->isChecked())
        query.bindValue(":5LarguraBarreiraVegetalLixao", 0);
    else if (ui->rbLarguraBarreiraVegetal_1->isChecked())
        query.bindValue(":5LarguraBarreiraVegetalLixao", 1);
    else if (ui->rbLarguraBarreiraVegetal_2->isChecked())
        query.bindValue(":5LarguraBarreiraVegetalLixao", 2);

    if (ui->rbDistanciaElemento_0->isChecked())
        query.bindValue(":5DistanciaElementoLixao", 0);
    else if (ui->rbDistanciaElemento_1->isChecked())
        query.bindValue(":5DistanciaElementoLixao", 1);
    else if (ui->rbDistanciaElemento_2->isChecked())
        query.bindValue(":5DistanciaElementoLixao", 2);
    else if (ui->rbDistanciaElemento_3->isChecked())
    {
        query.bindValue(":5DistanciaElementoLixao", 3);
        query.bindValue(":5DistanciaElementoDetalhesLixao", ui->leDistanciaElementoDetalhes->text());
    }

    if (ui->rbDesmatamento_0->isChecked())
        query.bindValue(":5DesmatamentoLixao", 0);
    else if (ui->rbDesmatamento_1->isChecked())
        query.bindValue(":5DesmatamentoLixao", 1);
    else if (ui->rbDesmatamento_2->isChecked())
        query.bindValue(":5DesmatamentoLixao", 2);

    if (ui->rbDispersaoResiduos_0->isChecked())
        query.bindValue(":5DispersaoResiduosLixao", 0);
    else if (ui->rbDispersaoResiduos_1->isChecked())
        query.bindValue(":5DispersaoResiduosLixao", 1);

    if (ui->rbContaminacaoManguePantano_0->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 0);
    else if (ui->rbContaminacaoManguePantano_1->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 1);
    else if (ui->rbContaminacaoManguePantano_2->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 2);
    else if (ui->rbContaminacaoManguePantano_3->isChecked())
        query.bindValue(":5ContaminacaoManguePantanoLixao", 3);

    if (ui->rbDanosAnimais_0->isChecked())
        query.bindValue(":5DanosAnimaisLixao", 0);
    else if (ui->rbDanosAnimais_1->isChecked())
        query.bindValue(":5DanosAnimaisLixao", 1);
    else if (ui->rbDanosAnimais_2->isChecked())
        query.bindValue(":5DanosAnimaisLixao", 2);
    // 5. Meio natural e paisagens [fim]

    // 6. Ar [início]
    if (ui->rbOdoresAr_0->isChecked())
        query.bindValue(":6OdoresLixao", 0);
    else if (ui->rbOdoresAr_1->isChecked())
        query.bindValue(":6OdoresLixao", 1);
    else if (ui->rbOdoresAr_2->isChecked())
        query.bindValue(":6OdoresLixao", 2);

    if (ui->rbExplosoes_0->isChecked())
        query.bindValue(":6ExplosoesLixao", 0);
    else if (ui->rbExplosoes_1->isChecked())
        query.bindValue(":6ExplosoesLixao", 1);
    else if (ui->rbExplosoes_2->isChecked())
        query.bindValue(":6ExplosoesLixao", 2);
    else if (ui->rbExplosoes_3->isChecked())
        query.bindValue(":6ExplosoesLixao", 3);

    if (ui->rbQueimaResiduos_0->isChecked())
        query.bindValue(":6QueimaResiduosLixao", 0);
    else if (ui->rbQueimaResiduos_1->isChecked())
        query.bindValue(":6QueimaResiduosLixao", 1);
    else if (ui->rbQueimaResiduos_2->isChecked())
        query.bindValue(":6QueimaResiduosLixao", 2);

    if (ui->rbBolsoesMigracaoBiogas_0->isChecked())
        query.bindValue(":6BolsoesMigracaoBiogasLixao", 0);
    else if (ui->rbBolsoesMigracaoBiogas_1->isChecked())
        query.bindValue(":6BolsoesMigracaoBiogasLixao", 1);
    else if (ui->rbBolsoesMigracaoBiogas_2->isChecked())
        query.bindValue(":6BolsoesMigracaoBiogasLixao", 2);

    if (ui->rbColetaGas_0->isChecked())
        query.bindValue(":6ColetaGasLixao", 0);
    else if (ui->rbColetaGas_1->isChecked())
        query.bindValue(":6ColetaGasLixao", 1);
    else if (ui->rbColetaGas_2->isChecked())
        query.bindValue(":6ColetaGasLixao", 2);
    else if (ui->rbColetaGas_3->isChecked())
        query.bindValue(":6ColetaGasLixao", 3);

    if (ui->rbTratamentoGas_0->isChecked())
        query.bindValue(":6TratamentoGasLixao", 0);
    else if (ui->rbTratamentoGas_1->isChecked())
        query.bindValue(":6TratamentoGasLixao", 1);
    else if (ui->rbTratamentoGas_2->isChecked())
        query.bindValue(":6TratamentoGasLixao", 2);
    // 6. Ar [fim]

    query.bindValue(":idLixao",_ID);
    query.exec();

    con.close();

    QMessageBox msgBox;
    msgBox.setText("Arquivo atualizado com sucesso");
    msgBox.exec();
}

void lixoes_cad::verificar_diagnostico()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    /*
     * Optou-se por criar duas querys, uma para tratar o preenchimento dos campos da tabela
     * e outra para tratar o diagnóstico, em função de cada uma das situações poder dar um break
     * no laço, assim, uma situação não influencia no laço da outra
     */

    QSqlQuery query;
    query.prepare("SELECT * FROM lixoes WHERE id=?");
    query.bindValue(0,_ID);
    query.exec();

    while (query.next())
    {
        // verifica se o preenchimento dos campos da tabela. 0=Não, 1=Sim
        for (int i=1;i<=95;i++)
        {
            // campos que não serão considerados pois não são obrigatórios e não pontuam
            if ((i == 35) ||      // g1_outros_detalhes
                    (i == 40) ||  // g1_residuos_classe_I_detalhes                    
                    (i == 77) ||  // g4_outras_aves_detalhes
                    (i == 79) ||  // g4_outros_animais_detalhes
                    (i == 85))    // g5_distancia_elemento_detalhes
                continue;
            else if ((query.value(i).toString() == NULL) || (query.value(i).toString() == ""))
            {
                _Preenchimento=0; // campo preenchido
                break;
            }
            else
                _Preenchimento=1; // campo não preenchido
        }
    }

    QSqlQuery query1;
    query1.prepare("SELECT * FROM lixoes WHERE id=?");
    query1.bindValue(0,_ID);
    query1.exec();

    while (query1.next())
    {
        // verifica se o diagnóstico ja foi feito em determinado lixão. 0=Não, 1=Sim
        for (int j=96;j<=97;j++) // recupera apenas os campos pontuacao e nivel_impacto
            if ((query1.value(j).toString() == NULL) || (query1.value(j).toString() == ""))
            {
                _Diagnostico=0; // Diagnóstico não foi feito
                break;
            }
            else
                _Diagnostico=1; // Diagnóstico foi feito
    }

    if (_Preenchimento==0 && _Diagnostico==0) // se o preenchimento da tabela não foi feito corretamente, e o lixão não foi diagnosticado
    {
        ui->pbDiagnosticar->setEnabled(false);
        ui->lbDiagnostico->setStyleSheet("QLabel { background-color : #FF2800; }");
        ui->lbDiagnostico->setText("<html><head/><body><p><span style='font-size:12pt;'>Verifique se todos os campos foram preenchidos corretamente</span></p></body></html>");
    }
    else if (_Preenchimento==0 && _Diagnostico==1) // se o preenchimento da tabela não foi feito corretamente, mas o lixão foi diagnosticado
    {
        ui->pbDiagnosticar->setEnabled(false);
        ui->lbDiagnostico->setStyleSheet("QLabel { background-color : #FF2800; }");
        ui->lbDiagnostico->setText("<html><head/><body><p><span style='font-size:12pt;'>Verifique se todos os campos foram preenchidos corretamente</span></p></body></html>");

        QSqlQuery query2;
        query2.prepare("SELECT diagnostico FROM lixoes WHERE id=?");
        query2.bindValue(0,_ID);
        query2.exec();
        while (query2.next())
        {
            ui->teDiagnostico->insertHtml(query2.value(0).toString());
        }

    }
    else if (_Preenchimento==1 && _Diagnostico==0) // se o preenchimento da tabela foi feito corretamente, mas o lixão não foi diagnosticado
    {
        ui->pbDiagnosticar->setEnabled(true);
        ui->lbDiagnostico->setStyleSheet("QLabel { background-color : #FEFB18; }");
        ui->lbDiagnostico->setText("<html><head/><body><p><span style='font-size:12pt;'>Lixão não diagnosticado</span></p></body></html>");

    }
    else if (_Preenchimento==1 && _Diagnostico==1) // se o preenchimento da tabela foi feito corretamente e o lixão foi diagnosticado
    {
        ui->pbDiagnosticar->setEnabled(true);
        ui->lbDiagnostico->setStyleSheet("QLabel { background-color : #AAD800; }");
        ui->lbDiagnostico->setText("<html><head/><body><p><span style='font-size:12pt;'>Lixão diagnosticado</span></p></body></html>");

        QSqlQuery query2;
        query2.prepare("SELECT diagnostico FROM lixoes WHERE id=?");
        query2.bindValue(0,_ID);
        query2.exec();
        while (query2.next())
        {
            ui->teDiagnostico->insertHtml(query2.value(0).toString());
        }
    }
}

void lixoes_cad::diagnosticar()
{
    ui->pbStatus->setVisible(true);
    double status = 0;
    int fontsize = 10;
    ui->pbStatus->setValue(0);
    ui->twCadastro->setCurrentIndex(8);

    ui->teDiagnostico->clear();
    ui->teDiagnostico->insertHtml("<html><head></head><body>");

    /*
     * 1ª Etapa: somatório das respostas no grupo (estruturar de form a permitir a listagem da próxima etapa)
     * 2ª Etapa: listar os 3 itens mais pontuados (verificar a necessidade de criar este campo na tabela)
     * 3ª Etapa: recuperar o valor da situação e e calcular Vts grupo a grupo
     * 4ª Etapa: calcular Vtg
     */
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QMap<QString, double> SG;
    QMap<QString, double> RG;
    int maxRespostas = 3;                    // quantidade de respostas de maior valor que serão inseridas no diagnóstico
    QMap<QString, double> respostasGrupo1;   // insere as respostas do grupo na matriz para ordenar as maxRespostas de maior valor
    QMap<QString, double> respostasGrupo2;
    QMap<QString, double> respostasGrupo3;
    QMap<QString, double> respostasGrupo4;
    QMap<QString, double> respostasGrupo5;
    QMap<QString, double> respostasGrupo6;

    // 1ª Etapa: somatório das respostas de cada grupo [inicio]
    QSqlQuery queryRG;
    queryRG.prepare("SELECT * FROM lixoes WHERE id=?");
    queryRG.bindValue(0,_ID);
    queryRG.exec();

    while (queryRG.next())
    {
        QString nome = queryRG.value(1).toString();
        ui->teDiagnostico->insertHtml("<span style='font-size:"+QString::number(fontsize)+"pt;'><b><u>Diagnóstico do lixão "+nome+"</u></b></span>");
        // Grupo 01 [início]
        status = status+7.7;
        ui->pbStatus->setValue(status);
        ui->lbStatus->setText("Calculando o somatório das respostas da caracterização do lixão...");

        respostasGrupo1["Área do lixão"] = 0;
        for (int g1_1=0;g1_1<=queryRG.value("g1_area").toInt();g1_1++)
            respostasGrupo1["Área do lixão"] = respostasGrupo1["Área do lixão"] + 1.5;

        respostasGrupo1["Atividade do lixão"] = 0;
        for (int g1_2=0;g1_2<=queryRG.value("g1_atividade").toInt();g1_2++)
            respostasGrupo1["Atividade do lixão"] =  respostasGrupo1["Atividade do lixão"] + 1.5;

        respostasGrupo1["Adensamento dos resíduos no lixão"] = 0;
        for (int g1_3=0;g1_3<=queryRG.value("g1_adensamento").toInt();g1_3++)
            respostasGrupo1["Adensamento dos resíduos no lixão"] =  respostasGrupo1["Adensamento dos resíduos no lixão"] + 3;

        respostasGrupo1["Deslizamento no lixão"] = 0;
        for (int g1_4=0;g1_4<=queryRG.value("g1_deslizamento").toInt();g1_4++)
            respostasGrupo1["Deslizamento no lixão"] =  respostasGrupo1["Deslizamento no lixão"] + 3;

        respostasGrupo1["Erosão no lixão"] = 0;
        for (int g1_5=0;g1_5<=queryRG.value("g1_erosao").toInt();g1_5++)
            respostasGrupo1["Erosão no lixão"] =  respostasGrupo1["Erosão no lixão"] + 3;

        respostasGrupo1["Outros acidentes ou eventos no lixão"] = 0;
        for (int g1_6=0;g1_6<=queryRG.value("g1_outros").toInt();g1_6++)
            respostasGrupo1["Outros acidentes ou eventos no lixão"] =  respostasGrupo1["Outros acidentes ou eventos no lixão"] + 3;

        respostasGrupo1["Espessura da camada de resíduos"] = 0;
        for (int g1_7=0;g1_7<=queryRG.value("g1_espessura").toInt();g1_7++)
            respostasGrupo1["Espessura da camada de resíduos"] =  respostasGrupo1["Espessura da camada de resíduos"] + 1.5;

        respostasGrupo1["Predominância de resíduos inertes - classe II B"] = 0; //os campos de classificação de resíduos são checkbox (0,1), então:
        if (queryRG.value("g1_residuos_classe_IIB").toInt()==0)
            respostasGrupo1["Predominância de resíduos inertes - classe II B"] = 0;
        else if (queryRG.value("g1_residuos_classe_IIB").toInt()==1)
            respostasGrupo1["Predominância de resíduos inertes - classe II B"] = 6;

        respostasGrupo1["Predominância de resíduos domésticos - classe II A"] = 0;
        if (queryRG.value("g1_residuos_classe_IIA").toInt()==0)
            respostasGrupo1["Predominância de resíduos domésticos - classe II A"] = 0;
        else if (queryRG.value("g1_residuos_classe_IIA").toInt()==1)
            respostasGrupo1["Predominância de resíduos domésticos - classe II A"] = 6;

        respostasGrupo1["Presença de resíduos perigosos - classe I"] = 0;
        if (queryRG.value("g1_residuos_classe_I").toInt()==0)
            respostasGrupo1["Presença de resíduos perigosos - classe I"] =  0;
        else if (queryRG.value("g1_residuos_classe_I").toInt()==1)
            respostasGrupo1["Presença de resíduos perigosos - classe I"] =  6;

        respostasGrupo1["Impermeabilização superior"] = 0;
        for (int g1_11=0;g1_11<=queryRG.value("g1_impermeabilizacao_superior").toInt();g1_11++)
            respostasGrupo1["Impermeabilização superior"] =  respostasGrupo1["Impermeabilização superior"] + 2;

        respostasGrupo1["Pluviometria"] = 0;
        for (int g1_12=0;g1_12<=queryRG.value("g1_pluviometria").toInt();g1_12++)
            respostasGrupo1["Pluviometria"] =  respostasGrupo1["Pluviometria"] + 1.5;

        respostasGrupo1["Declividade do terreno natural"] = 0;
        for (int g1_13=0;g1_13<=queryRG.value("g1_declividade").toInt();g1_13++)
            respostasGrupo1["Declividade do terreno natural"] =  respostasGrupo1["Declividade do terreno natural"] + 1;

        double somatorioRG1 = respostasGrupo1["Área do lixão"] +
                respostasGrupo1["Atividade do lixão"] +
                respostasGrupo1["Adensamento dos resíduos no lixão"] +
                respostasGrupo1["Deslizamento no lixão"] +
                respostasGrupo1["Erosão no lixão"] +
                respostasGrupo1["Outros acidentes ou eventos no lixão"] +
                respostasGrupo1["Espessura da camada de resíduos"] +
                respostasGrupo1["Predominância de resíduos inertes - classe II B"] +
                respostasGrupo1["Predominância de resíduos domésticos - classe II A"] +
                respostasGrupo1["Presença de resíduos perigosos - classe I"] +
                respostasGrupo1["Impermeabilização superior"] +
                respostasGrupo1["Pluviometria"] +
                respostasGrupo1["Declividade do terreno natural"];

        RG.insert("ΣRG1", somatorioRG1);
        // Grupo 01 [fim]

        // Grupo 02 [início]
        status = status+7.7;
        ui->pbStatus->setValue(status);
        ui->lbStatus->setText("Calculando o somatório das respostas do solo e águas subterrâneas...");

        respostasGrupo2["Impermeabilização inferior"] = 0;
        for (int g2_1=0;g2_1<=queryRG.value("g2_impermeabilizacao_inferior").toInt();g2_1++)
            respostasGrupo2["Impermeabilização inferior"] = respostasGrupo2["Impermeabilização inferior"] + 1.5;

        respostasGrupo2["Presença de lixiviados nos taludes, aterros e entornos"] = 0;
        for (int g2_2=0;g2_2<=queryRG.value("g2_presenca_lixiviado").toInt();g2_2++)
            respostasGrupo2["Presença de lixiviados nos taludes, aterros e entornos"] = respostasGrupo2["Presença de lixiviados nos taludes, aterros e entornos"] + 2;

        respostasGrupo2["Existência de coleta de lixiviados"] = 0;
        for (int g2_3=0;g2_3<=queryRG.value("g2_coleta_lixiviado").toInt();g2_3++)
            respostasGrupo2["Existência de coleta de lixiviados"] = respostasGrupo2["Existência de coleta de lixiviados"] + 2;

        respostasGrupo2["Existência de tratamento de lixiviados"] = 0;
        for (int g2_4=0;g2_4<=queryRG.value("g2_tratamento_lixiviado").toInt();g2_4++)
            respostasGrupo2["Existência de tratamento de lixiviados"] = respostasGrupo2["Existência de tratamento de lixiviados"] + 2;

        respostasGrupo2["Natureza do solo sob o lixão"] = 0;
        for (int g2_5=0;g2_5<=queryRG.value("g2_natureza_solo").toInt();g2_5++)
            respostasGrupo2["Natureza do solo sob o lixão"] = respostasGrupo2["Natureza do solo sob o lixão"] + 2;

        respostasGrupo2["Permeabilidade do solo onde está localizado o lixão"] = 0;
        for (int g2_6=0;g2_6<=queryRG.value("g2_permeabilidade_solo").toInt();g2_6++)
            respostasGrupo2["Permeabilidade do solo onde está localizado o lixão"] = respostasGrupo2["Permeabilidade do solo onde está localizado o lixão"] + 2;

        respostasGrupo2["Nível piezométrico abaixo dos resíduos"] = 0;
        for (int g2_7=0;g2_7<=queryRG.value("g2_nivel_piezometrico").toInt();g2_7++)
            respostasGrupo2["Nível piezométrico abaixo dos resíduos"] = respostasGrupo2["Nível piezométrico abaixo dos resíduos"] + 2;

        respostasGrupo2["Distância entre os resíduos e a água subterrânea inferior a 1,5m"] = 0;
        if (queryRG.value("g2_distancia_residuos_agua").toInt()>=0) // default deste campo é -1 (neste caso, não contabiliza no sistema de pontuação)
        {
            for (int g2_8=0;g2_8<=queryRG.value("g2_distancia_residuos_agua").toInt();g2_8++)
                respostasGrupo2["Distância entre os resíduos e a água subterrânea inferior a 1,5m"] = respostasGrupo2["Distância entre os resíduos e a água subterrânea inferior a 1,5m"] + 2;
        }

        respostasGrupo2["Descontinuidades do terreno sobre o qual está o lixão"] = 0;
        for (int g2_9=0;g2_9<=queryRG.value("g2_descontinuidade_terreno").toInt();g2_9++)
            respostasGrupo2["Descontinuidades do terreno sobre o qual está o lixão"] = respostasGrupo2["Descontinuidades do terreno sobre o qual está o lixão"] + 2;

        respostasGrupo2["Contaminação do solo comprovada de acordo com a resolução CONAMA n° 420/2008"] = 0;
        for (int g2_10=0;g2_10<=queryRG.value("g2_contaminacao_solo").toInt();g2_10++)
            respostasGrupo2["Contaminação do solo comprovada de acordo com a resolução CONAMA n° 420/2008"] = respostasGrupo2["Contaminação do solo comprovada de acordo com a resolução CONAMA n° 420/2008"] + 2;

        respostasGrupo2["Contaminação das águas subterrâneas comprovada de acordo com a resolução CONAMA n° 396/2008"] = 0;
        for (int g2_11=0;g2_11<=queryRG.value("g2_contaminacao_agua").toInt();g2_11++)
            respostasGrupo2["Contaminação das águas subterrâneas comprovada de acordo com a resolução CONAMA n° 396/2008"] = respostasGrupo2["Contaminação das águas subterrâneas comprovada de acordo com a resolução CONAMA n° 396/2008"] + 2;

        respostasGrupo2["Distância de um ponto de alimentação de água potável de uso doméstico"] = 0;
        for (int g2_12=0;g2_12<=queryRG.value("g2_distancia_abast_domestico").toInt();g2_12++)
            respostasGrupo2["Distância de um ponto de alimentação de água potável de uso doméstico"] = respostasGrupo2["Distância de um ponto de alimentação de água potável de uso doméstico"] + 1.5;

        respostasGrupo2["Distância de um ponto de alimentação de água para o abastecimento público"] = 0;
        for (int g2_13=0;g2_13<=queryRG.value("g2_distancia_abast_publico").toInt();g2_13++)
            respostasGrupo2["Distância de um ponto de alimentação de água para o abastecimento público"] = respostasGrupo2["Distância de um ponto de alimentação de água para o abastecimento público"] + 2;

        respostasGrupo2["Uso preponderante da água subterrânea da área ou entorno diretamente afetado pela presença do lixão"] = 0;
        for (int g2_14=0;g2_14<=queryRG.value("g2_uso_preponderante_agua").toInt();g2_14++)
            respostasGrupo2["Uso preponderante da água subterrânea da área ou entorno diretamente afetado pela presença do lixão"] = respostasGrupo2["Uso preponderante da água subterrânea da área ou entorno diretamente afetado pela presença do lixão"] + 2;

        double somatorioRG2 = respostasGrupo2["Impermeabilização inferior"] +
                respostasGrupo2["Presença de lixiviados nos taludes, aterros e entornos"] +
                respostasGrupo2["Existência de coleta de lixiviados"] +
                respostasGrupo2["Existência de tratamento de lixiviados"] +
                respostasGrupo2["Natureza do solo sob o lixão"] +
                respostasGrupo2["Permeabilidade do solo onde está localizado o lixão"] +
                respostasGrupo2["Nível piezométrico abaixo dos resíduos"] +
                respostasGrupo2["Distância entre os resíduos e a água subterrânea inferior a 1,5m"] +
                respostasGrupo2["Descontinuidades do terreno sobre o qual está o lixão"] +
                respostasGrupo2["Contaminação do solo comprovada de acordo com a resolução CONAMA n° 420/2008"] +
                respostasGrupo2["Contaminação das águas subterrâneas comprovada de acordo com a resolução CONAMA n° 396/2008"] +
                respostasGrupo2["Distância de um ponto de alimentação de água potável de uso doméstico"] +
                respostasGrupo2["Distância de um ponto de alimentação de água para o abastecimento público"] +
                respostasGrupo2["Uso preponderante da água subterrânea da área ou entorno diretamente afetado pela presença do lixão"];
        RG.insert("ΣRG2", somatorioRG2);
        // Grupo 02 [fim]

        // Grupo 03 [inicio]
        status = status+7.7;
        ui->pbStatus->setValue(status);
        ui->lbStatus->setText("Calculando o somatório das respostas das águas superficiais...");

        respostasGrupo3["Distância de um ponto de alimentação de água potável de uso doméstico"] = 0;
        for (int g3_1=0;g3_1<=queryRG.value("g3_distancia_abast_domestico").toInt();g3_1++)
            respostasGrupo3["Distância de um ponto de alimentação de água potável de uso doméstico"] = respostasGrupo3["Distância de um ponto de alimentação de água potável de uso doméstico"] + 1.5;

        respostasGrupo3["Utilização das águas (classificação das águas segundo CONAMA 357/2005)"] = 0;
        for (int g3_2=0;g3_2<=queryRG.value("g3_classe_aguas").toInt();g3_2++)
            respostasGrupo3["Utilização das águas (classificação das águas segundo CONAMA 357/2005)"] = respostasGrupo3["Utilização das águas (classificação das águas segundo CONAMA 357/2005)"] + 1.2;

        respostasGrupo3["Distância entre o lixão e a borda do corpo hídrico mais próximo"] = 0;
        for (int g3_3=0;g3_3<=queryRG.value("g3_distancia_corpo_hidrico").toInt();g3_3++)
            respostasGrupo3["Distância entre o lixão e a borda do corpo hídrico mais próximo"] = respostasGrupo3["Distância entre o lixão e a borda do corpo hídrico mais próximo"] + 2;

        respostasGrupo3["Distância de um ponto de alimentação de água para abastecimento público"] = 0;
        for (int g3_4=0;g3_4<=queryRG.value("g3_distancia_abast_publico").toInt();g3_4++)
            respostasGrupo3["Distância de um ponto de alimentação de água para abastecimento público"] = respostasGrupo3["Distância de um ponto de alimentação de água para abastecimento público"] + 2;

        respostasGrupo3["Distância da zona balneável mais próxima"] = 0;
        for (int g3_5=0;g3_5<=queryRG.value("g3_distancia_zona_balneavel").toInt();g3_5++)
            respostasGrupo3["Distância da zona balneável mais próxima"] = respostasGrupo3["Distância da zona balneável mais próxima"] + 2;

        respostasGrupo3["Distância de nascente d’água mais próxima"] = 0;
        for (int g3_6=0;g3_6<=queryRG.value("g3_distancia_nascente").toInt();g3_6++)
            respostasGrupo3["Distância de nascente d’água mais próxima"] = respostasGrupo3["Distância de nascente d’água mais próxima"] + 2;

        respostasGrupo3["Uso preponderante da água de superfície"] = 0;
        for (int g3_7=0;g3_7<=queryRG.value("g3_uso_preponderante_agua").toInt();g3_7++)
            respostasGrupo3["Uso preponderante da água de superfície"] = respostasGrupo3["Uso preponderante da água de superfície"] + 2;

        respostasGrupo3["Poluição das águas constatada por análises - valores máximos permitidos de turbidez, DQO, "
                        "DBO, pH, OD, E. coli, cloreto e nitrogênio amoniacal estabelecidos pela Resolução "
                        "CONAMA N° 357/2005"] = 0;
        for (int g3_8=0;g3_8<=queryRG.value("g3_poluicao_agua").toInt();g3_8++)
            respostasGrupo3["Poluição das águas constatada por análises - valores máximos permitidos de turbidez, DQO, "
                            "DBO, pH, OD, E. coli, cloreto e nitrogênio amoniacal estabelecidos pela Resolução "
                            "CONAMA N° 357/2005"] = respostasGrupo3["Poluição das águas constatada por análises - valores máximos permitidos de turbidez, DQO, "
                            "DBO, pH, OD, E. coli, cloreto e nitrogênio amoniacal estabelecidos pela Resolução "
                            "CONAMA N° 357/2005"] + 3;

        double somatorioRG3 = respostasGrupo3["Distância de um ponto de alimentação de água potável de uso doméstico"] +
                respostasGrupo3["Utilização das águas (classificação das águas segundo CONAMA 357/2005)"] +
                respostasGrupo3["Distância entre o lixão e a borda do corpo hídrico mais próximo"] +
                respostasGrupo3["Distância de um ponto de alimentação de água para abastecimento público"] +
                respostasGrupo3["Distância da zona balneável mais próxima"] +
                respostasGrupo3["Distância de nascente d’água mais próxima"] +
                respostasGrupo3["Uso preponderante da água de superfície"] +
                respostasGrupo3["Poluição das águas constatada por análises - valores máximos permitidos de turbidez, DQO, "
                                "DBO, pH, OD, E. coli, cloreto e nitrogênio amoniacal estabelecidos pela Resolução "
                                "CONAMA N° 357/2005"];
        RG.insert("ΣRG3", somatorioRG3);
        // Grupo 03 [fim]

        // Grupo 04 [inicio]
        status = status+7.7;
        ui->pbStatus->setValue(status);
        ui->lbStatus->setText("Calculando o somatório das respostas do meio social...");

        respostasGrupo4["Densidade populacional dentro de um raio de 500m"] = 0;
        for (int g4_1=0;g4_1<=queryRG.value("g4_densidade_populacional").toInt();g4_1++)
            respostasGrupo4["Densidade populacional dentro de um raio de 500m"] = respostasGrupo4["Densidade populacional dentro de um raio de 500m"] + 2;

        respostasGrupo4["Presença de hospital, creche, escola ou asilo na área do lixão ou num raio de 500m"] = 0;
        for (int g4_2=0;g4_2<=queryRG.value("g4_hospital_creche_escola_asilo").toInt();g4_2++)
            respostasGrupo4["Presença de hospital, creche, escola ou asilo na área do lixão ou num raio de 500m"] = respostasGrupo4["Presença de hospital, creche, escola ou asilo na área do lixão ou num raio de 500m"] + 3;

        respostasGrupo4["Distância do núcleo populacional mais próximo"] = 0;
        for (int g4_3=0;g4_3<=queryRG.value("g4_distancia_populacao").toInt();g4_3++)
            respostasGrupo4["Distância do núcleo populacional mais próximo"] = respostasGrupo4["Distância do núcleo populacional mais próximo"] + 2;

        respostasGrupo4["Existência de atividades agropecuárias na área ou no entorno"] = 0;
        for (int g4_4=0;g4_4<=queryRG.value("g4_atividades_agropecuarias").toInt();g4_4++)
            respostasGrupo4["Existência de atividades agropecuárias na área ou no entorno"] = respostasGrupo4["Existência de atividades agropecuárias na área ou no entorno"] + 2;

        respostasGrupo4["Utilização da área ou no entorno para atividades de lazer"] = 0;
        for (int g4_5=0;g4_5<=queryRG.value("g4_atividades_lazer").toInt();g4_5++)
            respostasGrupo4["Utilização da área ou no entorno para atividades de lazer"] = respostasGrupo4["Utilização da área ou no entorno para atividades de lazer"] + 2;

        respostasGrupo4["Zona de isolamento físico do lixão"] = 0;
        for (int g4_6=0;g4_6<=queryRG.value("g4_isolamento_fisico_lixao").toInt();g4_6++)
            respostasGrupo4["Zona de isolamento físico do lixão"] = respostasGrupo4["Zona de isolamento físico do lixão"] + 2;

        respostasGrupo4["Presença de insetos no lixão"] = 0;
        if (queryRG.value("g4_insetos").toInt()==0)
            respostasGrupo4["g4_insetos"] = 0;
        else if (queryRG.value("g4_insetos").toInt()==1)
            respostasGrupo4["Presença de insetos no lixão"] = 6;

        respostasGrupo4["Presença de roedores no lixão"] = 0;
        if (queryRG.value("g4_roedores").toInt()==0)
            respostasGrupo4["g4_roedores"] = 0;
        else if (queryRG.value("g4_roedores").toInt()==1)
            respostasGrupo4["Presença de roedores no lixão"] = 6;

        respostasGrupo4["Presença de escorpiões no lixão"] = 0;
        if (queryRG.value("g4_escorpioes").toInt()==0)
            respostasGrupo4["g4_escorpioes"] = 0;
        else if (queryRG.value("g4_escorpioes").toInt()==1)
            respostasGrupo4["Presença de escorpiões no lixão"] = 6;

        respostasGrupo4["Presença de urubus no lixão"] = 0;
        if (queryRG.value("g4_urubus").toInt()==0)
            respostasGrupo4["g4_urubus"] = 0;
        else if (queryRG.value("g4_urubus").toInt()==1)
            respostasGrupo4["Presença de urubus no lixão"] = 6;

        respostasGrupo4["Presença de outras aves no lixão"] = 0;
        if (queryRG.value("g4_outras_aves").toInt()==0)
            respostasGrupo4["g4_outras_aves"] = 0;
        else if (queryRG.value("g4_outras_aves").toInt()==1)
            respostasGrupo4["Presença de outras aves no lixão"] = 6;

        respostasGrupo4["Presença de outros animais no lixão"] = 0;
        if (queryRG.value("g4_outros_animais").toInt()==0)
            respostasGrupo4["g4_outros_animais"] = 0;
        else if (queryRG.value("g4_outros_animais").toInt()==1)
            respostasGrupo4["Presença de outros animais no lixão"] = 6;

        respostasGrupo4["Danos à saúde da população residente no lixão e/ou entorno"] = 0;
        for (int g4_8=0;g4_8<=queryRG.value("g4_danos_saude_populacao").toInt();g4_8++)
            respostasGrupo4["Danos à saúde da população residente no lixão e/ou entorno"] = respostasGrupo4["Danos à saúde da população residente no lixão e/ou entorno"] + 1.5;

        respostasGrupo4["Danos materiais à população residente no lixão e/ou entorno"] = 0;
        for (int g4_9=0;g4_9<=queryRG.value("g4_danos_materiais_populacao").toInt();g4_9++)
            respostasGrupo4["Danos materiais à população residente no lixão e/ou entorno"] = respostasGrupo4["Danos materiais à população residente no lixão e/ou entorno"] + 1.5;

        respostasGrupo4["Existência de catadores"] = 0;
        for (int g4_10=0;g4_10<=queryRG.value("g4_catadores").toInt();g4_10++)
            respostasGrupo4["Existência de catadores"] = respostasGrupo4["Existência de catadores"] + 3;

        double somatorioRG4 = respostasGrupo4["Densidade populacional dentro de um raio de 500m"] +
                respostasGrupo4["Presença de hospital, creche, escola ou asilo na área do lixão ou num raio de 500m"] +
                respostasGrupo4["Distância do núcleo populacional mais próximo"] +
                respostasGrupo4["Existência de atividades agropecuárias na área ou no entorno"] +
                respostasGrupo4["Utilização da área ou no entorno para atividades de lazer"] +
                respostasGrupo4["Zona de isolamento físico do lixão"] +
                respostasGrupo4["Presença de insetos no lixão"] +
                respostasGrupo4["Presença de roedores no lixão"] +
                respostasGrupo4["Presença de escorpiões no lixão"] +
                respostasGrupo4["Presença de urubus no lixão"] +
                respostasGrupo4["Presença de outras aves no lixão"] +
                respostasGrupo4["Presença de outros animais no lixão"] +
                respostasGrupo4["Danos à saúde da população residente no lixão e/ou entorno"] +
                respostasGrupo4["Danos materiais à população residente no lixão e/ou entorno"] +
                respostasGrupo4["Existência de catadores"];
        RG.insert("ΣRG4", somatorioRG4);
        // Grupo 04 [fim]

        // Grupo 05 [inicio]
        status = status+7.7;
        ui->pbStatus->setValue(status);
        ui->lbStatus->setText("Calculando o somatório das respostas do meio natural e paisagens...");

        respostasGrupo5["Largura da barreira vegetal (cerca viva) do lixão"] = 0;
        for (int g5_1=0;g5_1<=queryRG.value("g5_largura_barreira_vegetal").toInt();g5_1++)
            respostasGrupo5["Largura da barreira vegetal (cerca viva) do lixão"] = respostasGrupo5["Largura da barreira vegetal (cerca viva) do lixão"] + 2;

        respostasGrupo5["Distância de um elemento cultural, turístico, arqueológico ou ambiental importante"] = 0;
        for (int g5_2=0;g5_2<=queryRG.value("g5_distancia_elemento").toInt();g5_2++)
            respostasGrupo5["Distância de um elemento cultural, turístico, arqueológico ou ambiental importante"] = respostasGrupo5["Distância de um elemento cultural, turístico, arqueológico ou ambiental importante"] + 1.5;

        respostasGrupo5["Existência de desmatamento e/ou de redução de biodiversidade em razão da presença do lixão"] = 0;
        for (int g5_3=0;g5_3<=queryRG.value("g5_desmatamento").toInt();g5_3++)
            respostasGrupo5["Existência de desmatamento e/ou de redução de biodiversidade em razão da presença do lixão"] = respostasGrupo5["Existência de desmatamento e/ou de redução de biodiversidade em razão da presença do lixão"] + 2;

        respostasGrupo5["Dispersão de resíduos no entorno"] = 0;
        for (int g5_4=0;g5_4<=queryRG.value("g5_dispersao_residuos").toInt();g5_4++)
            respostasGrupo5["Dispersão de resíduos no entorno"] = respostasGrupo5["Dispersão de resíduos no entorno"] + 3;

        respostasGrupo5["Possibilidade das águas subterrâneas ou superficiais contaminadas se dirigirem a um mangue ou pântano"] = 0;
        for (int g5_5=0;g5_5<=queryRG.value("g5_contaminacao_mangue_pantano").toInt();g5_5++)
            respostasGrupo5["Possibilidade das águas subterrâneas ou superficiais contaminadas se dirigirem a um mangue ou pântano"] = respostasGrupo5["Possibilidade das águas subterrâneas ou superficiais contaminadas se dirigirem a um mangue ou pântano"] + 1.5;

        respostasGrupo5["Danos aos animais domésticos e/ou selvagens"] = 0;
        for (int g5_6=0;g5_6<=queryRG.value("g5_danos_animais").toInt();g5_6++)
            respostasGrupo5["Danos aos animais domésticos e/ou selvagens"] = respostasGrupo5["Danos aos animais domésticos e/ou selvagens"] + 2;

        double somatorioRG5 = respostasGrupo5["Largura da barreira vegetal (cerca viva) do lixão"] +
                respostasGrupo5["Distância de um elemento cultural, turístico, arqueológico ou ambiental importante"] +
                respostasGrupo5["Existência de desmatamento e/ou de redução de biodiversidade em razão da presença do lixão"] +
                respostasGrupo5["Dispersão de resíduos no entorno"] +
                respostasGrupo5["Possibilidade das águas subterrâneas ou superficiais contaminadas se dirigirem a um mangue ou pântano"] +
                respostasGrupo5["Danos aos animais domésticos e/ou selvagens"];
        RG.insert("ΣRG5", somatorioRG5);
        // Grupo 05 [fim]

        // Grupo 06 [inicio]
        status = status+7.7;
        ui->pbStatus->setValue(status);
        ui->lbStatus->setText("Calculando o somatório das respostas do meio atmosférico...");

        respostasGrupo6["Presença de odores no lixão e/ou entorno"] = 0;
        for (int g6_1=0;g6_1<=queryRG.value("g6_odores").toInt();g6_1++)
            respostasGrupo6["Presença de odores no lixão e/ou entorno"] = respostasGrupo6["Presença de odores no lixão e/ou entorno"] + 2;

        respostasGrupo6["Ocorrência de explosões recentes"] = 0;
        for (int g6_2=0;g6_2<=queryRG.value("g6_explosoes").toInt();g6_2++)
            respostasGrupo6["Ocorrência de explosões recentes"] = respostasGrupo6["Ocorrência de explosões recentes"] + 1.5;

        respostasGrupo6["Queima de resíduos"] = 0;
        for (int g6_3=0;g6_3<=queryRG.value("g6_queima_residuos").toInt();g6_3++)
            respostasGrupo6["Queima de resíduos"] = respostasGrupo6["Queima de resíduos"] + 2;

        respostasGrupo6["Possibilidade de bolsões de gás e/ou de migração de biogás"] = 0;
        for (int g6_4=0;g6_4<=queryRG.value("g6_bolsoes_migracao_biogas").toInt();g6_4++)
            respostasGrupo6["Possibilidade de bolsões de gás e/ou de migração de biogás"] = respostasGrupo6["Possibilidade de bolsões de gás e/ou de migração de biogás"] + 2;

        respostasGrupo6["Existência coleta de gás"] = 0;
        for (int g6_5=0;g6_5<=queryRG.value("g6_coleta_gas").toInt();g6_5++)
            respostasGrupo6["Existência coleta de gás"] = respostasGrupo6["Existência coleta de gás"] + 1.5;

        respostasGrupo6["Existência de tratamento de gás"] = 0;
        for (int g6_6=0;g6_6<=queryRG.value("g6_tratamento_gas").toInt();g6_6++)
            respostasGrupo6["Existência de tratamento de gás"] = respostasGrupo6["Existência de tratamento de gás"] + 2;

        double somatorioRG6 = respostasGrupo6["Presença de odores no lixão e/ou entorno"] +
                respostasGrupo6["Ocorrência de explosões recentes"] +
                respostasGrupo6["Queima de resíduos"] +
                respostasGrupo6["Possibilidade de bolsões de gás e/ou de migração de biogás"] +
                respostasGrupo6["Existência coleta de gás"] +
                respostasGrupo6["Existência de tratamento de gás"];
        RG.insert("ΣRG6", somatorioRG6);
        // Grupo 06 [fim]
    }
    // 1ª Etapa: somatório das respostas de cada grupo [fim]

    // 2ª Etapa: listar as maxRespostas respostas de maior valor em cada grupo [início]
    // Grupo 01 [inicio]
    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Listando as questões de maior impacto na caracterização do lixão...");
    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Questões de maior impacto na <u>caracterização do lixão</u>:</b></span>");

    QSqlQuery queryTMP;
    QMapIterator<QString, double> g1(respostasGrupo1);
    while (g1.hasNext()) {
        g1.next();
        queryTMP.prepare("INSERT INTO lixoes_tmp (grupo,questao,valor) VALUES (?,?,?)");
        queryTMP.bindValue(0,1);
        queryTMP.bindValue(1,g1.key());
        queryTMP.bindValue(2,g1.value());
        queryTMP.exec();
    }

    queryTMP.prepare("SELECT questao,valor FROM lixoes_tmp WHERE grupo=1 ORDER BY valor desc");
    queryTMP.exec();
    queryTMP.first();

    for (int g1=1;g1<=maxRespostas;g1++)
    {
        QString tmp = queryTMP.value(0).toString();
        ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'>- "+tmp+"</span>");
        queryTMP.next();
    }
    // Grupo 01 [fim]

    // Grupo 02 [inicio]
    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Listando as respostas de maior valor do solo e águas subterrâneas...");
    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Questões de maior impacto no <u>solo e águas subterrâneas</u>:</b></span>");

    QMapIterator<QString, double> g2(respostasGrupo2);
    while (g2.hasNext()) {
        g2.next();
        queryTMP.prepare("INSERT INTO lixoes_tmp (grupo,questao,valor) VALUES (?,?,?)");
        queryTMP.bindValue(0,2);
        queryTMP.bindValue(1,g2.key());
        queryTMP.bindValue(2,g2.value());
        queryTMP.exec();
    }

    queryTMP.prepare("SELECT questao,valor FROM lixoes_tmp WHERE grupo=2 ORDER BY valor desc");
    queryTMP.exec();
    queryTMP.first();

    for (int g2=1;g2<=maxRespostas;g2++)
    {
        QString tmp = queryTMP.value(0).toString();
        ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'>- "+tmp+"</span>");
        queryTMP.next();
    }
    // Grupo 02 [fim]

    // Grupo 03 [inicio]
    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Listando as respostas de maior valor das águas superficiais...");
    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Questões de maior impacto nas <u>águas superficiais</u>:</b></span>");

    QMapIterator<QString, double> g3(respostasGrupo3);
    while (g3.hasNext()) {
        g3.next();
        queryTMP.prepare("INSERT INTO lixoes_tmp (grupo,questao,valor) VALUES (?,?,?)");
        queryTMP.bindValue(0,3);
        queryTMP.bindValue(1,g3.key());
        queryTMP.bindValue(2,g3.value());
        queryTMP.exec();
    }

    queryTMP.prepare("SELECT questao,valor FROM lixoes_tmp WHERE grupo=3 ORDER BY valor desc");
    queryTMP.exec();
    queryTMP.first();

    for (int g3=1;g3<=maxRespostas;g3++)
    {
        QString tmp = queryTMP.value(0).toString();
        ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'>- "+tmp+"</span>");
        queryTMP.next();
    }
    // Grupo 03 [fim]

    // Grupo 04 [inicio]
    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Listando as respostas de maior valor do meio social...");
    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Questões de maior impacto no <u>meio social</u></b>:</span>");

    QMapIterator<QString, double> g4(respostasGrupo4);
    while (g4.hasNext()) {
        g4.next();
        queryTMP.prepare("INSERT INTO lixoes_tmp (grupo,questao,valor) VALUES (?,?,?)");
        queryTMP.bindValue(0,4);
        queryTMP.bindValue(1,g4.key());
        queryTMP.bindValue(2,g4.value());
        queryTMP.exec();
    }

    queryTMP.prepare("SELECT questao,valor FROM lixoes_tmp WHERE grupo=4 ORDER BY valor desc");
    queryTMP.exec();
    queryTMP.first();

    for (int g4=1;g4<=maxRespostas;g4++)
    {
        QString tmp = queryTMP.value(0).toString();
        ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'>- "+tmp+"</span>");
        queryTMP.next();
    }
    // Grupo 04 [fim]

    // Grupo 05 [inicio]
    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Listando as respostas de maior valor do meio natural e paisagens...");
    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Questões de maior impacto nas <u>meio natural e paisagens</u></b>:</span>");

    QMapIterator<QString, double> g5(respostasGrupo5);
    while (g5.hasNext()) {
        g5.next();
        queryTMP.prepare("INSERT INTO lixoes_tmp (grupo,questao,valor) VALUES (?,?,?)");
        queryTMP.bindValue(0,5);
        queryTMP.bindValue(1,g5.key());
        queryTMP.bindValue(2,g5.value());
        queryTMP.exec();
    }

    queryTMP.prepare("SELECT questao,valor FROM lixoes_tmp WHERE grupo=5 ORDER BY valor desc");
    queryTMP.exec();
    queryTMP.first();

    for (int g5=1;g5<=maxRespostas;g5++)
    {
        QString tmp = queryTMP.value(0).toString();
        ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'>- "+tmp+"</span>");
        queryTMP.next();
    }
    // Grupo 05 [fim]

    // Grupo 06 [inicio]
    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Listando as respostas de maior valor do meio atmosférico...");
    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Questões de maior impacto no <u>meio atmosférico</u>:</b></span>");

    QMapIterator<QString, double> g6(respostasGrupo6);
    while (g6.hasNext()) {
        g6.next();
        queryTMP.prepare("INSERT INTO lixoes_tmp (grupo,questao,valor) VALUES (?,?,?)");
        queryTMP.bindValue(0,6);
        queryTMP.bindValue(1,g6.key());
        queryTMP.bindValue(2,g6.value());
        queryTMP.exec();
    }

    queryTMP.prepare("SELECT questao,valor FROM lixoes_tmp WHERE grupo=6 ORDER BY valor desc");
    queryTMP.exec();

    queryTMP.first();
    for (int g6=1;g6<=maxRespostas;g6++)
    {
        QString tmp = queryTMP.value(0).toString();
        ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'>- "+tmp+"</span>");
        queryTMP.next();
    }
    // Grupo 06 [fim]

    queryTMP.prepare("DELETE FROM lixoes_tmp WHERE id >0");
    queryTMP.exec();
    // 2ª Etapa: listar as maxRespostas respostas de maior valor em cada grupo [fim]

    // 3ª Etapa: Somatório das situações, pontuação e nível de impacto do lixão  [inicio]
    QSqlQuery query;
    QSqlQuery querySG;
    QSqlQuery queryPL;
    query.prepare("SELECT lei_9985, area_urbana_rural, ocorrencia_talvegue, ocorrencia_varzea, "
                  "ocorrencia_corpos_hidricos, area_industrial FROM lixoes WHERE id=?");
    query.bindValue(0,_ID);
    query.exec();

    status = status+7.7;
    ui->pbStatus->setValue(status);
    ui->lbStatus->setText("Calculando a pontuação e determinando o nível de impacto do lixão...");

    double somatorioSG1 = 0;
    double somatorioSG2 = 0;
    double somatorioSG3 = 0;
    double somatorioSG4 = 0;
    double somatorioSG5 = 0;

    while(query.next())
    {
        if (query.value("lei_9985").toInt()==1) //ocorreu lei_9985 (situação 1)
        {
            for (int s1=1;s1<=6;s1++)
            {
                querySG.prepare("SELECT situacao_1 FROM valor_situacao WHERE id=?");
                querySG.bindValue(0,s1);
                querySG.exec();
                while (querySG.next())
                    //ΣSG1 = (ΣRG1*S1G1)+(ΣRG2*S1G2)+(ΣRG3*S1G3)+(ΣRG4*S1G4)+(ΣRG5*S1G5)+(ΣRG6*S1G6)
                    somatorioSG1 = somatorioSG1+(RG.find("ΣRG"+QString::number(s1)).value() * querySG.value(0).toInt());
            }
            SG.insert("ΣSG1",somatorioSG1);
        }
        else
            SG.insert("ΣSG1",0);

        if (query.value("area_urbana_rural").toInt()==0) //ocorreu area_urbana (situação 2)
        {            
            for (int s2=1;s2<=6;s2++)
            {
                querySG.prepare("SELECT situacao_2 FROM valor_situacao WHERE id=?");
                querySG.bindValue(0,s2);
                querySG.exec();
                while (querySG.next())
                    //ΣSG2 = (ΣRG1*S2G1)+(ΣRG2*S2G2)+(ΣRG3*S2G3)+(ΣRG4*S2G4)+(ΣRG5*S2G5)+(ΣRG6*S2G6)
                    somatorioSG2 = somatorioSG2+(RG.find("ΣRG"+QString::number(s2)).value() * querySG.value(0).toInt());
            }
            SG.insert("ΣSG2",somatorioSG2);
            SG.insert("ΣSG3",0);
        }
        else if (query.value("area_urbana_rural").toInt()==1) //ocorreu area_rural (situação 3)
        {            
            for (int s3=1;s3<=6;s3++)
            {
                querySG.prepare("SELECT situacao_3 FROM valor_situacao WHERE id=?");
                querySG.bindValue(0,s3);
                querySG.exec();
                while (querySG.next())
                    //ΣSG3 = (ΣRG1*S3G1)+(ΣRG2*S3G2)+(ΣRG3*S3G3)+(ΣRG4*S3G4)+(ΣRG5*S3G5)+(ΣRG6*S3G6)
                    somatorioSG3 = somatorioSG3+(RG.find("ΣRG"+QString::number(s3)).value() * querySG.value(0).toInt());
            }
            SG.insert("ΣSG2",0);
            SG.insert("ΣSG3",somatorioSG3);
        }

        if ((query.value("ocorrencia_talvegue").toInt()==1) ||          //ocorreu ocorrencia_talvegue (situação 4)
                (query.value("ocorrencia_varzea").toInt()==1) ||        //ocorreu ocorrencia_varzea (situação 4)
                (query.value("ocorrencia_corpos_hidricos").toInt()>=1)) //ocorreu ocorrencia_corpos_hidricos (situação 4)
        {            
            for (int s4=1;s4<=6;s4++)
            {
                querySG.prepare("SELECT situacao_4 FROM valor_situacao WHERE id=?");
                querySG.bindValue(0,s4);
                querySG.exec();
                while (querySG.next())
                    //ΣSG4 = (ΣRG1*S4G1)+(ΣRG2*S4G2)+(ΣRG3*S4G3)+(ΣRG4*S4G4)+(ΣRG5*S4G5)+(ΣRG6*S4G6)
                    somatorioSG4 = somatorioSG4+(RG.find("ΣRG"+QString::number(s4)).value() * querySG.value(0).toInt());
            }
            SG.insert("ΣSG4",somatorioSG4);
        }
        else
            SG.insert("ΣSG4",0);

        if (query.value("area_industrial").toInt()==1) //ocorreu area_industrial (situação 5)
        {
            for (int s5=1;s5<=6;s5++)
            {
                querySG.prepare("SELECT situacao_5 FROM valor_situacao WHERE id=?");
                querySG.bindValue(0,s5);
                querySG.exec();
                while (querySG.next())
                    //ΣSG5 = (ΣRG1*S5G1)+(ΣRG2*S5G2)+(ΣRG3*S5G3)+(ΣRG4*S5G4)+(ΣRG5*S5G5)+(ΣRG6*S5G6)
                    somatorioSG5 = somatorioSG5+(RG.find("ΣRG"+QString::number(s5)).value() * querySG.value(0).toInt());
            }
            SG.insert("ΣSG5",somatorioSG5);
        }
        else
            SG.insert("ΣSG5",0);
    }

    double PL = (SG.find("ΣSG1").value()+
                 SG.find("ΣSG2").value()+
                 SG.find("ΣSG3").value()+
                 SG.find("ΣSG4").value()+
                 SG.find("ΣSG5").value())/100;

    ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;'><b>Pontuação do Lixão:</b> "+QString::number(PL)+"</span>");

    queryPL.prepare("SELECT baixo, medio, alto FROM niveis_impacto WHERE id=1");
    queryPL.exec();

    QString NI = "";
    while (queryPL.next())
    {
        if (PL<=79)
        {
            NI = "Baixo";
            ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;background-color:#AAD800;'><b>Nível de impacto do lixão: Baixo</b></span>");
        }
        else if ((PL>=80) && (PL<=159))
        {
            NI = "Médio";
            ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;background-color:#FEFB18;'><b>Nível de impacto do lixão: Médio</b></span>");
        }
        else if (PL>=160)
        {
            NI = "Alto";
            ui->teDiagnostico->insertHtml("<br/><span style='font-size:"+QString::number(fontsize)+"pt;background-color:#FF2800;'><b>Nível de impacto do lixão: Alto</b></span>");
        }
    }
    ui->teDiagnostico->insertHtml("</body></html>");

    queryPL.prepare("UPDATE lixoes SET pontuacao=?, nivel_impacto=?, diagnostico=? WHERE id=?");
    queryPL.bindValue(0,PL);
    queryPL.bindValue(1,NI);
    queryPL.bindValue(2,ui->teDiagnostico->toHtml());
    queryPL.bindValue(3,_ID);
    queryPL.exec();

    ui->pbStatus->setVisible(false);
    ui->lbStatus->setText("<html><head/><body><p><span style=' color:#ff0000;'>* </span><span style=' color:#000000;'>Preenchimento obrigatório para fins de cadastro. Para diagnosticar o lixão, </span><span style=' font-weight:600; color:#000000;'>todos</span><span style=' color:#000000;'> os campos devem ser preenchidos</span></p></body></html>");
    // 3ª Etapa: Somatório das situações, pontuação e nível de impacto do lixão [fim]
}

void lixoes_cad::adicionarFigura(const int &fig)
{
    QMessageBox msg;
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle("Atenção");
    msg.setTextFormat(Qt::RichText);
    msg.setStandardButtons(QMessageBox::Ok);

    if (_ID==0)
    {
        msg.setText("<b>É necessário salvar o registro.<\b>");
        msg.setInformativeText("Para inserir fotos é necessário antes salvar o registro \n"
                                "clicando no botão salvar.");
        msg.exec();
    }
    else
    {
        QString file = QFileDialog::getOpenFileName(this, tr("Select an image"),".", tr("JPEG (*.jpg *jpeg)\n"
                                                                                        "PNG (*.png)\n"
                                                                                        "GIF (*.gif)\n"));
        QImage image = QImageReader (file).read();
        QPixmap pm = QPixmap::fromImage(image);
        QByteArray bytes;
        QBuffer buffer(&bytes);
        QString descricao = "";

        if (fig==1)
        {
            if (ui->leFig1->text()=="")
            {
                msg.setText("Descreva a figura antes de inserir a foto");
                msg.exec();
                ui->leFig1->setFocus();
            }
            else
            {
                //QPixmap pm = QPixmap::fromImage(image).scaled(ui->lbFig1->width(), ui->lbFig1->height());
                buffer.open(QIODevice::WriteOnly);
                pm.save(&buffer, "JPG");
                ui->lbFig1->setPixmap(pm.scaled(280, 200));
                descricao = ui->leFig1->text();
                ui->pbSalvarFig1->setEnabled(true);
                ui->pbExcluirFig1->setEnabled(true);
            }
        }
        else if (fig==2)
        {
            if (ui->leFig2->text()=="")
            {
                msg.setText("Descreva a figura antes de inserir a foto");
                msg.exec();
                ui->leFig2->setFocus();
            }
            else
            {
                //QPixmap pm = QPixmap::fromImage(image).scaled(ui->lbFig2->width(), ui->lbFig2->height());
                buffer.open(QIODevice::WriteOnly);
                pm.save(&buffer, "JPG");
                ui->lbFig2->setPixmap(pm.scaled(280, 200));
                descricao = ui->leFig2->text();
                ui->pbSalvarFig2->setEnabled(true);
                ui->pbExcluirFig2->setEnabled(true);
            }
        }
        else if (fig==3)
        {
            if (ui->leFig3->text()=="")
            {
                msg.setText("Descreva a figura antes de inserir a foto");
                msg.exec();
                ui->leFig3->setFocus();
            }
            else
            {
                //QPixmap pm = QPixmap::fromImage(image).scaled(ui->lbFig3->width(), ui->lbFig3->height());
                buffer.open(QIODevice::WriteOnly);
                pm.save(&buffer, "JPG"); // writes pixmap into bytes in PNG format
                ui->lbFig3->setPixmap(pm.scaled(280, 200));
                descricao = ui->leFig3->text();
                ui->pbSalvarFig3->setEnabled(true);
                ui->pbExcluirFig3->setEnabled(true);
            }
        }
        else if (fig==4)
        {
            if (ui->leFig4->text()=="")
            {
                msg.setText("Descreva a figura antes de inserir a foto");
                msg.exec();
                ui->leFig4->setFocus();
            }
            else
            {
                //QPixmap pm = QPixmap::fromImage(image).scaled(ui->lbFig4->width(), ui->lbFig4->height());
                buffer.open(QIODevice::WriteOnly);
                pm.save(&buffer, "JPG");
                ui->lbFig4->setPixmap(pm.scaled(280, 200));
                descricao = ui->leFig4->text();
                ui->pbSalvarFig4->setEnabled(true);
                ui->pbExcluirFig4->setEnabled(true);
            }
        }
        else if (fig==5)
        {
            if (ui->leFig5->text()=="")
            {
                msg.setText("Descreva a figura antes de inserir a foto");
                msg.exec();
                ui->leFig5->setFocus();
            }
            else
            {
                //QPixmap pm = QPixmap::fromImage(image).scaled(ui->lbFig5->width(), ui->lbFig5->height());
                buffer.open(QIODevice::WriteOnly);
                pm.save(&buffer, "JPG");
                ui->lbFig5->setPixmap(pm.scaled(280, 200));
                descricao = ui->leFig5->text();
                ui->pbSalvarFig5->setEnabled(true);
                ui->pbExcluirFig5->setEnabled(true);
            }
        }
        else if (fig==6)
        {
            if (ui->leFig6->text()=="")
            {
                msg.setText("Descreva a figura antes de inserir a foto");
                msg.exec();
                ui->leFig6->setFocus();
            }
            else
            {
                //QPixmap pm = QPixmap::fromImage(image).scaled(ui->lbFig6->width(), ui->lbFig6->height());
                buffer.open(QIODevice::WriteOnly);
                pm.save(&buffer, "JPG");
                ui->lbFig6->setPixmap(QPixmap::fromImage(image).scaled(ui->lbFig6->width(), ui->lbFig6->height()));
                descricao = ui->leFig6->text();
                ui->pbSalvarFig6->setEnabled(true);
                ui->pbExcluirFig6->setEnabled(true);
            }
        }

        QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
        con.setDatabaseName(_DB);
        con.open();

        QSqlQuery querySELECT;
        querySELECT.prepare("SELECT count(*) FROM lixoes_fotos WHERE id_lixao=? AND id_figura=?");
        querySELECT.bindValue(0,_ID);
        querySELECT.bindValue(1,fig);
        querySELECT.exec();

        while (querySELECT.next())
        {
            if (querySELECT.value(0).toInt()==0) // se a figura não existe no banco de dados então insere
            {
                QSqlQuery queryINSERT;
                queryINSERT.prepare("INSERT INTO lixoes_fotos (id_lixao, id_figura, figura, descricao) VALUES (?,?,?,?)");
                queryINSERT.bindValue(0,_ID);
                queryINSERT.bindValue(1,fig);
                queryINSERT.bindValue(2,bytes);
                queryINSERT.bindValue(3,descricao);
                queryINSERT.exec();
            }
            else if (querySELECT.value(0).toInt()==1) // se a figura existe no banco de dados então atualiza
            {
                QSqlQuery queryUPDATE;
                queryUPDATE.prepare("UPDATE lixoes_fotos set figura=?, descricao=? WHERE id_lixao=? AND id_figura=?");
                queryUPDATE.bindValue(0,bytes);
                queryUPDATE.bindValue(1,descricao);
                queryUPDATE.bindValue(2,_ID);
                queryUPDATE.bindValue(3,fig);
                queryUPDATE.exec();
            }
        }
        con.close();
    }
}

void lixoes_cad::excluirFigura(const int &fig)
{
    int msg = QMessageBox::question(this, "Atenção!", "Deseja excluir a figura?",QMessageBox::Yes|QMessageBox::No);
    if (msg == QMessageBox::Yes)
    {
        QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
        con.setDatabaseName(_DB);
        con.open();

        QSqlQuery query;
        query.prepare("DELETE FROM lixoes_fotos WHERE id_lixao=? AND id_figura=?");
        query.bindValue(0,_ID);
        query.bindValue(1,fig);
        query.exec();

        if (fig==1)
        {
            ui->leFig1->clear();
            ui->lbFig1->clear();
            ui->pbSalvarFig1->setEnabled(false);
            ui->pbExcluirFig1->setEnabled(false);
        }
        else if (fig==2)
        {
            ui->leFig2->clear();
            ui->lbFig2->clear();
            ui->pbSalvarFig2->setEnabled(false);
            ui->pbExcluirFig2->setEnabled(false);
        }
        else if (fig==3)
        {
            ui->leFig3->clear();
            ui->lbFig3->clear();
            ui->pbSalvarFig3->setEnabled(false);
            ui->pbExcluirFig3->setEnabled(false);
        }
        else if (fig==4)
        {
            ui->leFig4->clear();
            ui->lbFig4->clear();
            ui->pbSalvarFig4->setEnabled(false);
            ui->pbExcluirFig4->setEnabled(false);
        }
        else if (fig==5)
        {
            ui->leFig5->clear();
            ui->lbFig5->clear();
            ui->pbSalvarFig5->setEnabled(false);
            ui->pbExcluirFig5->setEnabled(false);
        }
        else if (fig==6)
        {
            ui->leFig6->clear();
            ui->lbFig6->clear();
            ui->pbSalvarFig6->setEnabled(false);
            ui->pbExcluirFig6->setEnabled(false);
        }
        QMessageBox msg1;
        msg1.setIcon(QMessageBox::Information);
        msg1.setWindowTitle("Confirmação");
        msg1.setStandardButtons(QMessageBox::Ok);
        msg1.setText("Figura excluí­da com sucesso.");
        msg1.exec();

        con.close();
    }
}

void lixoes_cad::salvarFigura(const int &fig)
{

    QString SQL = "";

    if (fig==1)
        SQL = "SELECT * FROM lixoes_fotos WHERE id_lixao=? AND id_figura=1";
    else if (fig==2)
        SQL = "SELECT * FROM lixoes_fotos WHERE id_lixao=? AND id_figura=2";
    else if (fig==3)
        SQL = "SELECT * FROM lixoes_fotos WHERE id_lixao=? AND id_figura=3";
    else if (fig==4)
        SQL = "SELECT * FROM lixoes_fotos WHERE id_lixao=? AND id_figura=4";
    else if (fig==5)
        SQL = "SELECT * FROM lixoes_fotos WHERE id_lixao=? AND id_figura=5";
    else if (fig==6)
        SQL = "SELECT * FROM lixoes_fotos WHERE id_lixao=? AND id_figura=6";

    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare(SQL);
    query.bindValue(0,_ID);
    query.exec();

    query.first();

    QByteArray bytes = query.value("figura").toByteArray();
    QPixmap pm;
    pm.loadFromData(bytes);

    QString s = qApp->applicationDirPath();
    QString filename = QFileDialog::getSaveFileName(0,tr("Salvar figura"),s,tr("Imagens (*.jpg)"),0,0); //this, tr("Salvar figura"),pm,tr("Imagens (*.jpg)")
    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    f.write(bytes);
    f.close();
    con.close();
}

void lixoes_cad::selecionarFigura()
{
    QSqlDatabase con = QSqlDatabase::addDatabase("QSQLITE");
    con.setDatabaseName(_DB);
    con.open();

    QSqlQuery query;
    query.prepare("SELECT * FROM lixoes_fotos WHERE id_lixao=? ORDER BY id_figura ASC");
    query.bindValue(0,_ID);
    query.exec();

    while (query.next())
    {
        QByteArray bytes = query.value("figura").toByteArray();
        QPixmap pm;
        pm.loadFromData(bytes);
        QString descicao = query.value("descricao").toString();

        if (query.value("id_figura")==1)
        {
            ui->lbFig1->setPixmap(pm.scaled(280, 200));
            ui->leFig1->setText(descicao);
            ui->pbSalvarFig1->setEnabled(true);
            ui->pbExcluirFig1->setEnabled(true);
        }
        else if (query.value("id_figura")==2)
        {
            ui->lbFig2->setPixmap(pm.scaled(280, 200));
            ui->leFig2->setText(descicao);
            ui->pbSalvarFig2->setEnabled(true);
            ui->pbExcluirFig2->setEnabled(true);
        }
        else if (query.value("id_figura")==3)
        {
            ui->lbFig3->setPixmap(pm.scaled(280, 200));
            ui->leFig3->setText(descicao);
            ui->pbSalvarFig3->setEnabled(true);
            ui->pbExcluirFig3->setEnabled(true);
        }
        else if (query.value("id_figura")==4)
        {
            ui->lbFig4->setPixmap(pm.scaled(280, 200));
            ui->leFig4->setText(descicao);
            ui->pbSalvarFig4->setEnabled(true);
            ui->pbExcluirFig4->setEnabled(true);
        }
        else if (query.value("id_figura")==5)
        {
            ui->lbFig5->setPixmap(pm.scaled(280, 200));
            ui->leFig5->setText(descicao);
            ui->pbSalvarFig5->setEnabled(true);
            ui->pbExcluirFig5->setEnabled(true);
        }
        else if (query.value("id_figura")==6)
        {
            ui->lbFig6->setPixmap(pm.scaled(280, 200));
            ui->leFig6->setText(descicao);
            ui->pbSalvarFig6->setEnabled(true);
            ui->pbExcluirFig6->setEnabled(true);
        }
    }
    con.close();
}


void lixoes_cad::on_pbExcluirFig1_clicked()
{
    excluirFigura(1);
}

void lixoes_cad::on_pbAdicionarFig1_clicked()
{
    adicionarFigura(1);
}

void lixoes_cad::on_pbExcluirFig2_clicked()
{
    excluirFigura(2);
}

void lixoes_cad::on_pbAdicionarFig2_clicked()
{
    adicionarFigura(2);
}

void lixoes_cad::on_pbExcluirFig3_clicked()
{
    excluirFigura(3);
}

void lixoes_cad::on_pbAdicionarFig3_clicked()
{
    adicionarFigura(3);
}

void lixoes_cad::on_pbExcluirFig4_clicked()
{
    excluirFigura(4);
}

void lixoes_cad::on_pbAdicionarFig4_clicked()
{
    adicionarFigura(4);
}

void lixoes_cad::on_pbExcluirFig5_clicked()
{
    excluirFigura(5);
}

void lixoes_cad::on_pbAdicionarFig5_clicked()
{
    adicionarFigura(5);
}

void lixoes_cad::on_pbExcluirFig6_clicked()
{
    excluirFigura(6);
}

void lixoes_cad::on_pbAdicionarFig6_clicked()
{
    adicionarFigura(6);
}

void lixoes_cad::on_pbSalvarFig1_clicked()
{
    salvarFigura(1);
}

void lixoes_cad::on_pbSalvarFig2_clicked()
{
    salvarFigura(2);
}

void lixoes_cad::on_pbSalvarFig3_clicked()
{
    salvarFigura(3);
}

void lixoes_cad::on_pbSalvarFig4_clicked()
{
    salvarFigura(4);
}

void lixoes_cad::on_pbSalvarFig5_clicked()
{
    salvarFigura(5);
}

void lixoes_cad::on_pbSalvarFig6_clicked()
{
    salvarFigura(6);
}
