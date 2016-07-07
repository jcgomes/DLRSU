#ifndef LIXOES_CEN_H
#define LIXOES_CEN_H

#include <QDialog>

namespace Ui {
class lixoes_cen;
}

class lixoes_cen : public QDialog
{
    Q_OBJECT

public:
    explicit lixoes_cen(QWidget *parent = 0);
    ~lixoes_cen();

private slots:
    void on_pbSalvar_clicked();

    void on_pbCancelar_clicked();

    void limpar_variaveis();

    void obter_cenario();

    void on_rbAreaProtegida_0_clicked();

    void on_rbAreaProtegida_1_clicked();

    void on_rbRetiradaResiduosCFB_1_clicked();

    void on_rbRetiradaResiduosCFB_0_clicked();

private:
    Ui::lixoes_cen *ui;
};

#endif // LIXOES_CEN_H
