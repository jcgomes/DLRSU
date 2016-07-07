#ifndef LIXOES_H
#define LIXOES_H

#include <QDialog>
#include <QItemDelegate>

namespace Ui {
class lixoes;
}

class lixoes : public QDialog
{
    Q_OBJECT

public:
    explicit lixoes(QWidget *parent = 0);
    ~lixoes();

private slots:
    void on_tvConsulta_doubleClicked(const QModelIndex &index);

    void on_leParametro_returnPressed();

    void on_leParametro_textChanged(const QString &arg1);

    void on_pbAtualizar_clicked();

    void on_pbAdicionar_clicked();

    void on_pbSair_clicked();

    void atualizar_db();

    void buscar();

private:
    Ui::lixoes *ui;
};

class Delegate : public QItemDelegate
{
    Q_OBJECT
public:
    Delegate(QWidget *parent = 0) : QItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};
#endif // LIXOES_H
