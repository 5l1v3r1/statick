import scipy, numpy as np
import statick.solver as statick_solver
from tick.solver import SAGA as TS
from tick.prox.base.prox import Prox as TPROX
from tick.base_model import ModelGeneralizedLinear as TMGL

def MODEL_CFUNC_RESOLVER(model, s = ""):
    X = model.features
    C = "s" if isinstance(X, scipy.sparse.csr.csr_matrix) else "d"
    T = "d" if X.dtype == np.dtype('float64') else "s"
    return model._MANGLING + s + C + T

class DummySAGA(TS):
    def _set_cpp_solver(self, dtype_or_object_with_dtype): return None
    def set_epoch_size(self, v): pass
    def set_rand_max(self, v): pass
    def set_model(self, model: TMGL): pass

import sys, inspect
print("__name__", __name__)
def print_classes():
    for name, obj in inspect.getmembers("statick.solver"):
        if inspect.isclass(obj):
            print(obj)

class SAGA(DummySAGA):

    def __init__(self, **kwargs):
        TS.__init__(self, **kwargs)
        object.__setattr__(self, "_solver", DummySAGA())
        object.__setattr__(self, "_dao", None)

    def set_model(self, model: TMGL):
        TS.set_model(self, model)
        func = "saga_" + MODEL_CFUNC_RESOLVER(model, "_dao_")
        object.__setattr__(self, "_dao", getattr(statick_solver, func)(model._dao))
        return self

    def set_prox(self, prox: TPROX):
        object.__setattr__(self, "_prox", prox)
        return self

    def solve(self):
        getattr(statick_solver, "solve_saga_" + MODEL_CFUNC_RESOLVER(self.model, "_" + self._prox._MANGLING + "_"))(self._dao, self.model._dao, self._prox._dao)
