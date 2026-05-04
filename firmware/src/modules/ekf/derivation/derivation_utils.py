from sympy import ccode, cse, Symbol, Matrix
from sympy.codegen.rewriting import create_expand_pow_optimization
import os


def quaternion_multiply(q1, q2):
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2

    w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2
    x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2
    y = w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2
    z = w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2

    return Matrix([w, x, y, z])


def quaternion_conjugate(q):
    w, x, y, z = q

    return Matrix([w, -x, -y, -z])


def quaternion_rot_matrix(q):
    w, x, y, z = q

    return Matrix(
        [
            [1 - 2 * (y**2 + z**2), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x**2 + z**2), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x**2 + y**2)],
        ]
    )


def save_file(name, content):
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "generated", name)

    with open(path, "w") as f:
        f.write(content)

    print(f"Generated {name}.")


def gen_expression(code, state, exprs, get_next_name):
    state_mapping = {}
    idx = 0

    for s in state.values():
        for v in s:
            state_mapping[v] = Symbol(f"state[{idx}]", real=True)
            idx += 1

    for i in range(len(exprs)):
        exprs[i] = exprs[i].subs(state_mapping)

    replacements, reduced = cse(exprs, optimizations="basic")
    expand_opt = create_expand_pow_optimization(5)

    for var, expr in replacements:
        code.append(f"    const float {var} = {ccode(expand_opt(expr))};")

    for i in range(len(reduced)):
        for idx, expr in enumerate(reduced[i]):
            name = get_next_name(i, idx)

            if name is not None:
                code.append(f"    {name} = {ccode(expand_opt(expr))};")


def write_cov_matrix(name, state, P):
    n = P.shape[0]
    code = []

    code.append("#pragma once")
    code.append("")
    code.append("namespace gen {")
    code.append("")

    code.append(f"void {name}(")
    code.append(f"    const float *state,")
    code.append(f"    const CovarianceMatrix<EKF_NUM_ERROR_STATES>& P,")
    code.append(f"    CovarianceMatrix<EKF_NUM_ERROR_STATES>& P_next,")
    code.append(f"    float var_acc,")
    code.append(f"    float accel_x,")
    code.append(f"    float accel_y,")
    code.append(f"    float accel_z,")
    code.append(f"    float var_gyro,")
    code.append(f"    float gyro_x,")
    code.append(f"    float gyro_y,")
    code.append(f"    float gyro_z,")
    code.append(f"    float dt")
    code.append(") {")

    def get_next_name(_, idx):
        i = idx // n
        j = idx % n

        if j >= i:
            return f"P_next({i}, {j})"
        else:
            return None

    gen_expression(code, state, [P], get_next_name)

    code.append("}")
    code.append("}")

    save_file(f"{name}.h", "\n".join(code))


def write_obs_eqs(name, state, var, innov, innov_var, H, K):
    code = []

    code.append("#pragma once")
    code.append("")
    code.append("namespace gen {")
    code.append("")

    code.append(f"void {name}(")
    code.append(f"    const float *state,")
    code.append(f"    const CovarianceMatrix<EKF_NUM_ERROR_STATES>& P,")
    code.append(f"    float meas,")
    code.append(f"    float {var.name},")
    code.append(f"    float* innov,")
    code.append(f"    float* innov_var,")
    code.append(f"    float* H,")
    code.append(f"    float* K")
    code.append(") {")

    def get_next_name(i, idx):
        if i == 0:
            return f"*innov"
        elif i == 1:
            return f"*innov_var"
        elif i == 2:
            return f"H[{idx}]"
        elif i == 3:
            return f"K[{idx}]"
        else:
            return None

    gen_expression(code, state, [innov, innov_var, H, K], get_next_name)

    code.append("}")
    code.append("}")

    save_file(f"{name}.h", "\n".join(code))
