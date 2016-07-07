#ifndef VALOR_SITUACAO_H
#define VALOR_SITUACAO_H

#include <QDialog>

namespace Ui {
class valor_situacao;
}

class valor_situacao : public QDialog
{
    Q_OBJECT

public:
    explicit valor_situacao(QWidget *parent = 0);
    ~valor_situacao();

private slots:
    void on_pbSalvar_clicked();

    void on_pbCancelar_clicked();

    void atualizar();

    void on_pbAtualizar_clicked();

    void on_pbAjuda_clicked();

private:
    Ui::valor_situacao *ui;
};

#endif // VALOR_SITUACAO_H
