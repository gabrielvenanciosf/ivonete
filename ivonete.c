#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef ENABLE_PROCESSED_OUTPUT
#define ENABLE_PROCESSED_OUTPUT 0x0001
#endif
#endif

/*
 * [SLIDE] CONFIGURACAO GERAL
 * Finalidade: definir limites fixos de arrays e buffers.
 * Ganho: evita valores "magicos" espalhados no codigo.
 */
#define MAX_GASTOS 7
#define MAX_ATIVOS 6
#define MAX_METODOS 4
#define TAM_LINHA 120

/*
 * [SLIDE] MODELOS DE DADOS
 * Finalidade: representar regras de orcamento, dados do usuario e carteira.
 * Ganho: separa claramente informacao (dados) de comportamento (funcoes).
 */
typedef struct {
    char nome[80];
    char descricao[140];
    double necessidades;
    double lazerEstilo;
    double investimentos;
    double percentuaisGastos[MAX_GASTOS];
} MetodoOrcamento;

static const MetodoOrcamento METODOS_ORCAMENTO[MAX_METODOS] = {
    {
        "70-20-10 - Organizar primeiro",
        "Indicado para quem tem renda apertada, dívidas ou reserva de emergência ainda pequena.",
        70.0,
        20.0,
        10.0,
        {35.0, 15.0, 8.0, 7.0, 5.0, 15.0, 5.0}
    },
    {
        "60-20-20 - Controle com segurança",
        "Indicado para quem quer ajustar gastos sem deixar de investir todo mês.",
        60.0,
        20.0,
        20.0,
        {30.0, 12.0, 7.0, 6.0, 5.0, 15.0, 5.0}
    },
    {
        "50-30-20 - Equilibrado",
        "Indicado para quem busca equilíbrio entre necessidades, estilo de vida e investimentos.",
        50.0,
        30.0,
        20.0,
        {25.0, 10.0, 5.0, 5.0, 5.0, 20.0, 10.0}
    },
    {
        "40-30-30 - Acelerador de objetivos",
        "Indicado para quem tem gastos controlados e quer aumentar o ritmo dos investimentos.",
        40.0,
        30.0,
        30.0,
        {20.0, 8.0, 4.0, 4.0, 4.0, 20.0, 10.0}
    }
};

/* Bloco com dados cadastrais, financeiros e resultados calculados. */
typedef struct {
    char nome[80];
    int idade;
    double rendaMensal;
    double dinheiroGuardadoInvestir;
    double gastos[MAX_GASTOS];
    double totalGastos;
    double sobraInvestir;
    int metodoOrcamento;
    int perfilInvestidor;
    int preenchido;
} Orcamento;

/* Bloco com carteira atual informada e pesos sugeridos pelo simulador. */
typedef struct {
    char nomes[MAX_ATIVOS][80];
    double precos[MAX_ATIVOS];
    double quantidades[MAX_ATIVOS];
    double valoresAtuais[MAX_ATIVOS];
    double pesos[MAX_ATIVOS];
    int preenchida;
} Carteira;

/*
 * [SLIDE] ESTADO GLOBAL
 * Finalidade: controlar atalhos de navegacao e status de cores do terminal.
 */
static int atalhoRetornoMenuAtivo = 0;
static int retornoMenuSolicitado = 0;
static int coresAtivas = 0;

/* Paleta ANSI usada para destacar mensagens no terminal. */
#define COR_RESET "\x1b[0m"
#define COR_TITULO "\x1b[1;36m"
#define COR_SECAO "\x1b[1;34m"
#define COR_SUCESSO "\x1b[1;32m"
#define COR_ALERTA "\x1b[1;33m"
#define COR_ERRO "\x1b[1;31m"
#define COR_INFO "\x1b[1;35m"
#define COR_TEXTO_PADRAO "\x1b[0;37m"

int consolePrintf(const char *formato, ...);

/* Toda saida passa por um printf customizado para Unicode + cores. */
#define printf consolePrintf

/*
 * [SLIDE] ASSINATURAS
 * Finalidade: mapa rapido das funcoes disponiveis no arquivo.
 * Leitura recomendada: de cima para baixo por responsabilidade.
 */
void limparTela(void);
void configurarConsole(void);
void pausar(void);
void retornarMenuPrincipal(void);
void ativarAtalhoRetornoMenu(void);
void desativarAtalhoRetornoMenu(void);
int consumirRetornoMenuSolicitado(void);
int linhaSolicitaRetornoMenu(const char *linha);
int linhaContemApenasZero(const char *linha);
void exibirAtalhoRetornoMenu(void);
int menuPrincipal(void);
int lerInteiroFaixa(const char *mensagem, int minimo, int maximo);
double lerDoubleMinimo(const char *mensagem, double minimo, int permiteIgual);
int lerTextoObrigatorio(const char *mensagem, char *destino, size_t tamanho);
const char *corTerminal(const char *codigo);
size_t comprimentoVisualUtf8(const char *texto);
void copiarTrechoUtf8Visivel(const char *origem, char *destino, size_t tamanhoDestino, int larguraMaxima);
void imprimirTextoPaddedUtf8(const char *texto, int largura, int alinharDireita);
void imprimirRepeticao(char caractere, int quantidade);
void imprimirBordaTabela2(int largura1, int largura2);
void imprimirLinhaTabela2(const char *coluna1, const char *coluna2, int largura1, int largura2);
void imprimirCabecalhoTabela2(const char *coluna1, const char *coluna2, int largura1, int largura2);
void inicializarCarteira(Carteira *carteira);
void calcularOrcamentoIdeal(Orcamento *orcamento);
int recomendarMetodoOrcamento(const Orcamento *orcamento);
int escolherMetodoOrcamento(int metodoRecomendado);
int identificarPerfilQuestionarioIntegrado(const Orcamento *orcamento, int *metodoRecomendado);
const MetodoOrcamento *obterMetodoOrcamento(int indice);
void exibirPlanoOrcamento(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]);
double calcularAporteDisponivelInvestimento(const Orcamento *orcamento);
void cadastrarOrcamento(Orcamento *orcamento);
void escolherPesos(Carteira *carteira, Orcamento *orcamento, const char categorias[MAX_GASTOS][30]);
const char *nomePerfilInvestidor(int perfil);
void definirPerfilAutomatico(Carteira *carteira, int perfil);
double somarPesos(const Carteira *carteira);
void cadastrarCarteiraAtual(Carteira *carteira);
void exibirResumo(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]);
void simularRebalanceamento(const Orcamento *orcamento, const Carteira *carteira);
void exibirAjudaInvestimentos(void);
double valorAbsoluto(double valor);

#ifdef _WIN32
/*
 * [SLIDE] APOIO DE ENCODING (WINDOWS)
 * Problema resolvido: texto com acento quebrado (mojibake).
 * Estrategia: converter entre code pages e priorizar exibicao correta.
 */
static int calcularPontuacaoMojibake(const wchar_t *texto) {
    int pontos = 0;
    const wchar_t *cursor = texto;

    while (cursor != NULL && *cursor != L'\0') {
        if (*cursor == L'Ã' || *cursor == L'Â' || *cursor == L'â') {
            pontos += 3;
        } else if (*cursor == L'¢' || *cursor == L'£' || *cursor == L'§' ||
                   *cursor == L'©' || *cursor == L'ª' || *cursor == L'º' ||
                   *cursor == L'«' || *cursor == L'»') {
            pontos += 1;
        }
        cursor++;
    }

    return pontos;
}

static wchar_t *converterBytesParaWide(const char *texto, UINT codePage, DWORD flags) {
    int tamanho = MultiByteToWideChar(codePage, flags, texto, -1, NULL, 0);
    wchar_t *saida;

    if (tamanho <= 0) {
        return NULL;
    }

    saida = (wchar_t *)malloc((size_t)tamanho * sizeof(wchar_t));
    if (saida == NULL) {
        return NULL;
    }

    if (MultiByteToWideChar(codePage, flags, texto, -1, saida, tamanho) <= 0) {
        free(saida);
        return NULL;
    }

    return saida;
}

static char *converterWideParaBytes(const wchar_t *texto, UINT codePage, int *tamanhoBytes) {
    int tamanho = WideCharToMultiByte(codePage, 0, texto, -1, NULL, 0, NULL, NULL);
    char *saida;

    if (tamanho <= 0) {
        return NULL;
    }

    saida = (char *)malloc((size_t)tamanho);
    if (saida == NULL) {
        return NULL;
    }

    if (WideCharToMultiByte(codePage, 0, texto, -1, saida, tamanho, NULL, NULL) <= 0) {
        free(saida);
        return NULL;
    }

    if (tamanhoBytes != NULL) {
        *tamanhoBytes = tamanho;
    }

    return saida;
}

static wchar_t *resolverTextoWideWindows(const char *texto) {
    wchar_t *textoUtf8 = converterBytesParaWide(texto, CP_UTF8, MB_ERR_INVALID_CHARS);
    wchar_t *textoAcp = converterBytesParaWide(texto, CP_ACP, 0);
    wchar_t *resultado = NULL;

    if (textoUtf8 != NULL && textoAcp != NULL) {
        int pontuacaoUtf8 = calcularPontuacaoMojibake(textoUtf8);
        int pontuacaoAcp = calcularPontuacaoMojibake(textoAcp);

        if (pontuacaoAcp < pontuacaoUtf8) {
            resultado = textoAcp;
            free(textoUtf8);
        } else {
            resultado = textoUtf8;
            free(textoAcp);
        }
    } else if (textoUtf8 != NULL) {
        resultado = textoUtf8;
    } else if (textoAcp != NULL) {
        resultado = textoAcp;
    }

    if (resultado != NULL && calcularPontuacaoMojibake(resultado) > 0) {
        char *bytesAcp;
        int tamanhoAcp = 0;
        wchar_t *corrigido = NULL;

        bytesAcp = converterWideParaBytes(resultado, CP_ACP, &tamanhoAcp);
        if (bytesAcp != NULL && tamanhoAcp > 0) {
            corrigido = converterBytesParaWide(bytesAcp, CP_UTF8, MB_ERR_INVALID_CHARS);
            free(bytesAcp);
        }

        if (corrigido != NULL && calcularPontuacaoMojibake(corrigido) < calcularPontuacaoMojibake(resultado)) {
            free(resultado);
            resultado = corrigido;
        } else if (corrigido != NULL) {
            free(corrigido);
        }
    }

    return resultado;
}
#endif

int consolePrintf(const char *formato, ...) {
    va_list argumentos;
    va_list copia;
    int tamanhoFormatado;
    int tamanhoEscrito = -1;
    int tamanhoSaida;
    char bufferLocal[4096];
    char *buffer = bufferLocal;
    char *saidaColorida = NULL;
    const char *saidaTexto;

    /*
     * [SLIDE] PIPELINE DE IMPRESSAO
     * Etapa 1: formata a mensagem.
     * Etapa 2: aplica cor padrao onde nao houver estilo explicito.
     * Etapa 3: escreve em Unicode no Windows para preservar acentos.
     */
    va_start(argumentos, formato);
    va_copy(copia, argumentos);
    tamanhoFormatado = vsnprintf(NULL, 0, formato, copia);
    va_end(copia);

    if (tamanhoFormatado < 0) {
        va_end(argumentos);
        return -1;
    }

    if (tamanhoFormatado >= (int)sizeof(bufferLocal)) {
        buffer = (char *)malloc((size_t)tamanhoFormatado + 1U);
        if (buffer == NULL) {
            va_end(argumentos);
            return -1;
        }
    }

    vsnprintf(buffer, (size_t)tamanhoFormatado + 1U, formato, argumentos);
    va_end(argumentos);
    tamanhoSaida = tamanhoFormatado;
    saidaTexto = buffer;

    if (coresAtivas && tamanhoFormatado > 0) {
        const char *padraoReset = COR_RESET;
        const char *substituicaoReset = COR_TEXTO_PADRAO;
        const char *cursor = buffer;
        const char *achou;
        size_t ocorrenciasReset = 0;
        size_t tamanhoInicio = strlen(COR_TEXTO_PADRAO);
        size_t tamanhoFim = strlen(COR_RESET);
        size_t tamanhoReset = strlen(padraoReset);
        size_t tamanhoSubstituicao = strlen(substituicaoReset);
        size_t tamanhoTotal;
        char *destino;

        while ((achou = strstr(cursor, padraoReset)) != NULL) {
            ocorrenciasReset++;
            cursor = achou + tamanhoReset;
        }

        tamanhoTotal = tamanhoInicio + (size_t)tamanhoFormatado + tamanhoFim;
        if (tamanhoSubstituicao >= tamanhoReset) {
            tamanhoTotal += ocorrenciasReset * (tamanhoSubstituicao - tamanhoReset);
        } else {
            tamanhoTotal -= ocorrenciasReset * (tamanhoReset - tamanhoSubstituicao);
        }

        saidaColorida = (char *)malloc(tamanhoTotal + 1U);
        if (saidaColorida != NULL) {
            const char *origem = buffer;

            destino = saidaColorida;
            memcpy(destino, COR_TEXTO_PADRAO, tamanhoInicio);
            destino += tamanhoInicio;

            while ((achou = strstr(origem, padraoReset)) != NULL) {
                size_t trecho = (size_t)(achou - origem);
                memcpy(destino, origem, trecho);
                destino += trecho;

                memcpy(destino, substituicaoReset, tamanhoSubstituicao);
                destino += tamanhoSubstituicao;
                origem = achou + tamanhoReset;
            }

            memcpy(destino, origem, strlen(origem));
            destino += strlen(origem);
            memcpy(destino, COR_RESET, tamanhoFim);
            destino += tamanhoFim;
            *destino = '\0';

            saidaTexto = saidaColorida;
            tamanhoSaida = (int)(destino - saidaColorida);
        }
    }

#ifdef _WIN32
    {
        HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD modo = 0;

        if (saida != INVALID_HANDLE_VALUE && GetConsoleMode(saida, &modo) != 0) {
            wchar_t *textoWide = resolverTextoWideWindows(saidaTexto);
            if (textoWide != NULL) {
                DWORD caracteresEscritos = 0;
                WriteConsoleW(saida, textoWide, (DWORD)wcslen(textoWide), &caracteresEscritos, NULL);
                tamanhoEscrito = (int)caracteresEscritos;
                free(textoWide);
            }
        }
    }
#endif

    if (tamanhoEscrito < 0) {
#ifdef _WIN32
        wchar_t *textoWide = resolverTextoWideWindows(saidaTexto);
        if (textoWide != NULL) {
            char *bytesAcp = NULL;
            int tamanhoBytes = 0;

            bytesAcp = converterWideParaBytes(textoWide, CP_ACP, &tamanhoBytes);
            if (bytesAcp != NULL && tamanhoBytes > 0) {
                tamanhoEscrito = (int)fwrite(bytesAcp, 1U, (size_t)(tamanhoBytes - 1), stdout);
                free(bytesAcp);
            }
            free(textoWide);
        }
#endif
    }

    if (tamanhoEscrito < 0) {
        tamanhoEscrito = (int)fwrite(saidaTexto, 1U, (size_t)tamanhoSaida, stdout);
    }

    if (saidaColorida != NULL) {
        free(saidaColorida);
    }

    if (buffer != bufferLocal) {
        free(buffer);
    }

    return tamanhoEscrito;
}

const char *corTerminal(const char *codigo) {
    /* [SLIDE] CAMADA DE COMPATIBILIDADE DE COR: aplica ANSI so quando suportado. */
    if (coresAtivas) {
        return codigo;
    }

    return "";
}

size_t comprimentoVisualUtf8(const char *texto) {
    /* [SLIDE] ALINHAMENTO UTF-8: conta largura visual real para manter colunas alinhadas. */
    size_t total = 0;
    const unsigned char *cursor = (const unsigned char *)texto;

    if (texto == NULL) {
        return 0;
    }

    while (*cursor != '\0') {
        if ((*cursor & 0xC0U) != 0x80U) {
            total++;
        }
        cursor++;
    }

    return total;
}

void copiarTrechoUtf8Visivel(const char *origem, char *destino, size_t tamanhoDestino, int larguraMaxima) {
    /* [SLIDE] CORTE SEGURO: limita texto por largura sem quebrar byte de acento. */
    const unsigned char *cursor = (const unsigned char *)origem;
    size_t escritos = 0;
    int larguraAtual = 0;

    if (destino == NULL || tamanhoDestino == 0) {
        return;
    }

    destino[0] = '\0';

    if (origem == NULL || larguraMaxima <= 0) {
        return;
    }

    while (*cursor != '\0' && larguraAtual < larguraMaxima) {
        int bytesCaractere;
        int j;

        if ((*cursor & 0x80U) == 0x00U) {
            bytesCaractere = 1;
        } else if ((*cursor & 0xE0U) == 0xC0U) {
            bytesCaractere = 2;
        } else if ((*cursor & 0xF0U) == 0xE0U) {
            bytesCaractere = 3;
        } else if ((*cursor & 0xF8U) == 0xF0U) {
            bytesCaractere = 4;
        } else {
            bytesCaractere = 1;
        }

        if (escritos + (size_t)bytesCaractere >= tamanhoDestino) {
            break;
        }

        for (j = 0; j < bytesCaractere; j++) {
            if (cursor[j] == '\0') {
                destino[escritos] = '\0';
                return;
            }
            destino[escritos++] = (char)cursor[j];
        }

        cursor += bytesCaractere;
        larguraAtual++;
    }

    destino[escritos] = '\0';
}

void imprimirTextoPaddedUtf8(const char *texto, int largura, int alinharDireita) {
    /* [SLIDE] CELULA DE TABELA: imprime texto com padding e alinhamento estavel. */
    char textoAjustado[512];
    size_t tamanhoVisual;
    int espacos = 0;
    int i;

    copiarTrechoUtf8Visivel(texto, textoAjustado, sizeof(textoAjustado), largura);
    tamanhoVisual = comprimentoVisualUtf8(textoAjustado);

    if ((int)tamanhoVisual < largura) {
        espacos = largura - (int)tamanhoVisual;
    }

    if (alinharDireita) {
        for (i = 0; i < espacos; i++) {
            printf(" ");
        }
        printf("%s", textoAjustado);
        return;
    }

    printf("%s", textoAjustado);
    for (i = 0; i < espacos; i++) {
        printf(" ");
    }
}

void imprimirRepeticao(char caractere, int quantidade) {
    int i;

    for (i = 0; i < quantidade; i++) {
        printf("%c", caractere);
    }
}

void imprimirBordaTabela2(int largura1, int largura2) {
    /* [SLIDE] TABELA 2 COLUNAS: borda horizontal padronizada. */
    printf("+");
    imprimirRepeticao('-', largura1 + 2);
    printf("+");
    imprimirRepeticao('-', largura2 + 2);
    printf("+\n");
}

void imprimirLinhaTabela2(const char *coluna1, const char *coluna2, int largura1, int largura2) {
    /* [SLIDE] TABELA 2 COLUNAS: linha com alinhamento seguro para UTF-8. */
    printf("| ");
    imprimirTextoPaddedUtf8(coluna1, largura1, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8(coluna2, largura2, 0);
    printf(" |\n");
}

void imprimirCabecalhoTabela2(const char *coluna1, const char *coluna2, int largura1, int largura2) {
    /* [SLIDE] TABELA 2 COLUNAS: cabecalho em destaque visual. */
    printf("%s", corTerminal(COR_SECAO));
    imprimirBordaTabela2(largura1, largura2);
    imprimirLinhaTabela2(coluna1, coluna2, largura1, largura2);
    imprimirBordaTabela2(largura1, largura2);
    printf("%s", corTerminal(COR_RESET));
}

/*
 * [SLIDE] FLUXO PRINCIPAL
 * Finalidade: iniciar console, carregar dados padrao e operar o menu.
 */
int main(void) {
    const char categorias[MAX_GASTOS][30] = {
        "Moradia",
        "Alimentação",
        "Transporte",
        "Saúde",
        "Educação",
        "Lazer",
        "Outros"
    };

    Orcamento orcamento = {0};
    Carteira carteira;
    int opcao;

    configurarConsole();
    orcamento.metodoOrcamento = -1;

    inicializarCarteira(&carteira);

    do {
        opcao = menuPrincipal();

        switch (opcao) {
            case 1:
                cadastrarOrcamento(&orcamento);
                break;
            case 2:
                escolherPesos(&carteira, &orcamento, categorias);
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
                printf("%sObrigado por usar o simulador. Bons estudos e bons investimentos!%s\n",
                       corTerminal(COR_SUCESSO),
                       corTerminal(COR_RESET));
                break;
            default:
                printf("%sEntrada inválida.%s\n", corTerminal(COR_ERRO), corTerminal(COR_RESET));
                pausar();
                break;
        }
    } while (opcao != 0);

    return 0;
}

/*
 * [SLIDE] CONTROLE DE TELA E CONSOLE
 * Finalidade: limpar tela, habilitar UTF-8 e ativar cores ANSI.
 */
void limparTela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void configurarConsole(void) {
    setlocale(LC_CTYPE, "");

#ifdef _WIN32
    {
        HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD modoSaida = 0;

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        if (saida != INVALID_HANDLE_VALUE && GetConsoleMode(saida, &modoSaida) != 0) {
            DWORD novoModo = modoSaida | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (SetConsoleMode(saida, novoModo) != 0) {
                coresAtivas = 1;
            } else {
                coresAtivas = 0;
            }
        } else {
            coresAtivas = 0;
        }
    }
#else
    {
        const char *term = getenv("TERM");
        coresAtivas = (term != NULL && strcmp(term, "dumb") != 0) ? 1 : 0;
    }
#endif
}

void pausar(void) {
    char linha[TAM_LINHA];

    printf("%s\nPressione ENTER para continuar...%s", corTerminal(COR_INFO), corTerminal(COR_RESET));
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

int linhaContemApenasZero(const char *linha) {
    const char *inicio;
    const char *fim;

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

    return (fim - inicio == 1 && *inicio == '0');
}

/*
 * [SLIDE] NAVEGACAO UI
 * Finalidade: permitir retorno rapido ao menu (0, MENU, VOLTAR).
 */
void exibirAtalhoRetornoMenu(void) {
    printf("Digite MENU ou VOLTAR a qualquer momento para retornar ao menu principal.\n");
    printf("Em perguntas de escolha, digite 0 para retornar ao menu.\n\n");
}

void retornarMenuPrincipal(void) {
    char linha[TAM_LINHA];

    printf("\n0 - Retornar ao menu principal\n");
    printf("Escolha uma opção: ");

    while (1) {
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada inválida. Tente novamente.\n");
            clearerr(stdin);
            printf("Escolha uma opção: ");
            continue;
        }

        if (linhaContemApenasZero(linha) || linhaSolicitaRetornoMenu(linha)) {
            return;
        }

        printf("Entrada inválida. Digite 0, MENU ou VOLTAR para retornar.\n");
        printf("Escolha uma opção: ");
    }
}

int menuPrincipal(void) {
    char mensagemOpcao[80];

    limparTela();
    printf("%s============================================================%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s SIMULADOR DE CARTEIRA DE INVESTIMENTOS E REBALANCEAMENTO%s\n", corTerminal(COR_TITULO), corTerminal(COR_RESET));
    printf("%s============================================================%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s1%s - Informar renda e salvar dados financeiros\n", corTerminal(COR_SUCESSO), corTerminal(COR_RESET));
    printf("%s2%s - Fazer questionário completo e definir perfil da carteira\n", corTerminal(COR_SUCESSO), corTerminal(COR_RESET));
    printf("%s3%s - Informar valores que já tenho investidos\n", corTerminal(COR_SUCESSO), corTerminal(COR_RESET));
    printf("%s4%s - Ver resumo, valor para investir e rebalanceamento\n", corTerminal(COR_SUCESSO), corTerminal(COR_RESET));
    printf("%s5%s - Ajuda para iniciantes: tipos de investimento\n", corTerminal(COR_SUCESSO), corTerminal(COR_RESET));
    printf("%s0%s - Sair\n", corTerminal(COR_ALERTA), corTerminal(COR_RESET));
    printf("%s============================================================%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));

    snprintf(mensagemOpcao, sizeof(mensagemOpcao), "%sEscolha uma opção:%s ",
             corTerminal(COR_INFO),
             corTerminal(COR_RESET));
    return lerInteiroFaixa(mensagemOpcao, 0, 5);
}

/*
 * [SLIDE] LEITURA VALIDADA
 * Finalidade: garantir entrada consistente (tipo, faixa e comandos de retorno).
 */
int lerInteiroFaixa(const char *mensagem, int minimo, int maximo) {
    char linha[TAM_LINHA];
    char *fim;
    long valor;

    while (1) {
        printf("%s", mensagem);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("Entrada inválida. Tente novamente.\n");
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
            printf("Entrada inválida. Digite apenas números inteiros.\n");
            continue;
        }

        if (atalhoRetornoMenuAtivo && valor == 0 && minimo > 0) {
            retornoMenuSolicitado = 1;
            return minimo;
        }

        if (valor < minimo || valor > maximo) {
            printf("Entrada inválida. Digite um valor entre %d e %d.\n", minimo, maximo);
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
            printf("Entrada inválida. Tente novamente.\n");
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
                printf("Entrada inválida. Digite um número maior ou igual a %.2f.\n", minimo);
            } else {
                printf("Entrada inválida. Digite um número maior que %.2f.\n", minimo);
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
            printf("Entrada inválida. Tente novamente.\n");
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
            printf("Entrada inválida. O nome não pode ficar vazio.\n");
            continue;
        }

        if (tamanho > 0) {
            strncpy(destino, inicio, tamanho - 1);
            destino[tamanho - 1] = '\0';
        }

        return 1;
    }
}

/*
 * [SLIDE] INICIALIZACAO E REGRAS DE NEGOCIO
 * Finalidade: aplicar metodos financeiros e transformar renda em plano.
 */
void inicializarCarteira(Carteira *carteira) {
    const char nomesPadrao[MAX_ATIVOS][80] = {
        "Reserva de emergência (Tesouro Selic/CDB liquidez diária)",
        "Renda fixa conservadora (CDB/LCI/LCA/Tesouro Selic)",
        "Tesouro IPCA+ (protege contra a inflação)",
        "Fundos imobiliários - FIIs (renda mensal)",
        "ETFs de ações (diversificação na bolsa)",
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

const MetodoOrcamento *obterMetodoOrcamento(int indice) {
    if (indice < 0 || indice >= MAX_METODOS) {
        return &METODOS_ORCAMENTO[2];
    }

    return &METODOS_ORCAMENTO[indice];
}

void calcularOrcamentoIdeal(Orcamento *orcamento) {
    /* [SLIDE] CALCULO CENTRAL: distribui a renda por categoria e investimento. */
    const MetodoOrcamento *metodo = obterMetodoOrcamento(orcamento->metodoOrcamento);
    int i;

    orcamento->totalGastos = 0.0;

    for (i = 0; i < MAX_GASTOS; i++) {
        orcamento->gastos[i] = orcamento->rendaMensal * metodo->percentuaisGastos[i] / 100.0;
        orcamento->totalGastos += orcamento->gastos[i];
    }

    orcamento->sobraInvestir = orcamento->rendaMensal * metodo->investimentos / 100.0;
}

double calcularAporteDisponivelInvestimento(const Orcamento *orcamento) {
    /* [SLIDE] APORTE TOTAL: mensal recomendado + reserva disponivel para investir. */
    return orcamento->sobraInvestir + orcamento->dinheiroGuardadoInvestir;
}

int recomendarMetodoOrcamento(const Orcamento *orcamento) {
    /* [SLIDE] QUESTIONARIO DE ORCAMENTO: define metodo mais coerente com o contexto do usuario. */
    int pontuacao = 0;
    int resposta;
    int metodoRecomendado;
    double mesesReserva = 0.0;

    printf("Questionário para escolher um método de organização financeira:\n");
    printf("Responda com 1, 2 ou 3 em cada pergunta.\n\n");

    printf("1) Como fica seu orçamento depois de pagar as contas essenciais?\n");
    printf("1 - Fica apertado ou falta dinheiro\n");
    printf("2 - Sobra pouco, mas consigo me organizar\n");
    printf("3 - Sobra com frequência\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return -1;
    }
    pontuacao += resposta;

    printf("\n2) Como está sua reserva de emergência?\n");
    printf("1 - Ainda não tenho, ou ela ainda é muito pequena\n");
    printf("2 - Estou montando aos poucos\n");
    printf("3 - Já cobre alguns meses de gastos\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return -1;
    }
    pontuacao += resposta;

    printf("\n3) Qual é sua prioridade agora?\n");
    printf("1 - Organizar a vida financeira e evitar aperto\n");
    printf("2 - Equilibrar gastos, lazer e investimentos\n");
    printf("3 - Acelerar investimentos e objetivos\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return -1;
    }
    pontuacao += resposta;

    printf("\n4) Quanto você aceitaria reduzir lazer/estilo de vida para investir mais?\n");
    printf("1 - Pouco; preciso manter mais folga no mês\n");
    printf("2 - Um pouco, desde que o plano continue realista\n");
    printf("3 - Bastante; quero priorizar objetivos financeiros\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return -1;
    }
    pontuacao += resposta;

    if (orcamento->rendaMensal > 0.0) {
        mesesReserva = orcamento->dinheiroGuardadoInvestir / orcamento->rendaMensal;
    }

    if (pontuacao <= 5) {
        metodoRecomendado = 0;
    } else if (pontuacao <= 7) {
        metodoRecomendado = 1;
    } else if (pontuacao <= 10) {
        metodoRecomendado = 2;
    } else {
        metodoRecomendado = 3;
    }

    if (mesesReserva < 1.0 && metodoRecomendado > 0) {
        metodoRecomendado--;
    } else if (mesesReserva >= 6.0 && metodoRecomendado < 3) {
        metodoRecomendado++;
    }

    if (orcamento->idade >= 55 && metodoRecomendado > 0) {
        metodoRecomendado--;
    } else if (orcamento->idade <= 30 && mesesReserva >= 2.0 && metodoRecomendado < 3) {
        metodoRecomendado++;
    }

    return metodoRecomendado;
}

int escolherMetodoOrcamento(int metodoRecomendado) {
    int i;
    int escolha;

    printf("\nMétodo recomendado para você: %d - %s\n",
           metodoRecomendado + 1,
           METODOS_ORCAMENTO[metodoRecomendado].nome);
    printf("%s\n\n", METODOS_ORCAMENTO[metodoRecomendado].descricao);

    printf("Você pode aceitar a recomendação ou escolher outro método:\n");
    for (i = 0; i < MAX_METODOS; i++) {
        printf("%d - %s\n", i + 1, METODOS_ORCAMENTO[i].nome);
        printf("    %.0f%% necessidades | %.0f%% lazer/estilo de vida | %.0f%% investimentos\n",
               METODOS_ORCAMENTO[i].necessidades,
               METODOS_ORCAMENTO[i].lazerEstilo,
               METODOS_ORCAMENTO[i].investimentos);
    }

    escolha = lerInteiroFaixa("\nEscolha o método que deseja usar: ", 1, MAX_METODOS);
    if (consumirRetornoMenuSolicitado()) {
        return -1;
    }

    return escolha - 1;
}

/*
 * [SLIDE] SAIDA DA OPCAO 2
 * Finalidade: apresentar resultado completo do questionario em formato de painel.
 */
void exibirPlanoOrcamento(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]) {
    const MetodoOrcamento *metodo = obterMetodoOrcamento(orcamento->metodoOrcamento);
    double aporteDisponivel = calcularAporteDisponivelInvestimento(orcamento);
    char valor[100];
    int i;

    printf("\n%s", corTerminal(COR_TITULO));
    imprimirRepeticao('=', 78);
    printf("\nRESULTADO DA SIMULAÇÃO DE ORÇAMENTO E INVESTIMENTOS\n");
    imprimirRepeticao('=', 78);
    printf("\n%s", corTerminal(COR_RESET));

    printf("%s\n1) PARÂMETROS INFORMADOS%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    imprimirCabecalhoTabela2("Parâmetro", "Valor", 34, 36);

    snprintf(valor, sizeof(valor), "%s", orcamento->nome);
    imprimirLinhaTabela2("Usuário", valor, 34, 36);

    snprintf(valor, sizeof(valor), "%d anos", orcamento->idade);
    imprimirLinhaTabela2("Idade", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", orcamento->rendaMensal);
    imprimirLinhaTabela2("Renda mensal", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", orcamento->dinheiroGuardadoInvestir);
    imprimirLinhaTabela2("Dinheiro guardado para investir", valor, 34, 36);
    imprimirBordaTabela2(34, 36);

    printf("%s\n2) ANÁLISE%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    imprimirCabecalhoTabela2("Análise", "Valor", 34, 36);

    if (orcamento->perfilInvestidor >= 1 && orcamento->perfilInvestidor <= 3) {
        snprintf(valor, sizeof(valor), "%s", nomePerfilInvestidor(orcamento->perfilInvestidor));
        imprimirLinhaTabela2("Perfil de investidor", valor, 34, 36);
    }

    snprintf(valor, sizeof(valor), "%s", metodo->nome);
    imprimirLinhaTabela2("Método de orçamento", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", orcamento->sobraInvestir);
    imprimirLinhaTabela2("Investimento mensal sugerido", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", aporteDisponivel);
    imprimirLinhaTabela2("Total para investir agora", valor, 34, 36);
    imprimirBordaTabela2(34, 36);

    printf("%s\n3) PLANO IDEAL DA RENDA MENSAL%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s+----------------------+----------------+--------------------+%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s| ", corTerminal(COR_SECAO));
    imprimirTextoPaddedUtf8("Área", 20, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Percentual", 14, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Valor ideal (R$)", 18, 0);
    printf(" |%s\n", corTerminal(COR_RESET));
    printf("%s+----------------------+----------------+--------------------+%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));

    for (i = 0; i < MAX_GASTOS; i++) {
        char percentual[30];
        char valorIdeal[40];

        snprintf(percentual, sizeof(percentual), "%.2f%%", metodo->percentuaisGastos[i]);
        snprintf(valorIdeal, sizeof(valorIdeal), "R$ %.2f", orcamento->gastos[i]);

        printf("| ");
        imprimirTextoPaddedUtf8(categorias[i], 20, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(percentual, 14, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(valorIdeal, 18, 1);
        printf(" |\n");
    }

    {
        char percentual[30];
        char valorIdeal[40];

        snprintf(percentual, sizeof(percentual), "%.2f%%", metodo->investimentos);
        snprintf(valorIdeal, sizeof(valorIdeal), "R$ %.2f", orcamento->sobraInvestir);

        printf("| ");
        imprimirTextoPaddedUtf8("Investimentos", 20, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(percentual, 14, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(valorIdeal, 18, 1);
        printf(" |\n");
    }
    printf("+----------------------+----------------+--------------------+\n");
    {
        char totalGastos[40];
        snprintf(totalGastos, sizeof(totalGastos), "R$ %.2f", orcamento->totalGastos);

        printf("| ");
        imprimirTextoPaddedUtf8("Total de gastos", 20, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8("", 14, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(totalGastos, 18, 1);
        printf(" |\n");
    }
    printf("+----------------------+----------------+--------------------+\n");

    if (carteira == NULL || !carteira->preenchida) {
        return;
    }

    printf("%s\n4) CARTEIRA SUGERIDA%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s+------------------------------------------------------------+----------+--------------------+--------------------+%s\n",
           corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s| ", corTerminal(COR_SECAO));
    imprimirTextoPaddedUtf8("Ativo", 58, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Peso", 8, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Aporte mensal (R$)", 18, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Aporte total (R$)", 18, 0);
    printf(" |%s\n", corTerminal(COR_RESET));
    printf("%s+------------------------------------------------------------+----------+--------------------+--------------------+%s\n",
           corTerminal(COR_SECAO), corTerminal(COR_RESET));

    for (i = 0; i < MAX_ATIVOS; i++) {
        double aporteMensal = orcamento->sobraInvestir * carteira->pesos[i] / 100.0;
        double aporteTotal = aporteDisponivel * carteira->pesos[i] / 100.0;
        char peso[20];
        char mensal[40];
        char total[40];

        snprintf(peso, sizeof(peso), "%.2f%%", carteira->pesos[i]);
        snprintf(mensal, sizeof(mensal), "R$ %.2f", aporteMensal);
        snprintf(total, sizeof(total), "R$ %.2f", aporteTotal);

        printf("| ");
        imprimirTextoPaddedUtf8(carteira->nomes[i], 58, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(peso, 8, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(mensal, 18, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(total, 18, 1);
        printf(" |\n");
    }

    printf("+------------------------------------------------------------+----------+--------------------+--------------------+\n");
    printf("%sMétodo escolhido:%s %s\n", corTerminal(COR_ALERTA), corTerminal(COR_RESET), metodo->descricao);
}

/*
 * [SLIDE] OPCAO 1 - CADASTRO BASE
 * Finalidade: capturar dados financeiros que alimentam os calculos das demais opcoes.
 */
void cadastrarOrcamento(Orcamento *orcamento) {
    Orcamento temporario = *orcamento;
    int i;

    limparTela();
    printf("============ DADOS FINANCEIROS DO USUÁRIO ============\n");
    printf("Nesta opção, vamos apenas cadastrar seus dados.\n");
    printf("O questionário completo será feito na opção 2.\n");
    printf("Use ponto para separar os centavos.\n");
    printf("Exemplo: 2500.50\n\n");
    exibirAtalhoRetornoMenu();

    ativarAtalhoRetornoMenu();

    if (!lerTextoObrigatorio("Nome do usuário: ", temporario.nome, sizeof(temporario.nome))) {
        desativarAtalhoRetornoMenu();
        return;
    }

    temporario.idade = lerInteiroFaixa("Idade do usuário: ", 1, 120);
    if (consumirRetornoMenuSolicitado()) {
        desativarAtalhoRetornoMenu();
        return;
    }

    temporario.rendaMensal = lerDoubleMinimo("Renda mensal total: R$ ", 0.0, 0);
    if (consumirRetornoMenuSolicitado()) {
        desativarAtalhoRetornoMenu();
        return;
    }

    temporario.dinheiroGuardadoInvestir = lerDoubleMinimo("Dinheiro guardado disponível para investir agora: R$ ", 0.0, 1);
    if (consumirRetornoMenuSolicitado()) {
        desativarAtalhoRetornoMenu();
        return;
    }

    desativarAtalhoRetornoMenu();

    for (i = 0; i < MAX_GASTOS; i++) {
        temporario.gastos[i] = 0.0;
    }

    temporario.totalGastos = 0.0;
    temporario.sobraInvestir = 0.0;
    temporario.metodoOrcamento = -1;
    temporario.perfilInvestidor = 0;
    temporario.preenchido = 1;
    *orcamento = temporario;

    printf("\nDados cadastrados com sucesso.\n");
    printf("Agora use a opção 2 para responder o questionário completo e definir seu perfil de investidor.\n");

    retornarMenuPrincipal();
}

int identificarPerfilQuestionarioIntegrado(const Orcamento *orcamento, int *metodoRecomendado) {
    /* [SLIDE] PERFIL INTEGRADO: combina tolerancia a risco com realidade financeira. */
    int pontuacao = 0;
    int resposta;
    int perfil;
    double mesesReserva = 0.0;

    if (metodoRecomendado == NULL) {
        return 0;
    }

    *metodoRecomendado = recomendarMetodoOrcamento(orcamento);
    if (*metodoRecomendado < 0) {
        return 0;
    }

    printf("\nAgora vamos definir seu perfil de investidor:\n");
    printf("Responda com 1, 2 ou 3 em cada pergunta.\n\n");

    printf("1) Qual seu objetivo principal ao investir?\n");
    printf("1 - Preservar meu dinheiro com baixo risco\n");
    printf("2 - Equilibrar segurança e crescimento\n");
    printf("3 - Buscar maior crescimento, aceitando oscilações\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n2) Por quanto tempo você pretende deixar o dinheiro investido?\n");
    printf("1 - Menos de 2 anos\n");
    printf("2 - Entre 2 e 5 anos\n");
    printf("3 - Mais de 5 anos\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n3) Se sua carteira cair 10%% em alguns meses, o que você faria?\n");
    printf("1 - Resgataria para evitar mais perdas\n");
    printf("2 - Manteria e aguardaria a recuperação\n");
    printf("3 - Aproveitaria para investir mais\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n4) Qual é seu nível de experiência com investimentos?\n");
    printf("1 - Iniciante\n");
    printf("2 - Intermediário\n");
    printf("3 - Avançado\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    printf("\n5) Quanto de oscilação você aceita para buscar mais retorno?\n");
    printf("1 - Quase nenhuma oscilação\n");
    printf("2 - Oscilação moderada\n");
    printf("3 - Oscilação alta\n");
    resposta = lerInteiroFaixa("Resposta: ", 1, 3);
    if (consumirRetornoMenuSolicitado()) {
        return 0;
    }
    pontuacao += resposta;

    if (pontuacao <= 8) {
        perfil = 1;
    } else if (pontuacao <= 11) {
        perfil = 2;
    } else {
        perfil = 3;
    }

    if (orcamento->rendaMensal > 0.0) {
        mesesReserva = orcamento->dinheiroGuardadoInvestir / orcamento->rendaMensal;
    }

    if (mesesReserva < 1.0 && perfil > 1) {
        perfil--;
    } else if (mesesReserva >= 6.0 && perfil < 3) {
        perfil++;
    }

    if (orcamento->idade >= 60 && perfil > 1) {
        perfil--;
    } else if (orcamento->idade <= 30 && mesesReserva >= 2.0 && perfil < 3) {
        perfil++;
    }

    if (*metodoRecomendado == 0 && perfil > 1) {
        perfil--;
    } else if (*metodoRecomendado == 3 && perfil < 3) {
        perfil++;
    }

    return perfil;
}

/*
 * [SLIDE] OPCAO 2 - QUESTIONARIO E PESOS
 * Finalidade: definir perfil, metodo e pesos da carteira em uma unica etapa.
 */
void escolherPesos(Carteira *carteira, Orcamento *orcamento, const char categorias[MAX_GASTOS][30]) {
    int perfil;
    int metodoRecomendado;
    int metodoEscolhido;

    limparTela();
    printf("=============== QUESTIONÁRIO E PERFIL ===============\n");

    if (!orcamento->preenchido) {
        printf("Dados financeiros ainda não cadastrados. Use a opção 1 primeiro.\n");
        retornarMenuPrincipal();
        return;
    }

    printf("Nesta opção, o sistema fará um questionário completo para:\n");
    printf("- sugerir um método de orçamento;\n");
    printf("- definir seu perfil de investidor;\n");
    printf("- calcular os pesos percentuais da carteira.\n\n");
    exibirAtalhoRetornoMenu();

    ativarAtalhoRetornoMenu();
    perfil = identificarPerfilQuestionarioIntegrado(orcamento, &metodoRecomendado);
    if (perfil == 0) {
        desativarAtalhoRetornoMenu();
        return;
    }

    metodoEscolhido = escolherMetodoOrcamento(metodoRecomendado);
    if (metodoEscolhido < 0) {
        desativarAtalhoRetornoMenu();
        return;
    }

    definirPerfilAutomatico(carteira, perfil);
    carteira->preenchida = 1;

    orcamento->metodoOrcamento = metodoEscolhido;
    orcamento->perfilInvestidor = perfil;
    calcularOrcamentoIdeal(orcamento);

    desativarAtalhoRetornoMenu();

    printf("\n%sQuestionário concluído com sucesso.%s\n", corTerminal(COR_SUCESSO), corTerminal(COR_RESET));
    printf("Perfil identificado: %s\n", nomePerfilInvestidor(perfil));
    printf("Método selecionado: %s\n", METODOS_ORCAMENTO[metodoEscolhido].nome);

    exibirPlanoOrcamento(orcamento, carteira, categorias);
    retornarMenuPrincipal();
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

/*
 * [SLIDE] OPCAO 3 - CARTEIRA ATUAL
 * Finalidade: registrar o que o usuario ja possui investido hoje.
 */
void cadastrarCarteiraAtual(Carteira *carteira) {
    int i;
    double valoresOriginais[MAX_ATIVOS];
    double precosOriginais[MAX_ATIVOS];
    double quantidadesOriginais[MAX_ATIVOS];
    double totalAtual = 0.0;

    limparTela();
    printf("=============== CARTEIRA ATUAL ===============\n");
    printf("Nesta etapa, informe quanto dinheiro você já tem em cada tipo de investimento.\n");
    printf("Não é necessário saber preço de cota, quantidade ou termos de corretora.\n");
    printf("Digite apenas o valor total em reais.\n\n");
    printf("Exemplos:\n");
    printf("- Tenho R$ 500 em Tesouro Selic: digite 500\n");
    printf("- Tenho R$ 120 em FIIs: digite 120\n");
    printf("- Ainda não tenho esse investimento: digite 0\n\n");
    exibirAtalhoRetornoMenu();

    ativarAtalhoRetornoMenu();

    for (i = 0; i < MAX_ATIVOS; i++) {
        valoresOriginais[i] = carteira->valoresAtuais[i];
        precosOriginais[i] = carteira->precos[i];
        quantidadesOriginais[i] = carteira->quantidades[i];
    }

    for (i = 0; i < MAX_ATIVOS; i++) {
        char mensagem[170];

        snprintf(mensagem, sizeof(mensagem), "Quanto você já tem em %s? R$ ", carteira->nomes[i]);
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
    printf("Total que você já possui investido: R$ %.2f\n", totalAtual);

    if (totalAtual == 0.0) {
        printf("Tudo bem se você ainda não tem investimentos. O simulador usará seu aporte mensal e o dinheiro guardado disponível para sugerir os primeiros aportes.\n");
    } else {
        printf("Esses valores serão comparados com os percentuais da opção 2 para mostrar o rebalanceamento.\n");
    }

    retornarMenuPrincipal();
}

/*
 * [SLIDE] OPCAO 4 - RESUMO GERAL
 * Finalidade: consolidar dados, plano ideal, alocacao sugerida e rebalanceamento.
 */
void exibirResumo(const Orcamento *orcamento, const Carteira *carteira, const char categorias[MAX_GASTOS][30]) {
    const MetodoOrcamento *metodo = obterMetodoOrcamento(orcamento->metodoOrcamento);
    double aporteDisponivel = calcularAporteDisponivelInvestimento(orcamento);
    char valor[100];
    int i;

    limparTela();
    printf("%s", corTerminal(COR_TITULO));
    imprimirRepeticao('=', 78);
    printf("\nRESUMO GERAL DA SIMULAÇÃO\n");
    imprimirRepeticao('=', 78);
    printf("\n%s", corTerminal(COR_RESET));

    if (!orcamento->preenchido) {
        printf("Orçamento ainda não cadastrado. Use a opção 1 primeiro.\n");
        retornarMenuPrincipal();
        return;
    }

    if (orcamento->metodoOrcamento < 0) {
        printf("Questionário e perfil ainda não definidos. Use a opção 2 primeiro.\n");
        retornarMenuPrincipal();
        return;
    }

    printf("%s\n1) VISÃO GERAL%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    imprimirCabecalhoTabela2("Indicador", "Valor", 34, 36);

    snprintf(valor, sizeof(valor), "%s", orcamento->nome);
    imprimirLinhaTabela2("Usuário", valor, 34, 36);

    snprintf(valor, sizeof(valor), "%d anos", orcamento->idade);
    imprimirLinhaTabela2("Idade", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", orcamento->rendaMensal);
    imprimirLinhaTabela2("Renda mensal", valor, 34, 36);

    if (orcamento->perfilInvestidor >= 1 && orcamento->perfilInvestidor <= 3) {
        snprintf(valor, sizeof(valor), "%s", nomePerfilInvestidor(orcamento->perfilInvestidor));
        imprimirLinhaTabela2("Perfil de investidor", valor, 34, 36);
    }

    snprintf(valor, sizeof(valor), "%s", metodo->nome);
    imprimirLinhaTabela2("Método de orçamento", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", orcamento->totalGastos);
    imprimirLinhaTabela2("Total ideal para gastos", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f (%.2f%% da renda)", orcamento->sobraInvestir, metodo->investimentos);
    imprimirLinhaTabela2("Investimento mensal sugerido", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", orcamento->dinheiroGuardadoInvestir);
    imprimirLinhaTabela2("Dinheiro guardado disponível", valor, 34, 36);

    snprintf(valor, sizeof(valor), "R$ %.2f", aporteDisponivel);
    imprimirLinhaTabela2("Total para investir agora", valor, 34, 36);
    imprimirBordaTabela2(34, 36);

    printf("%s\n2) GASTOS IDEAIS POR ÁREA%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s+----------------------+----------------+--------------------+%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s| ", corTerminal(COR_SECAO));
    imprimirTextoPaddedUtf8("Área", 20, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Percentual", 14, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Valor ideal (R$)", 18, 0);
    printf(" |%s\n", corTerminal(COR_RESET));
    printf("%s+----------------------+----------------+--------------------+%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));

    for (i = 0; i < MAX_GASTOS; i++) {
        char percentual[30];
        char valorIdeal[40];

        snprintf(percentual, sizeof(percentual), "%.2f%%", metodo->percentuaisGastos[i]);
        snprintf(valorIdeal, sizeof(valorIdeal), "R$ %.2f", orcamento->gastos[i]);

        printf("| ");
        imprimirTextoPaddedUtf8(categorias[i], 20, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(percentual, 14, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(valorIdeal, 18, 1);
        printf(" |\n");
    }
    printf("+----------------------+----------------+--------------------+\n");
    {
        char percentual[30];
        char valorIdeal[40];

        snprintf(percentual, sizeof(percentual), "%.2f%%", metodo->investimentos);
        snprintf(valorIdeal, sizeof(valorIdeal), "R$ %.2f", orcamento->sobraInvestir);

        printf("| ");
        imprimirTextoPaddedUtf8("Investimentos", 20, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(percentual, 14, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(valorIdeal, 18, 1);
        printf(" |\n");
    }
    printf("+----------------------+----------------+--------------------+\n");

    if (aporteDisponivel <= 0.0) {
        printf("%s\nNão há valor disponível para investimento no momento.%s\n", corTerminal(COR_ALERTA), corTerminal(COR_RESET));
        retornarMenuPrincipal();
        return;
    }

    if (!carteira->preenchida || valorAbsoluto(somarPesos(carteira) - 100.0) > 0.01) {
        printf("\nPesos da carteira ainda não cadastrados. Use a opção 2.\n");
        retornarMenuPrincipal();
        return;
    }

    printf("%s\n3) ALOCAÇÃO SUGERIDA PARA A CARTEIRA%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s+------------------------------------------------------------+----------+--------------------+--------------------+%s\n",
           corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s| ", corTerminal(COR_SECAO));
    imprimirTextoPaddedUtf8("Ativo", 58, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Peso", 8, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Aporte mensal (R$)", 18, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Aporte total (R$)", 18, 0);
    printf(" |%s\n", corTerminal(COR_RESET));
    printf("%s+------------------------------------------------------------+----------+--------------------+--------------------+%s\n",
           corTerminal(COR_SECAO), corTerminal(COR_RESET));

    for (i = 0; i < MAX_ATIVOS; i++) {
        double aporteMensal = orcamento->sobraInvestir * carteira->pesos[i] / 100.0;
        double aporteTotal = aporteDisponivel * carteira->pesos[i] / 100.0;
        char peso[20];
        char mensal[40];
        char total[40];

        snprintf(peso, sizeof(peso), "%.2f%%", carteira->pesos[i]);
        snprintf(mensal, sizeof(mensal), "R$ %.2f", aporteMensal);
        snprintf(total, sizeof(total), "R$ %.2f", aporteTotal);

        printf("| ");
        imprimirTextoPaddedUtf8(carteira->nomes[i], 58, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(peso, 8, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(mensal, 18, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(total, 18, 1);
        printf(" |\n");
    }
    printf("+------------------------------------------------------------+----------+--------------------+--------------------+\n");

    simularRebalanceamento(orcamento, carteira);
    retornarMenuPrincipal();
}

/* [SLIDE] REBALANCEAMENTO: compara carteira atual com meta e sugere ajuste por ativo. */
void simularRebalanceamento(const Orcamento *orcamento, const Carteira *carteira) {
    double totalAtual = 0.0;
    double aporteDisponivel = calcularAporteDisponivelInvestimento(orcamento);
    double totalProjetado;
    char acao[80];
    int i;

    for (i = 0; i < MAX_ATIVOS; i++) {
        totalAtual += carteira->valoresAtuais[i];
    }

    if (totalAtual <= 0.0) {
        printf("%s\nVocê ainda não cadastrou valores atuais na carteira.%s\n", corTerminal(COR_ALERTA), corTerminal(COR_RESET));
        printf("Mesmo assim, a tabela acima já mostra quanto investir em cada item.\n");
        return;
    }

    totalProjetado = totalAtual + aporteDisponivel;

    printf("%s\n4) REBALANCEAMENTO DA CARTEIRA%s\n", corTerminal(COR_SECAO), corTerminal(COR_RESET));
    imprimirCabecalhoTabela2("Indicador", "Valor", 34, 36);

    snprintf(acao, sizeof(acao), "R$ %.2f", totalAtual);
    imprimirLinhaTabela2("Carteira atual", acao, 34, 36);
    snprintf(acao, sizeof(acao), "R$ %.2f", aporteDisponivel);
    imprimirLinhaTabela2("Aporte disponível agora", acao, 34, 36);
    snprintf(acao, sizeof(acao), "R$ %.2f", totalProjetado);
    imprimirLinhaTabela2("Carteira após o aporte", acao, 34, 36);
    imprimirBordaTabela2(34, 36);

    printf("%s+------------------------------------------------------------+---------------+---------------+-----------------------------+%s\n",
           corTerminal(COR_SECAO), corTerminal(COR_RESET));
    printf("%s| ", corTerminal(COR_SECAO));
    imprimirTextoPaddedUtf8("Investimento", 58, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Atual (R$)", 13, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Ideal (R$)", 13, 0);
    printf(" | ");
    imprimirTextoPaddedUtf8("Ação sugerida", 27, 0);
    printf(" |%s\n", corTerminal(COR_RESET));
    printf("%s+------------------------------------------------------------+---------------+---------------+-----------------------------+%s\n",
           corTerminal(COR_SECAO), corTerminal(COR_RESET));

    for (i = 0; i < MAX_ATIVOS; i++) {
        double valorIdeal = totalProjetado * carteira->pesos[i] / 100.0;
        double diferenca = valorIdeal - carteira->valoresAtuais[i];
        char atual[40];
        char ideal[40];

        if (diferenca > 0.01) {
            snprintf(acao, sizeof(acao), "Aportar R$ %.2f", diferenca);
        } else if (diferenca < -0.01) {
            snprintf(acao, sizeof(acao), "Pausar aporte (excesso R$ %.2f)", valorAbsoluto(diferenca));
        } else {
            snprintf(acao, sizeof(acao), "Manter");
        }

        snprintf(atual, sizeof(atual), "%.2f", carteira->valoresAtuais[i]);
        snprintf(ideal, sizeof(ideal), "%.2f", valorIdeal);

        printf("| ");
        imprimirTextoPaddedUtf8(carteira->nomes[i], 58, 0);
        printf(" | ");
        imprimirTextoPaddedUtf8(atual, 13, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(ideal, 13, 1);
        printf(" | ");
        imprimirTextoPaddedUtf8(acao, 27, 0);
        printf(" |\n");
    }
    printf("+------------------------------------------------------------+---------------+---------------+-----------------------------+\n");

    printf("%s\nLeitura para iniciantes:%s\n", corTerminal(COR_ALERTA), corTerminal(COR_RESET));
    printf("- Aportar significa colocar mais dinheiro no item que está abaixo da meta.\n");
    printf("- Pausar aporte significa que o item está acima da meta. Iniciantes podem apenas não colocar dinheiro nele por enquanto.\n");
    printf("- Nunca invista dinheiro da reserva de emergência em produtos de alto risco.\n");
}

/*
 * [SLIDE] OPCAO 5 - AJUDA DIDATICA
 * Finalidade: explicar conceitos basicos para iniciantes em investimentos.
 */
void exibirAjudaInvestimentos(void) {
    limparTela();
    printf("============== AJUDA PARA INICIANTES ==============\n");
    printf("Este programa não substitui um profissional, mas ajuda a organizar ideias.\n\n");

    printf("1. Reserva de emergência\n");
    printf("   É o primeiro passo e deve ficar em algo seguro, com liquidez diária.\n");
    printf("   Exemplos: Tesouro Selic, CDB com liquidez diária e fundo DI simples.\n");
    printf("   Objetivo: cobrir de 3 a 6 meses de gastos essenciais.\n\n");

    printf("2. Renda fixa conservadora\n");
    printf("   Boa para quem está começando e quer previsibilidade.\n");
    printf("   Exemplos: CDB, LCI, LCA e Tesouro Selic. Compare rentabilidade, prazo e garantia do FGC.\n\n");

    printf("3. Tesouro IPCA+\n");
    printf("   Ajuda a proteger o dinheiro da inflação no longo prazo.\n");
    printf("   Pode oscilar antes do vencimento; portanto, combina melhor com objetivos distantes.\n\n");

    printf("4. Fundos imobiliários (FIIs)\n");
    printf("   São negociados na bolsa e podem pagar rendimentos mensais.\n");
    printf("   Possuem risco de mercado, vacância, gestão e mudança nos rendimentos.\n\n");

    printf("5. ETFs de ações\n");
    printf("   São fundos negociados em bolsa que compram várias ações de uma vez.\n");
    printf("   Servem para diversificar sem escolher empresa por empresa, mas oscilam bastante.\n\n");

    printf("Regra simples para iniciantes:\n");
    printf("- Quite dívidas caras antes de investir.\n");
    printf("- Monte uma reserva de emergência antes de correr risco.\n");
    printf("- Invista todo mês, mesmo que seja pouco.\n");
    printf("- Rebalanceie quando um investimento ficar muito acima ou abaixo da meta.\n");
    printf("- Desconfie de promessas de ganho rápido e garantido.\n");

    retornarMenuPrincipal();
}

double valorAbsoluto(double valor) {
    /* [SLIDE] UTILITARIO MATEMATICO: retorna magnitude positiva de um numero. */
    if (valor < 0.0) {
        return -valor;
    }

    return valor;
}
