import { API_BASE_URL } from "./config.js";
import { ApiClient } from "./api.js";
import {
  state,
  parseAllocationForm,
  getWeightsTotal,
  getSuggestedPlan,
  evaluateInvestorProfile,
  calculateRebalance,
  normalizeNumber
} from "./state.js";
import {
  getElements,
  setApiBaseUrl,
  setFeedback,
  renderUsers,
  renderIdealPlan,
  renderProfileResult,
  renderSummary
} from "./ui.js";

const api = new ApiClient(API_BASE_URL);
const elements = getElements();

function applyCurrentOnForm(values) {
  Object.entries(values).forEach(([key, value]) => {
    const input = elements.currentForm.elements.namedItem(key);
    if (input) {
      input.value = String(value);
    }
  });
}

async function loadUsers() {
  try {
    setFeedback(elements.feedback, "Carregando usuarios do backend C...", "neutral");
    const users = await api.getUsers();
    state.users = users;
    renderUsers(elements.usersContainer, state.users, handleDeleteUser);
    setFeedback(elements.feedback, "Usuarios carregados com sucesso.", "success");
  } catch (error) {
    setFeedback(elements.feedback, error.message, "error");
  }
}

async function handleCreateUser(event) {
  event.preventDefault();
  const nome = elements.userName.value.trim();

  if (!nome) {
    setFeedback(elements.feedback, "Informe o nome do usuario para cadastrar.", "error");
    return;
  }

  try {
    setFeedback(elements.feedback, "Enviando POST /usuarios...", "neutral");
    await api.createUser({ nome });
    elements.userForm.reset();
    await loadUsers();
    setFeedback(elements.feedback, "Usuario cadastrado e lista atualizada.", "success");
  } catch (error) {
    setFeedback(elements.feedback, error.message, "error");
  }
}

async function handleDeleteUser(id) {
  if (id === undefined || id === null) {
    setFeedback(elements.feedback, "ID invalido para exclusao.", "error");
    return;
  }

  try {
    setFeedback(elements.feedback, `Removendo usuario #${id}...`, "neutral");
    await api.deleteUser(id);
    await loadUsers();
    setFeedback(elements.feedback, `Usuario #${id} removido com sucesso.`, "success");
  } catch (error) {
    setFeedback(elements.feedback, error.message, "error");
  }
}

function handlePlannerSubmit(event) {
  event.preventDefault();
  const name = elements.investorName.value.trim();
  const age = normalizeNumber(elements.investorAge.value);
  const income = normalizeNumber(elements.monthlyIncome.value);

  if (!name) {
    setFeedback(elements.feedback, "Informe o nome do investidor.", "error");
    return;
  }

  if (!age || age < 1) {
    setFeedback(elements.feedback, "Informe uma idade valida.", "error");
    return;
  }

  if (!income) {
    setFeedback(elements.feedback, "Informe um ganho mensal valido.", "error");
    return;
  }

  state.investor = { name, age };
  state.monthlyIncome = income;
  state.idealPlan = getSuggestedPlan(income);
  renderIdealPlan(elements.idealPlan, state.idealPlan, income, state.investor);

  setFeedback(
    elements.feedback,
    `Plano ideal gerado para ${name}.`,
    "success"
  );
}

function handleProfileSubmit(event) {
  event.preventDefault();
  const data = new FormData(elements.profileForm);
  const answers = [
    Number(data.get("q1")),
    Number(data.get("q2")),
    Number(data.get("q3")),
    Number(data.get("q4")),
    Number(data.get("q5"))
  ];

  if (answers.some((value) => !Number.isFinite(value))) {
    setFeedback(elements.feedback, "Responda as 5 perguntas do perfil antes de continuar.", "error");
    return;
  }

  const profile = evaluateInvestorProfile(answers);
  state.investorProfile = profile;
  state.weights = { ...profile.weights };
  renderProfileResult(elements.profileResult, state.investorProfile);
  setFeedback(elements.feedback, `Perfil ${profile.label} definido. Pesos atualizados automaticamente.`, "success");
}

function handleCurrentSubmit(event) {
  event.preventDefault();
  state.currentInvestments = parseAllocationForm(elements.currentForm);
  setFeedback(elements.feedback, "Valores atuais salvos no estado da aplicacao.", "success");
}

function handleRebalance() {
  const totalWeights = getWeightsTotal(state.weights);
  if (Math.abs(totalWeights - 100) > 0.0001) {
    setFeedback(elements.feedback, "Nao e possivel rebalancear com pesos diferentes de 100%.", "error");
    return;
  }

  const monthlyContribution = state.idealPlan?.monthlyContribution ?? state.monthlyIncome * 0.2;
  const summary = calculateRebalance(state.weights, state.currentInvestments, monthlyContribution);
  renderSummary(elements.summary, summary);
  setFeedback(elements.feedback, "Resumo de rebalanceamento calculado.", "success");
}

function bootstrap() {
  setApiBaseUrl(elements.apiBaseUrl, API_BASE_URL);

  applyCurrentOnForm(state.currentInvestments);

  renderIdealPlan(elements.idealPlan, null, 0, state.investor);
  renderProfileResult(elements.profileResult, state.investorProfile);
  renderSummary(elements.summary, null);

  elements.userForm.addEventListener("submit", handleCreateUser);
  elements.refreshUsersButton.addEventListener("click", loadUsers);
  elements.plannerForm.addEventListener("submit", handlePlannerSubmit);
  elements.profileForm.addEventListener("submit", handleProfileSubmit);
  elements.currentForm.addEventListener("submit", handleCurrentSubmit);
  elements.rebalanceButton.addEventListener("click", handleRebalance);

  loadUsers();
}

bootstrap();
