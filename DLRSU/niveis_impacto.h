#ifndef NIVEIS_IMPACTO_H
#define NIVEIS_IMPACTO_H

#include <QDialog>

namespace Ui {
class niveis_impacto;
}

class niveis_impacto : public QDialog
{
    Q_OBJECT

public:
    explicit niveis_impacto(QWidget *parent = 0);
    ~niveis_impacto();

private slots:
    void on_pbSalvar_clicked();

    void on_pbCancelar_clicked();

    void recuperar_registro();

    void formatar_niveis();

private:
    Ui::niveis_impacto *ui;
};

#endif // NIVEIS_IMPACTO_H
