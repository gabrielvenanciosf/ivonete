# Frontend - Simulador de Carteira e Rebalanceamento

Frontend desacoplado em HTML5 + CSS puro + JavaScript modular (ES Modules), integrando com backend C via API REST JSON.

## Stack usada

- Linguagem/Framework: HTML5 + Vanilla JavaScript
- Estilizacao: CSS puro
- Requisicoes HTTP: Fetch API nativa

## Estrutura

```text
frontend/
  index.html
  styles.css
  src/
    config.js
    api.js
    state.js
    ui.js
    main.js
```

## Endpoints esperados do backend C

- `GET /usuarios`
- `POST /usuarios` com payload JSON, por exemplo: `{"nome":"Alice"}`
- `DELETE /usuarios/:id`

Por padrao, o frontend aponta para `http://localhost:8080`.

Se a sua API estiver em outra porta, abra:

`http://localhost:5500/?api=http://localhost:9090`

## Como executar

1. Suba o backend C (API HTTP REST) localmente.
2. Sirva a pasta `frontend` com um servidor estatico.

Exemplo com Python:

```bash
cd frontend
python -m http.server 5500
```

3. Abra no navegador:

`http://localhost:5500`

## O que foi implementado

- Menu visual no formato solicitado (itens 1 a 5)
- Passo 1: nome, idade e ganho mensal com plano ideal pela regra 50/30/20
- Passo 2: questionario de 5 perguntas para definir perfil de investidor (conservador, moderado ou arrojado)
- Passo 3: entrada de valores ja investidos
- Passo 4: resumo com calculo de rebalanceamento e valor a investir por classe
- Passo 5: ajuda para leigos por tipo de investimento
- CRUD de usuarios integrado ao backend C (GET/POST/DELETE)

## Requisitos tecnicos atendidos

- Headers JSON enviados nas requisicoes (`Accept` e `Content-Type`)
- Tratamento de erro com mensagens amigaveis (timeout, indisponibilidade, status HTTP)
- Atualizacao automatica da UI apos POST e DELETE
- Separacao clara entre camada de API, estado/regra de negocio e renderizacao
