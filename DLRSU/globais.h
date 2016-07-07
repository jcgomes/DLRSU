#ifndef GLOBAIS_H
#define GLOBAIS_H
// globais.h é onde armazenamos as variáveis globais do projeto
#include <string>
#include <QString>

extern int _ID;                             // Recupera o id do registro. id = 0 (insere), id <=1 (atualiza)
extern QString _DB;                         // banco de dados
extern QHash<QString,QString> _cbCampo;     // QHash é mais rápido que o QMap. http://doc.qt.io/qt-5/qhash.html
extern QString _FRM;                        // Usada em mainwindow para definir se a operação é referente ao diagnóstico ou ao cenário do lixão


#endif // GLOBAL_H
