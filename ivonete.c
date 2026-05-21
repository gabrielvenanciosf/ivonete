#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Trabalho Academico em Linguagem C
    Tema: Simulador de Organizacao Financeira, Investimentos e Rebalanceamento

    Requisitos tecnicos atendidos:
    1. Estruturas Condicionais: if/else e switch.7

    2. Estruturas de Repeticao: for, while e do/while.
    3. Modularizacao: funcoes separadas para menu, cadastro, calculos e relatorios.
    4. Estruturas de Dados: vetores e matrizes para nomes, gastos, valores e metas.
*/

#define TAM_NOME 50
#define MAX_ATIVOS 10
#define QTD_GASTOS 7
#define EPSILON 0.01

typedef struct {
    char nome[TAM_NOME];
    int idade;
    double salario;
    double gastos[QTD_GASTOS];
    double economiaMensal;
    int configurado;
} Usuario;

typedef struct { 
    int quantidade;
    char nomes[MAX_ATIVOS][TAM_NOME];
    double valores[MAX_ATIVOS];
    double metas[MAX_ATIVOS];
    int configurada;
} Carteira;

const char categoriasGastos[QTD_GASTOS][TAM_NOME] = {
    "Moradia",
    "Alimentacao",
    "Transporte",
    "Saude",
    "Educacao",
    "Lazer",
    "Outros"
};

void limparTela(void);
void pausar(void);
void removerQuebraLinha(char texto[]);
void trocarVirgulaPorPonto(char texto[]);
int lerInteiro(const char mensagem[], int minimo, int maximo);
double lerDouble(const char mensagem[], double minimo);
void lerTexto(const char mensagem[], char texto[], int tamanho);

int menuPrincipal(void);
void carregarExemplo(Usuario *usuario, Carteira *carteira);
void cadastrarUsuario(Usuario *usuario);
void cadastrarCarteira(Carteira *carteira);
void exibirDiagnosticoFinanceiro(const Usuario *usuario, const Carteira *carteira);
void exibirRelatorioRebalanceamento(const Carteira *carteira);
void simularQueda(Carteira *carteira);
void exibirRequisitosAcademicos(void);

double calcularTotalGastos(const Usuario *usuario);
double calcularSaldoMensal(const Usuario *usuario);
double calcularTotalCarteira(const Carteira *carteira);
void calcularRebalanceamento(const Carteira *carteira, double diferencas[]);
int metasSomam100(const Carteira *carteira);
int escolherAtivo(const Carteira *carteira);

int main(void) {
    Usuario usuario = {0};
    Carteira carteira = {0};
    int opcao;

    carregarExemplo(&usuario, &carteira);

    do {
        limparTela();
        opcao = menuPrincipal();

        switch (opcao) {
            case 1:
                cadastrarUsuario(&usuario);
                break;
            case 2:
                cadastrarCarteira(&carteira);
                break;
            case 3:
                exibirDiagnosticoFinanceiro(&usuario, &carteira);
                break;
            case 4:
                exibirRelatorioRebalanceamento(&carteira);
                break;
            case 5:
                simularQueda(&carteira);
                break;
            case 6:
                carregarExemplo(&usuario, &carteira);
                printf("\nDados de exemplo carregados com sucesso.\n");
                pausar();
                break;
            case 7:
                exibirRequisitosAcademicos();
                break;
            case 0:
                printf("\nPrograma encerrado. Organize seu dinheiro antes de investir!\n");
                break;
            default:
                printf("\nOpcao invalida.\n");
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
    char entrada[8];

    printf("\nPressione ENTER para continuar...");
    fgets(entrada, sizeof(entrada), stdin);
}

void removerQuebraLinha(char texto[]) {
    texto[strcspn(texto, "\n")] = '\0';
}

void trocarVirgulaPorPonto(char texto[]) {
    int i;

    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] == ',') {
            texto[i] = '.';
        }
    }
}

int lerInteiro(const char mensagem[], int minimo, int maximo) {
    char entrada[80];
    char extra;
    int numero;

    while (1) {
        printf("%s", mensagem);

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        if (sscanf(entrada, " %d %c", &numero, &extra) != 1) {
            printf("Entrada invalida. Digite apenas numeros inteiros.\n");
            continue;
        }

        if (numero < minimo || numero > maximo) {
            printf("Valor fora do intervalo permitido (%d a %d).\n", minimo, maximo);
            continue;
        }

        return numero;
    }
}

double lerDouble(const char mensagem[], double minimo) {
    char entrada[80];
    char *fim;
    double numero;

    while (1) {
        printf("%s", mensagem);

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            continue;
        }

        trocarVirgulaPorPonto(entrada);
        numero = strtod(entrada, &fim);

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        if (fim == entrada || *fim != '\0') {
            printf("Entrada invalida. Digite um numero valido.\n");
            continue;
        }

        if (numero < minimo) {
            printf("Valor invalido. O minimo permitido e %.2f.\n", minimo);
            continue;
        }

        return numero;
    }
}

void lerTexto(const char mensagem[], char texto[], int tamanho) {
    do {
        printf("%s", mensagem);

        if (fgets(texto, tamanho, stdin) == NULL) {
            printf("Entrada invalida. Tente novamente.\n");
            clearerr(stdin);
            texto[0] = '\0';
            continue;
        }

        removerQuebraLinha(texto);

        if (strlen(texto) == 0) {
            printf("O campo nao pode ficar vazio.\n");
        }
    } while (strlen(texto) == 0);
}

int menuPrincipal(void) {
    printf("============================================================\n");
    printf("       SIMULADOR FINANCEIRO E DE INVESTIMENTOS\n");
    printf("============================================================\n");
    printf("1 - Cadastrar dados pessoais e gastos mensais\n");
    printf("2 - Cadastrar carteira de investimentos e metas\n");
    printf("3 - Ver diagnostico financeiro mensal\n");
    printf("4 - Gerar relatorio de rebalanceamento\n");
    printf("5 - Simular queda de um investimento\n"); 
    printf("6 - Carregar exemplo para demonstracao\n");
    printf("7 - Ver requisitos academicos usados\n");
    printf("0 - Sair\n");
    printf("============================================================\n");

    return lerInteiro("Escolha uma opcao: ", 0, 7);
}

void carregarExemplo(Usuario *usuario, Carteira *carteira) {
    strcpy(usuario->nome, "Aluno Visitante");
    usuario->idade = 20;
    usuario->salario = 3000.00;
    usuario->gastos[0] = 900.00;
    usuario->gastos[1] = 600.00;
    usuario->gastos[2] = 250.00;
    usuario->gastos[3] = 150.00;
    usuario->gastos[4] = 200.00;
    usuario->gastos[5] = 200.00;
    usuario->gastos[6] = 200.00;
    usuario->economiaMensal = 500.00;
    usuario->configurado = 1;

    carteira->quantidade = 4;

    strcpy(carteira->nomes[0], "Acoes");
    strcpy(carteira->nomes[1], "Renda Fixa");
    strcpy(carteira->nomes[2], "Fundos");
    strcpy(carteira->nomes[3], "Cripto");

    carteira->valores[0] = 4000.00;
    carteira->valores[1] = 3000.00;
    carteira->valores[2] = 2000.00;
    carteira->valores[3] = 1000.00;

    carteira->metas[0] = 40.00;
    carteira->metas[1] = 30.00;
    carteira->metas[2] = 20.00;
    carteira->metas[3] = 10.00;

    carteira->configurada = 1;
}

void cadastrarUsuario(Usuario *usuario) {
    int i;

    limparTela();
    printf("============================================================\n");
    printf("             CADASTRO FINANCEIRO DO USUARIO\n");
    printf("============================================================\n");

    lerTexto("Nome: ", usuario->nome, TAM_NOME);
    usuario->idade = lerInteiro("Idade: ", 1, 120);
    usuario->salario = lerDouble("Quanto recebe de salario por mes (R$): ", 0.0);

    printf("\nInforme seus gastos mensais por area da vida.\n");

    for (i = 0; i < QTD_GASTOS; i++) {
        char mensagem[100];
        sprintf(mensagem, "Gasto com %s (R$): ", categoriasGastos[i]);
        usuario->gastos[i] = lerDouble(mensagem, 0.0);
    }

    usuario->economiaMensal = lerDouble("\nQuanto consegue economizar para investir por mes (R$): ", 0.0);

    while (usuario->economiaMensal > usuario->salario) {
        printf("A economia mensal nao pode ser maior que o salario.\n");
        usuario->economiaMensal = lerDouble("Digite novamente a economia mensal (R$): ", 0.0);
    }

    usuario->configurado = 1;

    printf("\nDados mensais cadastrados com sucesso.\n");
    pausar();
}

void cadastrarCarteira(Carteira *carteira) {
    int i;
    double somaMetas;

    limparTela();
    printf("============================================================\n");
    printf("          CADASTRO DA CARTEIRA DE INVESTIMENTOS\n");
    printf("============================================================\n");

    carteira->quantidade = lerInteiro("Quantas categorias de investimento deseja cadastrar? (2 a 10): ", 2, MAX_ATIVOS);

    do {
        somaMetas = 0.0;

        for (i = 0; i < carteira->quantidade; i++) {
            printf("\nInvestimento %d\n", i + 1);

            lerTexto("Nome da categoria (ex: Acoes, Renda Fixa): ", carteira->nomes[i], TAM_NOME);
            carteira->valores[i] = lerDouble("Valor atual investido nessa categoria (R$): ", 0.0);
            carteira->metas[i] = lerDouble("Meta de alocacao para essa categoria (%): ", 0.0);

            somaMetas += carteira->metas[i];
        }

        if (somaMetas < 100.0 - EPSILON || somaMetas > 100.0 + EPSILON) {
            printf("\nErro: as metas somaram %.2f%%, mas precisam somar exatamente 100%%.\n", somaMetas);
            printf("Cadastre novamente as categorias da carteira.\n");
            pausar();
            limparTela();
        }
    } while (somaMetas < 100.0 - EPSILON || somaMetas > 100.0 + EPSILON);

    carteira->configurada = 1;

    printf("\nCarteira cadastrada com sucesso.\n");
    pausar();
}

double calcularTotalGastos(const Usuario *usuario) {
    int i;
    double total = 0.0;

    for (i = 0; i < QTD_GASTOS; i++) {
        total += usuario->gastos[i];
    }

    return total;
}

double calcularSaldoMensal(const Usuario *usuario) {
    return usuario->salario - calcularTotalGastos(usuario);
}

double calcularTotalCarteira(const Carteira *carteira) {
    int i;
    double total = 0.0;

    for (i = 0; i < carteira->quantidade; i++) {
        total += carteira->valores[i];
    }

    return total;
}

void exibirDiagnosticoFinanceiro(const Usuario *usuario, const Carteira *carteira) {
    int i;
    double totalGastos;
    double saldo;
    double percentualGastos;
    double totalCarteira;

    limparTela();

    if (!usuario->configurado) {
        printf("Cadastre primeiro os dados pessoais e gastos mensais.\n");
        pausar();
        return;
    }

    totalGastos = calcularTotalGastos(usuario);
    saldo = calcularSaldoMensal(usuario);
    percentualGastos = usuario->salario > 0.0 ? (totalGastos / usuario->salario) * 100.0 : 0.0;

    printf("============================================================\n");
    printf("              DIAGNOSTICO FINANCEIRO MENSAL\n");
    printf("============================================================\n");
    printf("Nome: %s\n", usuario->nome);
    printf("Idade: %d anos\n", usuario->idade);
    printf("Salario mensal: R$ %.2f\n", usuario->salario);
    printf("Total de gastos: R$ %.2f\n", totalGastos);
    printf("Saldo apos gastos: R$ %.2f\n", saldo);
    printf("Economia informada para investir: R$ %.2f\n", usuario->economiaMensal);
    printf("Percentual do salario comprometido com gastos: %.2f%%\n", percentualGastos);

    printf("\nGastos por area:\n");
    printf("------------------------------------------------------------\n");

    for (i = 0; i < QTD_GASTOS; i++) {
        printf("%-15s R$ %10.2f\n", categoriasGastos[i], usuario->gastos[i]);
    }

    printf("------------------------------------------------------------\n");

    if (saldo < 0.0) {
        printf("ALERTA: seus gastos estao maiores que sua renda.\n");
        printf("Sugestao: reduza gastos antes de investir.\n");
    } else if (usuario->economiaMensal > saldo + EPSILON) {
        printf("ALERTA: voce informou que investe mais do que sobra no mes.\n");
        printf("Sugestao: revise os gastos ou reduza o valor destinado a investimentos.\n");
    } else if (usuario->economiaMensal <= EPSILON) {
        printf("Sugestao: comece criando uma reserva mensal, mesmo que pequena.\n");
    } else {
        printf("Situacao positiva: existe dinheiro mensal disponivel para investir.\n");
    }

    if (carteira->configurada && metasSomam100(carteira) && usuario->economiaMensal > EPSILON) {
        printf("\nSugestao de aplicacao da economia mensal conforme sua meta:\n");

        for (i = 0; i < carteira->quantidade; i++) {
            double valorSugerido = usuario->economiaMensal * (carteira->metas[i] / 100.0);
            printf("%-15s R$ %10.2f\n", carteira->nomes[i], valorSugerido);
        }
    }

    if (carteira->configurada) {
        totalCarteira = calcularTotalCarteira(carteira);
        printf("\nPatrimonio atual investido: R$ %.2f\n", totalCarteira);
    }

    pausar();
}

void calcularRebalanceamento(const Carteira *carteira, double diferencas[]) {
    int i;
    double total = calcularTotalCarteira(carteira);
    double valorIdeal;

    for (i = 0; i < carteira->quantidade; i++) {
        valorIdeal = total * (carteira->metas[i] / 100.0);
        diferencas[i] = valorIdeal - carteira->valores[i];
    }
}

void exibirRelatorioRebalanceamento(const Carteira *carteira) {
    int i;
    int houveDesvio = 0;
    double total;
    double percentualAtual;
    double desvio;
    double diferencas[MAX_ATIVOS];

    limparTela();

    if (!carteira->configurada) {
        printf("Cadastre primeiro a carteira de investimentos.\n");
        pausar();
        return;
    }

    if (!metasSomam100(carteira)) {
        printf("Erro: as metas cadastradas nao somam 100%%.\n");
        pausar();
        return;
    }

    total = calcularTotalCarteira(carteira);

    if (total <= 0.0) {
        printf("Nao ha dinheiro investido na carteira para rebalancear.\n");
        pausar();
        return;
    }

    calcularRebalanceamento(carteira, diferencas);

    printf("==========================================================================\n");
    printf("                  RELATORIO DE REBALANCEAMENTO\n");
    printf("==========================================================================\n");
    printf("Patrimonio total investido: R$ %.2f\n\n", total);
    printf("%-20s %12s %12s %12s %18s\n",
           "Categoria", "Atual %", "Meta %", "Desvio", "Acao sugerida");
    printf("--------------------------------------------------------------------------\n");

    for (i = 0; i < carteira->quantidade; i++) {
        percentualAtual = (carteira->valores[i] / total) * 100.0;
        desvio = percentualAtual - carteira->metas[i];

        printf("%-20s %11.2f%% %11.2f%% %+11.2f%% ",
               carteira->nomes[i],
               percentualAtual,
               carteira->metas[i],
               desvio);

        if (diferencas[i] > EPSILON) {
            printf("COMPRAR R$ %9.2f\n", diferencas[i]);
            houveDesvio = 1;
        } else if (diferencas[i] < -EPSILON) {
            printf("VENDER  R$ %9.2f\n", -diferencas[i]);
            houveDesvio = 1;
        } else {
            printf("MANTER\n");
        }
    }

    printf("--------------------------------------------------------------------------\n");

    if (houveDesvio) {
        printf("Alerta de desvio! Execute as compras e vendas acima para voltar a meta.\n");
    } else {
        printf("Carteira equilibrada. Nenhum rebalanceamento necessario.\n");
    }

    pausar();
}

void simularQueda(Carteira *carteira) {
    int categoria;
    double queda;
    double valorAntes;

    limparTela();

    if (!carteira->configurada) {
        printf("Cadastre primeiro a carteira de investimentos.\n");
        pausar();
        return;
    }

    printf("============================================================\n");
    printf("              SIMULACAO DE QUEDA DE MERCADO\n");
    printf("============================================================\n");

    categoria = escolherAtivo(carteira);
    queda = lerDouble("Digite o percentual de queda da categoria escolhida (%): ", 0.0);

    while (queda > 100.0) {
        printf("A queda nao pode ser maior que 100%%.\n");
        queda = lerDouble("Digite novamente o percentual de queda (%): ", 0.0);
    }

    valorAntes = carteira->valores[categoria];
    carteira->valores[categoria] = carteira->valores[categoria] * (1.0 - queda / 100.0);

    limparTela();
    printf("Simulacao aplicada: %s caiu %.2f%%.\n", carteira->nomes[categoria], queda);
    printf("Valor anterior: R$ %.2f\n", valorAntes);
    printf("Valor atual:    R$ %.2f\n\n", carteira->valores[categoria]);
    printf("Agora o programa vai gerar o relatorio de rebalanceamento.\n");

    pausar();
    exibirRelatorioRebalanceamento(carteira);
}

int escolherAtivo(const Carteira *carteira) {
    int i;
    int escolha;

    printf("\nCategorias disponiveis:\n");

    for (i = 0; i < carteira->quantidade; i++) {
        printf("%d - %s (R$ %.2f)\n", i + 1, carteira->nomes[i], carteira->valores[i]);
    }

    escolha = lerInteiro("\nEscolha a categoria: ", 1, carteira->quantidade);
    return escolha - 1;
}

int metasSomam100(const Carteira *carteira) {
    int i;
    double soma = 0.0;

    for (i = 0; i < carteira->quantidade; i++) {
        soma += carteira->metas[i];
    }

    return soma >= 100.0 - EPSILON && soma <= 100.0 + EPSILON;
}

void exibirRequisitosAcademicos(void) {
    limparTela();
    printf("============================================================\n");
    printf("             REQUISITOS TECNICOS DO TRABALHO\n");
    printf("============================================================\n");
    printf("1. Estruturas Condicionais\n");
    printf("   - if/else: usado nas validacoes, alertas e relatorios.\n");
    printf("   - switch: usado para controlar as opcoes do menu principal.\n\n");

    printf("2. Estruturas de Repeticao\n");
    printf("   - for: usado para percorrer gastos e investimentos.\n");
    printf("   - while: usado para validar entradas incorretas.\n");
    printf("   - do/while: usado para manter o menu ativo ate o usuario sair.\n\n");

    printf("3. Modularizacao com Funcoes\n");
    printf("   - menuPrincipal(), cadastrarUsuario(), cadastrarCarteira(),\n");
    
    printf("     exibirRelatorioRebalanceamento() e simularQueda().\n\n");

    printf("4. Estruturas de Dados\n");
    printf("   - vetor gastos[]: armazena gastos mensais por area da vida.\n");
    printf("   - vetor valores[]: armazena o valor atual de cada investimento.\n");
    printf("   - vetor metas[]: armazena o percentual ideal de cada investimento.\n");
    printf("   - matriz nomes[][]: armazena os nomes das categorias da carteira.\n");
    printf("============================================================\n");

    pausar();
}