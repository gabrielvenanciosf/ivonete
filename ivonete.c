#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GASTOS 7
#define MAX_ATIVOS 6
#define TAM_LINHA 120
#define PERCENTUAL_NECESSIDADES 50.0
#define PERCENTUAL_LAZER_ESTILO 30.0
#define PERCENTUAL_INVESTIMENTOS 20.0

static const double PERCENTUAIS_GASTOS_IDEAIS[MAX_GASTOS] = {
    25.0, /* Moradia - necessidades */
    10.0, /* Alimentacao - necessidades */
    5.0,  /* Transporte - necessidades */
    5.0,  /* Saude - necessidades */
    5.0,  /* Educacao - necessidades */
    20.0, /* Lazer - lazer e estilo de vida */
    10.0  /* Outros - lazer e estilo de vida */
};

typedef struct {
    char nome[80];
    int idade;
    double rendaMensal;
    double gastos[MAX_GASTOS];
    double totalGastos;
    double sobraInvestir;
    int preenchido;
} Orcamento;

typedef struct {
    char nomes[MAX_ATIVOS][80];
    double precos[MAX_ATIVOS];
    double quantidades[MAX_ATIVOS];
    double valoresAtuais[MAX_ATIVOS];
    double pesos[MAX_ATIVOS];
    int preenchida;
} Carteira;

static int atalhoRetornoMenuAtivo = 0;
static int retornoMenuSolicitado = 0;

void limparTela(void);
void pausar(void);
void retornarMenuPrincipal(void);
void ativarAtalhoRetornoMenu(void);
void desativarAtalhoRetornoMenu(void);
int consumirRetornoMenuSolicitado(void);
int linhaSolicitaRetornoMenu(const char *linha);
void exibirAtalhoRetornoMenu(void);
int confirmarContinuar(const char *textoContinuar);
int menuPrincipal(void);
int lerInteiroFaixa(const char *mensagem, int minimo, int maximo);
double lerDoubleMinimo(const char *mensagem, double minimo, int permiteIgual);
int lerTextoObrigatorio(const char *mensagem, char *destino, size_t tamanho);
void inicializarCarteira(Carteira *carteira);
void calcularOrcamentoIdeal(Orcamento *orcamento);
void cadastrarOrcamento(Orcamento *orcamento, const char categorias[MAX_GASTOS][30]);
void escolherPesos(Carteira *carteira);
int identificarPerfilQuestionario(void);
const char *nomePerfilInvestidor(int perfil);
void definirPerfilAutomatico(Carteira *carteira, int perfil);
double somarPesos(const Carteira *carteira);
void cadastrarCarteiraAtual(Carteira *carteira);
void exibirResumo(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]);
void simularRebalanceamento(const Orcamento *orcamento, const Carteira *carteira);
void exibirAjudaInvestimentos(void);
double valorAbsoluto(double valor);

int main(void) {
    const char categorias[MAX_GASTOS][30] = {
        "Moradia",
        "Alimentacao",
        "Transporte",
        "Saude",
        "Educacao",
        "Lazer",
        "Outros"
    };

    Orcamento orcamento = {0};
    Carteira carteira;
    int opcao;

    inicializarCarteira(&carteira);

    do {
        opcao = menuPrincipal();

        switch (opcao) {
            case 1:
                cadastrarOrcamento(&orcamento, categorias);
                break;
            case 2:
                escolherPesos(&carteira);
                break;
            case 3:
                cadastrarCarteiraAtual(&carteira);
                break;
            case 4:
                exibirResumo(&orcamento, &carteira, categorias);
                break;
            case 5:
                exibirAjudaInvestimentos();
                break;
            case 0:
                limparTela();
                printf("Obrigado por usar o simulador. Bons estudos e bons investimentos!\n");
                break;
            default:
                printf("Entrada invalida.\n");
                pausar();
                break;
        }
    } while (opcao != 0);

    return 0;
}

void limparTela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void) {
    char linha[TAM_LINHA];

    printf("\nPressione ENTER para continuar...");
    fgets(linha, sizeof(linha), stdin);
}

void ativarAtalhoRetornoMenu(void) {
    atalhoRetornoMenuAtivo = 1;
    retornoMenuSolicitado = 0;
}

void desativarAtalhoRetornoMenu(void) {
    atalhoRetornoMenuAtivo = 0;
    retornoMenuSolicitado = 0;
}

int consumirRetornoMenuSolicitado(void) {
    int solicitado = retornoMenuSolicitado;
    retornoMenuSolicitado = 0;
    return solicitado;
}

int linhaSolicitaRetornoMenu(const char *linha) {
    const char *inicio;
    const char *fim;
    size_t tamanho;

    if (linha == NULL) {
        return 0;
    }

    inicio = linha;
    while (isspace((unsigned char)*inicio)) {
        inicio++;
    }

    fim = inicio + strlen(inicio);
    while (fim > inicio && isspace((unsigned char)*(fim - 1))) {
        fim--;
    }

    tamanho = (size_t)(fim - inicio);

    if (tamanho == 4 &&
        tolower((unsigned char)inicio[0]) == 'm' &&
        tolower((unsigned char)inicio[1]) == 'e' &&
        tolower((unsigned char)inicio[2]) == 'n' &&
        tolower((unsigned char)inicio[3]) == 'u') {
        return 1;
    }

    if (tamanho == 6 &&
        tolower((unsigned char)inicio[0]) == 'v' &&
        tolower((unsigned char)inicio[1]) == 'o' &&
        tolower((unsigned char)inicio[2]) == 'l' &&
        tolower((unsigned char)inicio[3]) == 't' &&
        tolower((unsigned char)inicio[4]) == 'a' &&
        tolower((unsigned char)inicio[5]) == 'r') {
        return 1;
    }

    return 0;
}

void exibirAtalhoRetornoMenu(void) {
    printf("Digite MENU (ou VOLTAR) a qualquer momento para retornar ao menu principal.\n\n");
}

void retornarMenuPrincipal(void) {
    printf("\n0 - Retornar ao menu principal\n");
    lerInteiroFaixa("Escolha uma opcao: ", 0, 0);
}

int confirmarContinuar(const char *textoContinuar) {
    printf("Opcao selecionada: %s\n\n", textoContinuar);
    printf("1 - Continuar nesta opcao\n");
    printf("0 - Retornar ao menu principal\n\n");

    return lerInteiroFaixa("Escolha uma opcao: ", 0, 1);
}

int menuPrincipal(void) {
    limparTela();
    printf("============================================================\n");
    printf(" SIMULADOR DE CARTEIRA DE INVESTIMENTOS E REBALANCEAMENTO\n");
    printf("============================================================\n");
    printf("1 - Informar ganho mensal e ver plano ideal\n");
    printf("2 - Definir pesos percentuais da carteira\n");
    printf("3 - Informar valores que ja tenho investidos\n");
    printf("4 - Ver resumo, valor para investir e rebalanceamento\n");
    printf("5 - Ajuda para leigos: tipos de investimento\n");
    printf("0 - Sair\n");
    printf("============================================================\n");

    return lerInteiroFaixa("Escolha uma opcao: ", 0, 5);
}

int lerInteiroFaixa(const char *mensagem, int minimo, int maximo) {
    char linha[TAM_LINHA];
    char *fim;
    long valor;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        if (atalhoRetornoMenuAtivo && linhaSolicitaRetornoMenu(linha)) {
            retornoMenuSolicitado = 1;
            return minimo;
        }

        errno = 0;
        valor = strtol(linha, &fim, 10);

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        if (fim == linha || *fim != '\0' || errno == ERANGE) {
            printf("Entrada invalida. Digite apenas numeros inteiros.\n");
            continue;
        }

        if (valor < minimo || valor > maximo) {
            printf("Entrada invalida. Digite um valor entre %d e %d.\n", minimo, maximo);
            continue;
        }

        return (int)valor;
    }
}

double lerDoubleMinimo(const char *mensagem, double minimo, int permiteIgual) {
    char linha[TAM_LINHA];
    char *fim;
    double valor;
    int valorValido;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        if (atalhoRetornoMenuAtivo && linhaSolicitaRetornoMenu(linha)) {
            retornoMenuSolicitado = 1;
            return minimo;
        }

        errno = 0;
        valor = strtod(linha, &fim);

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        valorValido = permiteIgual ? (valor >= minimo) : (valor > minimo);

        if (fim == linha || *fim != '\0' || errno == ERANGE || !valorValido) {
            if (permiteIgual) {
                printf("Entrada invalida. Digite um numero maior ou igual a %.2f.\n", minimo);
            } else {
                printf("Entrada invalida. Digite um numero maior que %.2f.\n", minimo);
            }
            continue;
        }

        return valor;
    }
}

int lerTextoObrigatorio(const char *mensagem, char *destino, size_t tamanho) {
    char linha[TAM_LINHA];
    char *inicio;
    char *fim;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        if (atalhoRetornoMenuAtivo && linhaSolicitaRetornoMenu(linha)) {
            retornoMenuSolicitado = 1;
            return 0;
        }

        linha[strcspn(linha, "\n")] = '\0';
        inicio = linha;

        while (isspace((unsigned char)*inicio)) {
            inicio++;
        }

        fim = inicio + strlen(inicio);
        while (fim > inicio && isspace((unsigned char)*(fim - 1))) {
            fim--;
        }
        *fim = '\0';

        if (strlen(inicio) == 0) {
            printf("Entrada invalida. O nome nao pode ficar vazio.\n");
            continue;
        }

        if (tamanho > 0) {
            strncpy(destino, inicio, tamanho - 1);
            destino[tamanho - 1] = '\0';
        }

        return 1;
    }
}

void inicializarCarteira(Carteira *carteira) {
    const char nomesPadrao[MAX_ATIVOS][80] = {
        "Reserva de emergencia (Tesouro Selic/CDB liquidez diaria)",
        "Renda fixa conservadora (CDB/LCI/LCA/Tesouro Selic)",
        "Tesouro IPCA+ (protege contra inflacao)",
        "Fundos imobiliarios - FIIs (renda mensal)",
        "ETFs de acoes (diversificacao na bolsa)",
        "Caixa para objetivos curtos e oportunidades"
    };
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        strcpy(carteira->nomes[i], nomesPadrao[i]);
        carteira->precos[i] = 1.0;
        carteira->quantidades[i] = 0.0;
        carteira->valoresAtuais[i] = 0.0;
        carteira->pesos[i] = 0.0;
    }

    carteira->preenchida = 0;
}

void calcularOrcamentoIdeal(Orcamento *orcamento) {
    int i;

    orcamento->totalGastos = 0.0;

    for (i = 0; i < MAX_GASTOS; i++) {
        orcamento->gastos[i] = orcamento->rendaMensal * PERCENTUAIS_GASTOS_IDEAIS[i] / 100.0;
        orcamento->totalGastos += orcamento->gastos[i];
    }

    orcamento->sobraInvestir = orcamento->rendaMensal * PERCENTUAL_INVESTIMENTOS / 100.0;
}

void cadastrarOrcamento(Orcamento *orcamento, const char categorias[MAX_GASTOS][30]) {
    int i;

    limparTela();
    printf("================ ORCAMENTO MENSAL ================\n");

    if (confirmarContinuar("Informar nome, idade, renda mensal e plano ideal") == 0) {
        return;
    }

    limparTela();
    printf("================ ORCAMENTO MENSAL ================\n");
    printf("Informe seu nome, idade e renda. Use ponto para centavos.\n");
    printf("O plano sera calculado pela regra 50-30-20.\n");
    printf("Exemplo: 2500.50\n\n");
    exibirAtalhoRetornoMenu();

    ativarAtalhoRetornoMenu();

    if (!lerTextoObrigatorio("Nome do usuario: ", orcamento->nome, sizeof(orcamento->nome))) {
        desativarAtalhoRetornoMenu();
        return;
    }

    orcamento->idade = lerInteiroFaixa("Idade do usuario: ", 1, 120);
    if (consumirRetornoMenuSolicitado()) {
        desativarAtalhoRetornoMenu();
        return;
    }

    orcamento->rendaMensal = lerDoubleMinimo("Ganho mensal total: R$ ", 0.0, 0);
    if (consumirRetornoMenuSolicitado()) {
        desativarAtalhoRetornoMenu();
        return;
    }

    desativarAtalhoRetornoMenu();
    calcularOrcamentoIdeal(orcamento);
    orcamento->preenchido = 1;

    printf("\nUsuario: %s, %d anos\n", orcamento->nome, orcamento->idade);
    printf("Renda mensal: R$ %.2f\n\n", orcamento->rendaMensal);

    printf("Regra usada: %.0f%% necessidades, %.0f%% lazer/estilo de vida e %.0f%% investimentos.\n\n",
           PERCENTUAL_NECESSIDADES,
           PERCENTUAL_LAZER_ESTILO,
           PERCENTUAL_INVESTIMENTOS);

    printf("Plano ideal de uso da renda mensal:\n");
    printf("%-15s %12s %15s\n", "Area", "% da renda", "Valor ideal");
    printf("------------------------------------------------\n");

    for (i = 0; i < MAX_GASTOS; i++) {
        printf("%-15s %11.2f%% R$ %11.2f\n",
               categorias[i],
               PERCENTUAIS_GASTOS_IDEAIS[i],
               orcamento->gastos[i]);
    }

    printf("%-15s %11.2f%% R$ %11.2f\n",
           "Investimentos",
           PERCENTUAL_INVESTIMENTOS,
           orcamento->sobraInvestir);
    printf("------------------------------------------------\n");
    printf("Total ideal para gastos:       R$ %.2f\n", orcamento->totalGastos);
    printf("Valor ideal para investimentos: R$ %.2f\n", orcamento->sobraInvestir);

    retornarMenuPrincipal();
}

void escolherPesos(Carteira *carteira) {
    int i;
    int perfil;

    limparTela();
    printf("=============== PESOS DA CARTEIRA ===============\n");

    if (confirmarContinuar("Definir pesos percentuais da carteira") == 0) {
        return;
    }

    limparTela();
    printf("=============== PESOS DA CARTEIRA ===============\n");
    printf("Agora vamos descobrir automaticamente seu perfil de investidor.\n");
    printf("Responda ao questionario para receber uma sugestao de distribuicao.\n\n");
    exibirAtalhoRetornoMenu();

    ativarAtalhoRetornoMenu();
    perfil = identificarPerfilQuestionario();

    if (perfil == 0) {
        desativarAtalhoRetornoMenu();
        return;
    }

    definirPerfilAutomatico(carteira, perfil);
    desativarAtalhoRetornoMenu();
    carteira->preenchida = 1;

    printf("\nPerfil identificado: %s.\n", nomePerfilInvestidor(perfil));

    if (perfil == 1) {
        printf("Seu foco principal e seguranca e estabilidade.\n");
    } else if (perfil == 2) {
        printf("Voce aceita algum risco para buscar retorno maior com equilibrio.\n");
    } else {
        printf("Voce tolera mais oscilacao para buscar crescimento no longo prazo.\n");
    }

    printf("\nSugestao de distribuicao para seus investimentos:\n");
    for (i = 0; i < MAX_ATIVOS; i++) {
        printf("- %-58s %.2f%%\n", carteira->nomes[i], carteira->pesos[i]);
    }

    retornarMenuPrincipal();
}

int identificarPerfilQuestionario(void) {
    int pontuacao = 0;
    int resposta;

    printf("Questionario rapido (5 perguntas):\n");
    printf("Responda com 1, 2 ou 3 em cada pergunta.\n\n");

    printf("1) Qual seu objetivo principal ao investir?\n");
    printf("1 - Preservar meu dinheiro, com baixo risco\n");
    printf("2 - Equilibrar seguranca e crescimento\n");
    printf("3 - Buscar maior crescimento, aceitando oscilacoes\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n2) Por quanto tempo voce pretende deixar o dinheiro investido?\n");
    printf("1 - Menos de 2 anos\n");
    printf("2 - Entre 2 e 5 anos\n");
    printf("3 - Mais de 5 anos\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n3) Se sua carteira cair 10%% em alguns meses, o que voce faria?\n");
    printf("1 - Resgataria para evitar mais perdas\n");
    printf("2 - Manteria e aguardaria recuperacao\n");
    printf("3 - Aproveitaria para investir mais\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n4) Qual seu nivel de experiencia com investimentos?\n");
    printf("1 - Iniciante\n");
    printf("2 - Intermediario\n");
    printf("3 - Avancado\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n5) Quanto de oscilacao voce aceita para buscar mais retorno?\n");
    printf("1 - Quase nenhuma oscilacao\n");
    printf("2 - Oscilacao moderada\n");
    printf("3 - Oscilacao alta\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    if (pontuacao <= 8) {
        return 1;
    }

    if (pontuacao <= 11) {
        return 2;
    }

    return 3;
}

const char *nomePerfilInvestidor(int perfil) {
    if (perfil == 1) {
        return "Conservador";
    }

    if (perfil == 2) {
        return "Moderado";
    }

    return "Arrojado";
}

void definirPerfilAutomatico(Carteira *carteira, int perfil) {
    double conservador[MAX_ATIVOS] = {35, 35, 15, 5, 5, 5};
    double moderado[MAX_ATIVOS] = {25, 25, 20, 10, 15, 5};
    double arrojado[MAX_ATIVOS] = {20, 15, 20, 15, 25, 5};
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        if (perfil == 1) {
            carteira->pesos[i] = conservador[i];
        } else if (perfil == 2) {
            carteira->pesos[i] = moderado[i];
        } else {
            carteira->pesos[i] = arrojado[i];
        }
    }
}

double somarPesos(const Carteira *carteira) {
    double soma = 0.0;
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        soma += carteira->pesos[i];
    }

    return soma;
}

void cadastrarCarteiraAtual(Carteira *carteira) {
    int i;
    double valoresOriginais[MAX_ATIVOS];
    double precosOriginais[MAX_ATIVOS];
    double quantidadesOriginais[MAX_ATIVOS];
    double totalAtual = 0.0;

    limparTela();
    printf("=============== CARTEIRA ATUAL ===============\n");

    if (confirmarContinuar("Informar valores que ja tenho investidos") == 0) {
        return;
    }

    limparTela();
    printf("=============== CARTEIRA ATUAL ===============\n");
    printf("Nesta parte, informe quanto dinheiro voce JA TEM em cada tipo de investimento.\n");
    printf("Nao precisa saber preco de cota, quantidade ou termos de corretora.\n");
    printf("Digite apenas o valor total em reais.\n\n");
    printf("Exemplos:\n");
    printf("- Tenho R$ 500 em Tesouro Selic: digite 500\n");
    printf("- Tenho R$ 120 em FIIs: digite 120\n");
    printf("- Ainda nao tenho esse investimento: digite 0\n\n");
    exibirAtalhoRetornoMenu();

    ativarAtalhoRetornoMenu();

    for (i = 0; i < MAX_ATIVOS; i++) {
        valoresOriginais[i] = carteira->valoresAtuais[i];
        precosOriginais[i] = carteira->precos[i];
        quantidadesOriginais[i] = carteira->quantidades[i];
    }

    for (i = 0; i < MAX_ATIVOS; i++) {
        char mensagem[170];

        snprintf(mensagem, sizeof(mensagem), "Quanto voce ja tem em %s? R$ ", carteira->nomes[i]);
        carteira->valoresAtuais[i] = lerDoubleMinimo(mensagem, 0.0, 1);
        if (consumirRetornoMenuSolicitado()) {
            for (i = 0; i < MAX_ATIVOS; i++) {
                carteira->valoresAtuais[i] = valoresOriginais[i];
                carteira->precos[i] = precosOriginais[i];
                carteira->quantidades[i] = quantidadesOriginais[i];
            }
            desativarAtalhoRetornoMenu();
            return;
        }

        carteira->precos[i] = carteira->valoresAtuais[i];
        carteira->quantidades[i] = carteira->valoresAtuais[i] > 0.0 ? 1.0 : 0.0;
        totalAtual += carteira->valoresAtuais[i];
    }

    desativarAtalhoRetornoMenu();
    printf("\nCarteira atual cadastrada com sucesso.\n");
    printf("Total que voce ja possui investido: R$ %.2f\n", totalAtual);

    if (totalAtual == 0.0) {
        printf("Tudo bem se voce ainda nao tem investimentos. O simulador vai usar o valor mensal ideal para sugerir os primeiros aportes.\n");
    } else {
        printf("Esses valores serao comparados com os percentuais da opcao 2 para mostrar o rebalanceamento.\n");
    }

    retornarMenuPrincipal();
}

void exibirResumo(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]) {
    int i;

    limparTela();
    printf("===================== RESUMO GERAL =====================\n");

    if (confirmarContinuar("Ver resumo e rebalanceamento") == 0) {
        return;
    }

    limparTela();
    printf("===================== RESUMO GERAL =====================\n");

    if (!orcamento->preenchido) {
        printf("Orcamento ainda nao cadastrado. Use a opcao 1 primeiro.\n");
        retornarMenuPrincipal();
        return;
    }

    printf("Usuario: %s\n", orcamento->nome);
    printf("Idade:          %d anos\n", orcamento->idade);
    printf("Renda mensal:       R$ %.2f\n", orcamento->rendaMensal);
    printf("Total ideal para gastos: R$ %.2f\n", orcamento->totalGastos);
    printf("Valor ideal para investir:");

    if (orcamento->sobraInvestir > 0.0) {
        printf(" R$ %.2f (%.2f%% da renda)\n\n", orcamento->sobraInvestir, PERCENTUAL_INVESTIMENTOS);
    } else {
        printf(" R$ %.2f\n\n", orcamento->sobraInvestir);
        printf("Como nao houve valor positivo para investir, revise os percentuais do orcamento.\n\n");
    }

    printf("Regra usada: %.0f%% necessidades, %.0f%% lazer/estilo de vida e %.0f%% investimentos.\n\n",
           PERCENTUAL_NECESSIDADES,
           PERCENTUAL_LAZER_ESTILO,
           PERCENTUAL_INVESTIMENTOS);

    printf("Plano ideal de gastos por area:\n");
    for (i = 0; i < MAX_GASTOS; i++) {
        printf("- %-12s R$ %10.2f (%6.2f%% da renda)\n",
               categorias[i],
               orcamento->gastos[i],
               PERCENTUAIS_GASTOS_IDEAIS[i]);
    }

    if (orcamento->sobraInvestir <= 0.0) {
        retornarMenuPrincipal();
        return;
    }

    if (!carteira->preenchida || valorAbsoluto(somarPesos(carteira) - 100.0) > 0.01) {
        printf("\nPesos da carteira ainda nao cadastrados. Use a opcao 2.\n");
        retornarMenuPrincipal();
        return;
    }

    printf("\nSugestao simples para investir o valor mensal ideal:\n");
    for (i = 0; i < MAX_ATIVOS; i++) {
        double aporte = orcamento->sobraInvestir * carteira->pesos[i] / 100.0;
        printf("- %-58s R$ %10.2f (%5.2f%%)\n", carteira->nomes[i], aporte, carteira->pesos[i]);
    }

    simularRebalanceamento(orcamento, carteira);
    retornarMenuPrincipal();
}

void simularRebalanceamento(const Orcamento *orcamento, const Carteira *carteira) {
    double totalAtual = 0.0;
    double totalProjetado;
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        totalAtual += carteira->valoresAtuais[i];
    }

    if (totalAtual <= 0.0) {
        printf("\nVoce ainda nao cadastrou valores atuais na carteira.\n");
        printf("Mesmo assim, a divisao acima ja mostra quanto investir em cada item por mes.\n");
        return;
    }

    totalProjetado = totalAtual + orcamento->sobraInvestir;

    printf("\n================ REBALANCEAMENTO ================\n");
    printf("Carteira atual: R$ %.2f\n", totalAtual);
    printf("Carteira depois do novo aporte: R$ %.2f\n\n", totalProjetado);
    printf("%-58s %12s %12s %18s\n", "Investimento", "Atual", "Ideal", "Acao sugerida");
    printf("------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < MAX_ATIVOS; i++) {
        double valorIdeal = totalProjetado * carteira->pesos[i] / 100.0;
        double diferenca = valorIdeal - carteira->valoresAtuais[i];

        printf("%-58s R$ %9.2f R$ %9.2f ", carteira->nomes[i], carteira->valoresAtuais[i], valorIdeal);

        if (diferenca > 0.01) {
            printf("Aportar R$ %.2f\n", diferenca);
        } else if (diferenca < -0.01) {
            printf("Pausar aporte. Excesso de R$ %.2f\n", valorAbsoluto(diferenca));
        } else {
            printf("Manter\n");
        }
    }

    printf("\nLeitura para leigos:\n");
    printf("- Aportar significa colocar mais dinheiro no item que esta abaixo da meta.\n");
    printf("- Pausar aporte significa que o item esta acima da meta. Iniciantes podem apenas nao colocar dinheiro nele por enquanto.\n");
    printf("- Nunca invista dinheiro da reserva de emergencia em produtos de alto risco.\n");
}

void exibirAjudaInvestimentos(void) {
    limparTela();
    printf("================ AJUDA PARA LEIGOS ================\n");

    if (confirmarContinuar("Ver ajuda sobre tipos de investimento") == 0) {
        return;
    }

    limparTela();
    printf("================ AJUDA PARA LEIGOS ================\n");
    printf("Este programa nao substitui um profissional, mas ajuda a organizar ideias.\n\n");

    printf("1. Reserva de emergencia\n");
    printf("   Primeiro passo. Deve ficar em algo seguro e com liquidez diaria.\n");
    printf("   Exemplos: Tesouro Selic, CDB com liquidez diaria e fundo DI simples.\n");
    printf("   Objetivo: cobrir de 3 a 6 meses de gastos essenciais.\n\n");

    printf("2. Renda fixa conservadora\n");
    printf("   Boa para quem esta comecando e quer previsibilidade.\n");
    printf("   Exemplos: CDB, LCI, LCA e Tesouro Selic. Compare rentabilidade, prazo e garantia do FGC.\n\n");

    printf("3. Tesouro IPCA+\n");
    printf("   Ajuda a proteger o dinheiro da inflacao no longo prazo.\n");
    printf("   Pode oscilar antes do vencimento, entao combina melhor com objetivos distantes.\n\n");

    printf("4. Fundos imobiliarios (FIIs)\n");
    printf("   Sao negociados na bolsa e podem pagar rendimentos mensais.\n");
    printf("   Possuem risco de mercado, vacancia, gestao e mudanca nos rendimentos.\n\n");

    printf("5. ETFs de acoes\n");
    printf("   Sao fundos negociados em bolsa que compram varias acoes de uma vez.\n");
    printf("   Servem para diversificar sem escolher empresa por empresa, mas oscilam bastante.\n\n");

    printf("Regra simples para iniciantes:\n");
    printf("- Quite dividas caras antes de investir.\n");
    printf("- Monte reserva de emergencia antes de correr risco.\n");
    printf("- Invista todo mes, mesmo pouco.\n");
    printf("- Rebalanceie quando um investimento ficar muito acima ou abaixo da meta.\n");
    printf("- Desconfie de promessa de ganho rapido e garantido.\n");

    retornarMenuPrincipal();
}

double valorAbsoluto(double valor) {
    if (valor < 0.0) {
        return -valor;
    }

    return valor;
}
