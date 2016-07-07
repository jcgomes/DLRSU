#ifndef LIXOES_CAD_H
#define LIXOES_CAD_H

#include <QDialog>

namespace Ui {
class lixoes_cad;
}

class lixoes_cad : public QDialog
{
    Q_OBJECT

public:
    explicit lixoes_cad(QWidget *parent = 0);
    ~lixoes_cad();

private slots:
    void on_pbSalvar_clicked();

    void on_pbCancelar_clicked();

    void on_rbOutrosAcidentes_0_clicked();

    void on_rbOutrosAcidentes_1_clicked();

    void on_cbNaturezaResiduos_2_clicked();

    void on_rbNivelPiezometrico_0_clicked();

    void on_rbNivelPiezometrico_1_clicked();

    void on_rbNivelPiezometrico_2_clicked();

    void on_cbOutrasAves_clicked();

    void on_cbOutrosAnimais_clicked();

    void on_rbDistanciaElemento_3_clicked();

    void on_pbDiagnosticar_clicked();

    void recuperar_registro();

    void inserir();

    void editar();

    void verificar_diagnostico();

    void diagnosticar();

    void on_rbDistanciaElemento_0_clicked();

    void on_rbDistanciaElemento_1_clicked();

    void on_rbDistanciaElemento_2_clicked();

    void on_pbExcluirFig1_clicked();

    void on_pbAdicionarFig1_clicked();

    void on_pbExcluirFig2_clicked();

    void on_pbAdicionarFig2_clicked();

    void on_pbExcluirFig3_clicked();

    void on_pbAdicionarFig3_clicked();

    void on_pbExcluirFig4_clicked();

    void on_pbAdicionarFig4_clicked();

    void on_pbExcluirFig5_clicked();

    void on_pbAdicionarFig5_clicked();

    void on_pbExcluirFig6_clicked();

    void on_pbAdicionarFig6_clicked();

    void adicionarFigura(const int &fig);

    void excluirFigura(const int &fig);

    void salvarFigura(const int &fig);

    void selecionarFigura();

    void on_pbSalvarFig1_clicked();

    void on_pbSalvarFig2_clicked();

    void on_pbSalvarFig3_clicked();

    void on_pbSalvarFig4_clicked();

    void on_pbSalvarFig5_clicked();

    void on_pbSalvarFig6_clicked();

private:
    Ui::lixoes_cad *ui;
};

#endif // LIXOES_CAD_H
