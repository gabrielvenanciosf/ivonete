export const CATEGORIES = [
  { key: "rendaFixa", label: "Renda fixa" },
  { key: "acoes", label: "Acoes" },
  { key: "fiis", label: "FIIs" },
  { key: "exterior", label: "Exterior" },
  { key: "reserva", label: "Reserva" }
];

export const defaultWeights = {
  rendaFixa: 40,
  acoes: 25,
  fiis: 15,
  exterior: 10,
  reserva: 10
};

export const profileWeights = {
  conservador: {
    rendaFixa: 65,
    acoes: 10,
    fiis: 10,
    exterior: 5,
    reserva: 10
  },
  moderado: { ...defaultWeights },
  arrojado: {
    rendaFixa: 25,
    acoes: 40,
    fiis: 15,
    exterior: 15,
    reserva: 5
  }
};

export const state = {
  users: [],
  investor: {
    name: "",
    age: 0
  },
  investorProfile: null,
  monthlyIncome: 0,
  idealPlan: null,
  weights: { ...defaultWeights },
  currentInvestments: {
    rendaFixa: 0,
    acoes: 0,
    fiis: 0,
    exterior: 0,
    reserva: 0
  }
};

export function normalizeNumber(value) {
  const number = Number(value);
  if (!Number.isFinite(number) || number < 0) {
    return 0;
  }
  return number;
}

export function sumObjectValues(obj) {
  return Object.values(obj).reduce((acc, value) => acc + normalizeNumber(value), 0);
}

export function parseAllocationForm(form) {
  const data = new FormData(form);
  return CATEGORIES.reduce((acc, item) => {
    acc[item.key] = normalizeNumber(data.get(item.key));
    return acc;
  }, {});
}

export function getWeightsTotal(weights) {
  return sumObjectValues(weights);
}

export function evaluateInvestorProfile(answers) {
  const normalizedAnswers = answers.map((value) => {
    const number = Number(value);
    if (!Number.isFinite(number)) {
      return 0;
    }
    return Math.max(1, Math.min(3, Math.round(number)));
  });

  const score = normalizedAnswers.reduce((acc, value) => acc + value, 0);
  let key = "moderado";
  let label = "Moderado";

  if (score <= 8) {
    key = "conservador";
    label = "Conservador";
  } else if (score >= 12) {
    key = "arrojado";
    label = "Arrojado";
  }

  return {
    key,
    label,
    score,
    minScore: 5,
    maxScore: 15,
    answers: normalizedAnswers,
    weights: { ...profileWeights[key] }
  };
}

export function getSuggestedPlan(monthlyIncome) {
  const income = normalizeNumber(monthlyIncome);
  const distribution = [
    { area: "Moradia", percent: 25 },
    { area: "Alimentacao", percent: 10 },
    { area: "Transporte", percent: 5 },
    { area: "Saude", percent: 5 },
    { area: "Educacao", percent: 5 },
    { area: "Lazer", percent: 20 },
    { area: "Outros", percent: 10 },
    { area: "Investimentos", percent: 20 }
  ];

  const rows = distribution.map((item) => ({
    ...item,
    idealValue: income * (item.percent / 100)
  }));

  const totalExpenses = rows
    .filter((row) => row.area !== "Investimentos")
    .reduce((acc, row) => acc + row.idealValue, 0);

  const investmentRow = rows.find((row) => row.area === "Investimentos");
  const investmentsValue = investmentRow ? investmentRow.idealValue : 0;

  return {
    monthlyContribution: investmentsValue,
    needsPercent: 50,
    lifestylePercent: 30,
    investmentsPercent: 20,
    rows,
    totalExpenses,
    investmentsValue
  };
}

export function calculateRebalance(weights, currentInvestments, monthlyContribution) {
  const currentTotal = sumObjectValues(currentInvestments);
  const plannedContribution = normalizeNumber(monthlyContribution);
  const projectedTotal = currentTotal + plannedContribution;

  const rows = CATEGORIES.map((category) => {
    const currentValue = normalizeNumber(currentInvestments[category.key]);
    const targetPercent = normalizeNumber(weights[category.key]);
    const targetValue = projectedTotal * (targetPercent / 100);
    const delta = targetValue - currentValue;

    return {
      label: category.label,
      currentValue,
      targetPercent,
      targetValue,
      delta
    };
  });

  const totalPositiveDelta = rows.reduce((acc, row) => acc + Math.max(0, row.delta), 0);
  const totalNegativeDelta = rows.reduce((acc, row) => acc + Math.min(0, row.delta), 0);

  return {
    currentTotal,
    plannedContribution,
    projectedTotal,
    totalPositiveDelta,
    totalNegativeDelta,
    rows
  };
}
