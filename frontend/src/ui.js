const moneyFormatter = new Intl.NumberFormat("pt-BR", {
  style: "currency",
  currency: "BRL"
});

const percentFormatter = new Intl.NumberFormat("pt-BR", {
  minimumFractionDigits: 1,
  maximumFractionDigits: 1
});

export function getElements() {
  return {
    apiBaseUrl: document.getElementById("api-base-url"),
    feedback: document.getElementById("feedback"),
    userForm: document.getElementById("user-form"),
    userName: document.getElementById("user-name"),
    usersContainer: document.getElementById("users-container"),
    refreshUsersButton: document.getElementById("refresh-users"),
    plannerForm: document.getElementById("planner-form"),
    investorName: document.getElementById("investor-name"),
    investorAge: document.getElementById("investor-age"),
    monthlyIncome: document.getElementById("monthly-income"),
    idealPlan: document.getElementById("ideal-plan"),
    profileForm: document.getElementById("profile-form"),
    profileResult: document.getElementById("profile-result"),
    currentForm: document.getElementById("current-form"),
    summary: document.getElementById("summary"),
    rebalanceButton: document.getElementById("rebalance-btn")
  };
}

export function setApiBaseUrl(element, baseUrl) {
  element.textContent = baseUrl;
}

export function setFeedback(element, message, type = "neutral") {
  element.className = `feedback ${type}`;
  element.textContent = message;
}

export function renderUsers(container, users, onDelete) {
  if (!users.length) {
    container.innerHTML = "<p class='hint'>Nenhum usuario encontrado no backend.</p>";
    return;
  }

  const list = document.createElement("ul");
  list.className = "users-list";

  users.forEach((user) => {
    const item = document.createElement("li");
    const text = document.createElement("span");
    text.textContent = `#${user.id ?? "?"} - ${user.nome ?? "Sem nome"}`;

    const button = document.createElement("button");
    button.type = "button";
    button.className = "btn btn-danger";
    button.textContent = "Excluir";
    button.addEventListener("click", () => onDelete(user.id));

    item.appendChild(text);
    item.appendChild(button);
    list.appendChild(item);
  });

  container.innerHTML = "";
  container.appendChild(list);
}

export function renderIdealPlan(element, plan, monthlyIncome, investor) {
  if (!plan) {
    element.innerHTML = "<p class='hint'>Informe o ganho mensal para gerar o plano.</p>";
    return;
  }

  const rows = plan.rows
    .map(
      (row) => `
        <tr>
          <td>${row.area}</td>
          <td>${percentFormatter.format(row.percent)}%</td>
          <td>${moneyFormatter.format(row.idealValue)}</td>
        </tr>
      `
    )
    .join("");

  element.innerHTML = `
    <p><strong>Investidor:</strong> ${investor.name} | <strong>Idade:</strong> ${investor.age} anos</p>
    <p><strong>Renda mensal considerada:</strong> ${moneyFormatter.format(monthlyIncome)}</p>
    <p><strong>Regra usada:</strong> ${plan.needsPercent}% necessidades, ${plan.lifestylePercent}% lazer/estilo de vida e ${plan.investmentsPercent}% investimentos.</p>
    <p><strong>Plano ideal de uso da renda mensal:</strong></p>
    <table>
      <thead>
        <tr>
          <th>Area</th>
          <th>% da renda</th>
          <th>Valor ideal</th>
        </tr>
      </thead>
      <tbody>${rows}</tbody>
    </table>
    <p><strong>Total ideal para gastos:</strong> ${moneyFormatter.format(plan.totalExpenses)}</p>
    <p><strong>Valor ideal para investimentos:</strong> ${moneyFormatter.format(plan.investmentsValue)}</p>
  `;
}

export function renderProfileResult(element, profile) {
  if (!profile) {
    element.innerHTML = "<p class='hint'>Responda o questionario para descobrir seu perfil.</p>";
    return;
  }

  element.innerHTML = `
    <p><strong>Perfil identificado:</strong> ${profile.label}</p>
    <p><strong>Pontuacao:</strong> ${profile.score} (faixa ${profile.minScore} a ${profile.maxScore})</p>
    <p><strong>Pesos recomendados para rebalanceamento:</strong></p>
    <table>
      <thead>
        <tr>
          <th>Classe</th>
          <th>Peso</th>
        </tr>
      </thead>
      <tbody>
        <tr><td>Renda fixa</td><td>${percentFormatter.format(profile.weights.rendaFixa)}%</td></tr>
        <tr><td>Acoes</td><td>${percentFormatter.format(profile.weights.acoes)}%</td></tr>
        <tr><td>FIIs</td><td>${percentFormatter.format(profile.weights.fiis)}%</td></tr>
        <tr><td>Exterior</td><td>${percentFormatter.format(profile.weights.exterior)}%</td></tr>
        <tr><td>Reserva</td><td>${percentFormatter.format(profile.weights.reserva)}%</td></tr>
      </tbody>
    </table>
  `;
}

export function renderSummary(element, result) {
  if (!result) {
    element.innerHTML = "<p class='hint'>Clique em calcular para ver o rebalanceamento.</p>";
    return;
  }

  const rows = result.rows
    .map((row) => {
      const actionText =
        row.delta >= 0
          ? `Investir ${moneyFormatter.format(row.delta)}`
          : `Reduzir ${moneyFormatter.format(Math.abs(row.delta))}`;

      return `
        <tr>
          <td>${row.label}</td>
          <td>${moneyFormatter.format(row.currentValue)}</td>
          <td>${percentFormatter.format(row.targetPercent)}%</td>
          <td>${moneyFormatter.format(row.targetValue)}</td>
          <td>${actionText}</td>
        </tr>
      `;
    })
    .join("");

  element.innerHTML = `
    <p><strong>Total atual:</strong> ${moneyFormatter.format(result.currentTotal)}</p>
    <p><strong>Aporte planejado:</strong> ${moneyFormatter.format(result.plannedContribution)}</p>
    <p><strong>Total projetado:</strong> ${moneyFormatter.format(result.projectedTotal)}</p>
    <p><strong>Total a investir para equilibrar:</strong> ${moneyFormatter.format(result.totalPositiveDelta)}</p>
    <table>
      <thead>
        <tr>
          <th>Classe</th>
          <th>Atual</th>
          <th>Peso alvo</th>
          <th>Valor alvo</th>
          <th>Acao</th>
        </tr>
      </thead>
      <tbody>${rows}</tbody>
    </table>
  `;
}
