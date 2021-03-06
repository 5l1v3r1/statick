import scipy, numpy as np
from statick.solver.solver import Solver as SOLVER
from tick.solver import SAGA as TS
from tick.prox.base.prox import Prox as TPROX
from tick.base_model import ModelGeneralizedLinear as TMGL

from .solver import *

class SAGA(SOLVER, TS):

    def __init__(self, **kwargs):
        TS.__init__(self, **kwargs)
        SOLVER.__init__(self, **kwargs)
        object.__setattr__(self, "_s_name", "saga")
        if self.n_threads > 1:
            object.__setattr__(self, "_s_name", "asaga")

    def set_model(self, model: TMGL):
        return SOLVER.set_model(self, SUPER=TS, model=model)
