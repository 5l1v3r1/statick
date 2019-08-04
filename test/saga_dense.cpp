#include "ipp.ipp"
#include "statick/solver/saga.hpp"
constexpr bool INTERCEPT = false;
constexpr size_t N_FEATURES = 1000, N_SAMPLES = 1000, N_ITER = 100, SEED = 1933;
int main() {
  using T        = double;
  using FEATURES = statick::Array2D<T>;
  using LABELS   = statick::Array<T>;
  using PROX     = statick::ProxL2Sq<T>;
  using MODEL    = statick::ModelLogReg<std::shared_ptr<FEATURES>, std::shared_ptr<LABELS>>;
  using SOLVER   = statick::solver::SAGA<MODEL>;
  MODEL::DAO modao(FEATURES::RANDOM(N_SAMPLES, N_FEATURES, SEED),
                   LABELS::RANDOM(N_FEATURES, SEED));
  const T STRENGTH = (1. / n_samples) + 1e-10;
  SOLVER::DAO dao(modao); PROX prox(STRENGTH); std::vector<T> objs; auto start = NOW;
  for (size_t j = 0; j < N_ITER; ++j) {
    SOLVER::SOLVE(dao, modao, prox);
    if (j % 10 == 0) objs.emplace_back(MODEL::LOSS(modao, dao.iterate.data()));
  }
  auto finish = NOW;
  for (auto &o : objs) std::cout << __LINE__ << " " << o << std::endl;
  std::cout << (finish - start) / 1e3 << std::endl;
  return 0;
}
