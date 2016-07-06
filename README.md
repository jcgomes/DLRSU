# Informações para os usuários software
Diagnostico de Lixões de Resíduos Sólidos Urbanos (DLRSU), é uma ferramenta de apoio à decisão desenvolvida para auxiliar no diagnóstico de lixões de resíduos sólidos urbanos. É possível trabalhar com quantos lixões e municípios forem necessários. O diagnóstico é feito através de um sistema de pontuação e hierarquização do impacto do lixão, onde quanto maior a pontuação do lixão, maior o seu impacto, consequentemente, maior a prioridade na recuperação do mesmo.

O software foi projetado e desenvolvido para ser fácil de uso e entendimento. As telas são todas padronizadas, ou seja, as telas de consulta possuem os mesmos objetos nos mesmos lugares, bem como as telas de cadastro.

ATENÇÃO:  Leia o Manual do usuário antes de tudo: https://github.com/lareso/DLRSU/release/Manual.pdf

PARA BAIXAR O SOFTWARE (disponível para Windows e Linux): https://github.com/lareso/DLRSU/release

# Informações para os desenvolvedores
Plataforma Windows:

O diretório DLL contém as DLLs necessárias a incluir na compilação do instalador do software (setup). Elas são encontradas do diretório de instalação do mingw (exemplo: "C:\Qt\5.5\mingw492_32\bin" e "C:\Qt\5.5\mingw492_32\plugins") e precisam ser da mesma versão em que o executável foi compilado. Neste caso, foi utilizado o Qt 5.5, mingw492 (32 bits). Caso você compile o código fonte com outra versão do Qt/mingw, substitua as DLLs.

Plataforma Linux: 
- Instalar o qt-sdk (linux mint) ou Qt-devel (outras distribuições): sudo apt-get install qt-sdk
- Baixar do site do Qt, a versão 5.5.0-2 (qt-opensource-linux-x64-5.5.0-2.run). Para instalar, seguir as etapas abaixo: 
  - sudo chmod 777 ./qt-opensource-linux-x64-5.5.0-2.run
  - ./qt-opensource-linux-x64-5.5.0-2.run
- Isto irá instalar a versão "Qt Creator (community)". Recomendo usar ela.

Documentação de desenvolvimento:

O software foi desenvolvido em C++, banco de dados sqlite, e precisa ser melhor documentado, principalmente quanto aos padrões de projeto, banco de dados, todo lists, etc.

Lógica de negócio:

Para compreender o funcionamento do sistema de pontuação criado no software é recomendado ler a dissertação/tese que deram origem a ele, pois trata-se de uma nova abordagem (metodologia) de hierarquização de impactos ambientais no Brasil. Ele foi desenvolvido na UFSC, caso estes documentos sejam publicados online, disponibilizaremos aqui os links.

Cordialmente,

Juliano C. Gomes
