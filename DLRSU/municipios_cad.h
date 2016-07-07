#ifndef MUNICIPIOS_CAD_H
#define MUNICIPIOS_CAD_H

#include <QDialog>

namespace Ui {
class municipios_cad;
}

class municipios_cad : public QDialog
{
    Q_OBJECT

public:
    explicit municipios_cad(QWidget *parent = 0);
    ~municipios_cad();

private slots:
    void on_pbSalvar_clicked();

    void on_pbCancelar_clicked();

    void recuperar_registro();

    void inserir();

    void editar();

private:
    Ui::municipios_cad *ui;
};

#endif // MUNICIPIOS_CAD_H
