"""
Resources:
    - https://github.com/PX4/PX4-ECL/blob/master/EKF/python/ekf_derivation/code_gen.py
"""

from sympy import ccode, cse, symbols, Matrix
from sympy.codegen.ast import float32, real
from sympy.codegen.rewriting import create_expand_pow_optimization
import os


class CodeGenerator:
    def __init__(self, file_name):
        path = os.path.dirname(os.path.abspath(__file__))
        file_dir = os.path.join(path, "generated")

        if not os.path.exists(file_dir):
            os.makedirs(file_dir)

        file_path = os.path.join(file_dir, file_name)

        self.file = open(file_path, "w")

    def get_ccode(self, expression):
        expand_opt = create_expand_pow_optimization(5)

        return ccode(expand_opt(expression), type_aliases={real: float32})

    def write_subexpressions(self, subexpressions):
        write_string = ""

        for item in subexpressions:
            write_string = write_string + "const float " + str(item[0]) + " = " + self.get_ccode(item[1]) + ";\n"

        if len(subexpressions) > 0:
            write_string += "\n"

        return write_string

    def write_matrix(self, matrix, variable_name, is_symmetric=False, pre_bracket="[", post_bracket="]", separator="]["):
        write_string = ""

        for i in range(0, matrix.shape[0]):
            for j in range(0, matrix.shape[1]):
                if j >= i or not is_symmetric:
                    write_string = write_string + variable_name + pre_bracket + str(i) + separator + str(j) + post_bracket + " = " + self.get_ccode(matrix[i, j]) + ";\n"

        write_string += "\n\n"

        return write_string

    def flush(self, s):
        s = s.replace("F", "f")
        s = s.replace("P[", "data->P[")
        s = s.replace("P_Next[", "data->P_Next[")

        self.file.write(s)
        self.file.close()


def write_cov_matrix(name, P):
    P_new = cse(P, symbols("PS0:10000"), optimizations="basic")

    gen = CodeGenerator(f"{name}.c")

    s = ""
    s += gen.write_subexpressions(P_new[0])
    s += gen.write_matrix(Matrix(P_new[1]), "P_Next", True)

    gen.flush(s)


def write_obs_eqs(name, HK):
    H = cse(HK[0], symbols("HS0:10000"), optimizations="basic")
    K = cse(HK[1], symbols("KS0:10000"), optimizations="basic")

    gen = CodeGenerator(f"{name}.c")

    s = ""
    s += gen.write_subexpressions(H[0])
    s += gen.write_matrix(H[1][0], "H")
    s += gen.write_subexpressions(K[0])
    s += gen.write_matrix(K[1][0], "K")

    gen.flush(s)
