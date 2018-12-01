#ifndef TICK_SOLVER_SVRG_HPP_
#define TICK_SOLVER_SVRG_HPP_

#include <thread>
#include "statick/solver/history.hpp"

namespace statick {
namespace svrg {
namespace VarianceReductionMethod {
constexpr uint16_t Last = 1, Average = 2, Random = 3;
}
namespace StepType {
constexpr uint16_t Fixed = 1, BarzilaiBorwein = 2;
}
template <typename MODEL, uint16_t RM = VarianceReductionMethod::Last,
          uint16_t ST = StepType::Fixed, bool INTERCEPT = false, typename NEXT_I,
          typename T = typename MODEL::value_type,
          typename DAO>
void prepare_solve(DAO &dao, typename MODEL::DAO &modao, size_t &t, NEXT_I fn_next_i) {
  const size_t n_samples = modao.n_samples(), n_features = modao.n_features();
  std::vector<T> previous_iterate, previous_full_gradient;
  size_t iterate_size = n_features + static_cast<size_t>(INTERCEPT);
  if constexpr(ST == StepType::BarzilaiBorwein) {
    if (t > 1) {
      previous_iterate = dao.fixed_w;
      previous_full_gradient = dao.full_gradient;
    }
  }
  dao.next_iterate = std::vector<T>(iterate_size, 0);
  copy(dao.iterate.data(), dao.next_iterate.data(), iterate_size);
  dao.fixed_w = dao.next_iterate;
  dao.full_gradient = std::vector<T>(iterate_size, 0);
  MODEL::template grad<INTERCEPT>(modao, dao.fixed_w.data(), dao.full_gradient.data(), iterate_size);
  if constexpr(ST == StepType::BarzilaiBorwein) {
    if (t > 1) {
      std::vector<T> iterate_diff = dao.next_iterate;
      mult_incr(iterate_diff.data(), previous_iterate.data(), -1, iterate_size);
      std::vector<T> full_gradient_diff = dao.full_gradient;
      mult_incr(full_gradient_diff.data(), previous_full_gradient.data(), -1, iterate_size);
      dao.step = 1. / n_samples * norm_sq(iterate_diff.data(), iterate_size) /
                 dot(iterate_diff.data(), full_gradient_diff.data(), iterate_size);
    }
  }
  dao.grad_i = std::vector<T>(iterate_size);
  dao.grad_i_fixed_w = std::vector<T>(iterate_size);
  if constexpr(RM == VarianceReductionMethod::Random || RM == VarianceReductionMethod::Average)
    std::fill(dao.next_iterate.begin(), dao.next_iterate.end(), 0);
  dao.rand_index = 0;
  if constexpr(RM == VarianceReductionMethod::Random) dao.rand_index = fn_next_i();
}
}  // namespace svrg
}  // namespace statick

#include "statick/solver/svrg/dense.hpp"
#include "statick/solver/svrg/sparse.hpp"

#endif  // TICK_SOLVER_SVRG_HPP_
