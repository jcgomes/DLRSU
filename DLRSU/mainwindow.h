#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_niveis_impacto_triggered();

    void on_action_municipios_triggered();

    void on_action_valores_situacoes_triggered();

    void on_action_sobre_sistema_triggered();

    void on_action_sair_triggered();

    void on_action_formCampo_triggered();

    void on_action_manual_sistema_triggered();

    void on_action_cenarios_triggered();

    void on_action_diagnostico_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
