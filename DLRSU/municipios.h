#ifndef MUNICIPIOS_H
#define MUNICIPIOS_H

#include <QDialog>

namespace Ui {
class municipios;
}

class municipios : public QDialog
{
    Q_OBJECT

public:
    explicit municipios(QWidget *parent = 0);
    ~municipios();

private slots:
    void on_tvConsulta_doubleClicked(const QModelIndex &index);

    void on_leParametro_returnPressed();

    void on_leParametro_textChanged(const QString &arg1);

    void on_pbAtualizar_clicked();

    void on_pbAdicionar_clicked();

    void on_pbSair_clicked();

    void buscar();

    void atualizar_db();

private:
    Ui::municipios *ui;
};

#endif // MUNICIPIOS_H
