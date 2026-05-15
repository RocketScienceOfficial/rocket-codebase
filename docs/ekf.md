# Error-State Kalman Filter — Theory

> **Reference:** Joan Solà, "Quaternion Kinematics for the Error-State Kalman Filter," 2017.

---

## 1. Motivation: Why an Error-State Formulation?

Attitude cannot be represented as an unconstrained vector — the set of orientations forms a **Lie group** (SO(3)), and unit quaternions live on the 3-sphere $S^3$. Adding a perturbation directly to a quaternion moves it off the manifold and invalidates the constraint $\|\mathbf{q}\| = 1$.

The error-state EKF (ES-EKF) resolves this by maintaining two parallel representations:

- A **nominal state** $\mathbf{x}$ that evolves on the true nonlinear manifold via the full kinematic equations.
- A **small error state** $\delta\mathbf{x}$ that lives in the local tangent space (a flat Euclidean space) around the nominal. The Kalman filter operates exclusively on this error state.

After each measurement update the error is **injected** back into the nominal state (composition on the manifold) and **reset** to zero, keeping the error small and the linearization valid.

---

## 2. State Representation

### 2.1 Nominal State

$$
\mathbf{x} =
\begin{bmatrix}
\mathbf{q} \\[4pt]
\mathbf{p} \\[4pt]
\mathbf{v} \\[4pt]
\mathbf{m} \\[4pt]
\mathbf{b}_\omega \\[4pt]
\mathbf{b}_a \\[4pt]
\mathbf{b}_m
\end{bmatrix}
\in \mathbb{R}^{22}
\quad
\begin{aligned}
&\mathbf{q} \in S^3 &&\text{unit quaternion, body (FRD) → world (NED)} \\
&\mathbf{p} \in \mathbb{R}^3 &&\text{position, NED [m]} \\
&\mathbf{v} \in \mathbb{R}^3 &&\text{velocity, NED [m/s]} \\
&\mathbf{m} \in \mathbb{R}^3 &&\text{Earth magnetic field, NED [T]} \\
&\mathbf{b}_\omega \in \mathbb{R}^3 &&\text{gyroscope bias, body [rad/s]} \\
&\mathbf{b}_a \in \mathbb{R}^3 &&\text{accelerometer bias, body [m/s}^2] \\
&\mathbf{b}_m \in \mathbb{R}^3 &&\text{magnetometer bias, body [T]}
\end{aligned}
$$

### 2.2 Error State

$$
\delta\mathbf{x} =
\begin{bmatrix}
\delta\boldsymbol{\theta} \\[4pt]
\delta\mathbf{p} \\[4pt]
\delta\mathbf{v} \\[4pt]
\delta\mathbf{m} \\[4pt]
\delta\mathbf{b}_\omega \\[4pt]
\delta\mathbf{b}_a \\[4pt]
\delta\mathbf{b}_m
\end{bmatrix}
\in \mathbb{R}^{21}
$$

The attitude error $\delta\boldsymbol{\theta} \in \mathbb{R}^3$ is a rotation vector (axis-angle) in the local frame. The quaternion's four degrees of freedom collapse to three in the tangent space, giving 21 error states from 22 nominal states.

### 2.3 Multiplicative Attitude Error

The true attitude is the composition of the nominal quaternion with the error quaternion:

$$
\mathbf{q}_{\text{true}} = \delta\mathbf{q} \otimes \hat{\mathbf{q}}
$$

For small $\delta\boldsymbol{\theta}$ the error quaternion is approximated by its first-order Taylor expansion on $S^3$:

$$
\delta\mathbf{q} \approx \begin{bmatrix} 1 \\[2pt] \tfrac{1}{2}\,\delta\boldsymbol{\theta} \end{bmatrix}
$$

All remaining states have additive errors: $\mathbf{x}_{\text{true}} = \hat{\mathbf{x}} + \delta\mathbf{x}$.

---

## 3. Prediction (Time Update)

The prediction step is driven at the IMU rate from delta-angle $\Delta\boldsymbol{\theta}_m$ and delta-velocity $\Delta\mathbf{v}_m$ over interval $\Delta t$.

### 3.1 Nominal State Propagation

$$
\hat{\mathbf{q}}^+ = \hat{\mathbf{q}} \otimes \begin{bmatrix} 1 \\[2pt] \tfrac{1}{2}(\Delta\boldsymbol{\theta}_m - \mathbf{b}_\omega\,\Delta t) \end{bmatrix}, \qquad \hat{\mathbf{q}}^+ \leftarrow \frac{\hat{\mathbf{q}}^+}{\|\hat{\mathbf{q}}^+\|}
$$

$$
\hat{\mathbf{v}}^+ = \hat{\mathbf{v}} + \bigl(\mathbf{R}(\hat{\mathbf{q}})\,(\Delta\mathbf{v}_m - \mathbf{b}_a\,\Delta t) + \mathbf{g}\,\Delta t\bigr)
$$

$$
\hat{\mathbf{p}}^+ = \hat{\mathbf{p}} + \frac{\hat{\mathbf{v}} + \hat{\mathbf{v}}^+}{2}\,\Delta t
$$

$$
\hat{\mathbf{b}}^+ = \hat{\mathbf{b}} \quad \text{(all bias groups — random walk, noise added via $\mathbf{Q}$)}
$$

where $\mathbf{R}(\hat{\mathbf{q}}) \in SO(3)$ is the rotation matrix corresponding to $\hat{\mathbf{q}}$ and $\mathbf{g} = [0,\, 0,\, g]^T$ is the NED gravity vector.

### 3.2 Error-State Dynamics

Linearizing the true dynamics around the nominal trajectory yields the continuous-time error-state equation:

$$
\delta\dot{\mathbf{x}} = \mathbf{F}_c\,\delta\mathbf{x} + \mathbf{G}_c\,\mathbf{n}
$$

The non-zero blocks of $\mathbf{F}_c$ that couple states are:

$$
\frac{\partial\,\delta\dot{\boldsymbol{\theta}}}{\partial\,\delta\boldsymbol{\theta}} = -\lfloor\boldsymbol{\omega}\rfloor_\times, \qquad
\frac{\partial\,\delta\dot{\boldsymbol{\theta}}}{\partial\,\delta\mathbf{b}_\omega} = -\mathbf{I}
$$

$$
\frac{\partial\,\delta\dot{\mathbf{v}}}{\partial\,\delta\boldsymbol{\theta}} = -\mathbf{R}\,\lfloor\mathbf{a}\rfloor_\times, \qquad
\frac{\partial\,\delta\dot{\mathbf{v}}}{\partial\,\delta\mathbf{b}_a} = -\mathbf{R}
$$

$$
\frac{\partial\,\delta\dot{\mathbf{p}}}{\partial\,\delta\mathbf{v}} = \mathbf{I}
$$

where $\boldsymbol{\omega} = \Delta\boldsymbol{\theta}_m/\Delta t - \mathbf{b}_\omega$ and $\mathbf{a} = \Delta\mathbf{v}_m/\Delta t - \mathbf{b}_a$ are the bias-corrected angular velocity and acceleration, and $\lfloor\cdot\rfloor_\times$ denotes the skew-symmetric (cross-product) matrix.

The noise vector $\mathbf{n} = [\mathbf{n}_\omega,\, \mathbf{n}_a,\, \mathbf{n}_{b\omega},\, \mathbf{n}_{ba},\, \mathbf{n}_{bm}]^T$ contains process noise for each random-walk model.

### 3.3 Covariance Prediction

The discrete-time transition and noise Jacobians are computed via first-order discretization:

$$
\mathbf{F} \approx \mathbf{I} + \mathbf{F}_c\,\Delta t, \qquad \mathbf{G} = \mathbf{G}_c\,\Delta t
$$

The covariance propagates as:

$$
\mathbf{P}^+ = \mathbf{F}\,\mathbf{P}\,\mathbf{F}^T + \mathbf{G}\,\mathbf{Q}\,\mathbf{G}^T
$$

where $\mathbf{Q} = \operatorname{diag}(\sigma_\omega^2\mathbf{I},\; \sigma_a^2\mathbf{I},\; \sigma_{b\omega}^2\mathbf{I},\; \sigma_{ba}^2\mathbf{I},\; \sigma_{bm}^2\mathbf{I})$ is the process noise covariance. $\mathbf{P}$ is stored as an upper-triangular 21×21 matrix, exploiting symmetry.

---

## 4. Correction (Measurement Update)

### 4.1 Scalar Sequential Updates

Each measurement $z_i \in \mathbb{R}$ is treated as an independent 1-DOF observation with noise variance $r_i$. Given the linearized observation row vector $\mathbf{h}_i \in \mathbb{R}^{1 \times 21}$, the update proceeds as:

**Innovation:**
$$
\nu_i = z_i - h_i(\hat{\mathbf{x}})
$$

**Innovation variance:**
$$
S_i = \mathbf{h}_i\,\mathbf{P}\,\mathbf{h}_i^T + r_i \quad \in \mathbb{R}
$$

**Scalar Kalman gain:**
$$
\mathbf{K}_i = \frac{\mathbf{P}\,\mathbf{h}_i^T}{S_i} \quad \in \mathbb{R}^{21}
$$

**Error-state update:**
$$
\delta\hat{\mathbf{x}} \;\mathrel{+}=\; \mathbf{K}_i\,\nu_i
$$

**Covariance update** (Joseph form for numerical stability):
$$
\mathbf{P} \;\mathrel{-}=\; \mathbf{K}_i\,\mathbf{h}_i\,\mathbf{P}
$$

Sequential 1-DOF updates avoid the $n \times n$ matrix inversion that a joint update would require.

### 4.2 Innovation Gate

Before accepting a measurement, a chi-square consistency check is applied. For a scalar observation the gate is:

$$
\nu_i^2 < \gamma^2\, S_i, \qquad \gamma = 6 \quad (6\sigma\text{ threshold})
$$

Measurements failing the gate are discarded as outliers.

### 4.3 Observation Models

| Sensor | Scalar observation $z_i$ | Predicted observation $h_i(\hat{\mathbf{x}})$ |
|---|---|---|
| GPS position (×3) | $p_j^{\text{GPS}}$ | $\hat{p}_j$ |
| GPS velocity (×3) | $v_j^{\text{GPS}}$ | $\hat{v}_j$ |
| Barometer (×1) | $p_d^{\text{baro}}$ | $\hat{p}_d$ |
| Magnetometer (×3) | $m_j^{\text{body}}$ | $\bigl[\mathbf{R}(\hat{\mathbf{q}})^T(\hat{\mathbf{m}} + \hat{\mathbf{b}}_m)\bigr]_j$ |

The magnetometer prediction $\mathbf{R}^T(\hat{\mathbf{m}} + \hat{\mathbf{b}}_m)$ rotates the estimated Earth field from NED into body frame, so $\mathbf{h}_i$ is non-zero in the attitude, Earth-field, and body-bias blocks simultaneously.

### 4.4 Error Injection and Reset

After all observations for a given time step have been fused, the accumulated error state $\delta\hat{\mathbf{x}}$ is injected into the nominal state and then reset:

**Attitude** (multiplicative injection):
$$
\hat{\mathbf{q}} \leftarrow \delta\mathbf{q}\bigl(\delta\hat{\boldsymbol{\theta}}\bigr) \otimes \hat{\mathbf{q}}, \qquad \hat{\mathbf{q}} \leftarrow \frac{\hat{\mathbf{q}}}{\|\hat{\mathbf{q}}\|}
$$

**All other states** (additive injection):
$$
\hat{\mathbf{x}}_{\text{other}} \leftarrow \hat{\mathbf{x}}_{\text{other}} + \delta\hat{\mathbf{x}}_{\text{other}}
$$

**Reset:**
$$
\delta\hat{\mathbf{x}} \leftarrow \mathbf{0}
$$

The covariance $\mathbf{P}$ requires a corresponding reset Jacobian $\mathbf{G}_{\text{reset}}$ (Solà §7.4), but to first order $\mathbf{G}_{\text{reset}} \approx \mathbf{I}_{21}$, so $\mathbf{P}$ is left unchanged after injection.

---

## 5. Delay-Horizon Fusion

### 5.1 The Problem

Physical sensors have non-negligible and unequal measurement latencies $\tau_s$ (e.g. GPS pipeline delay, filter group delays). Fusing a measurement as if it were instantaneous introduces a systematic error proportional to $\tau_s$ times the rate of change of the state — significant at high dynamics.

### 5.2 Delayed Filter Approach

The filter is run on an internal timeline that lags real time by a fixed horizon $\tau_H \geq \max_s(\tau_s)$:

$$
t_{\text{filter}} = t_{\text{real}} - \tau_H
$$

All sensor observations and IMU samples are stored in timestamped ring buffers. At each step the filter processes only measurements whose timestamp satisfies $t_{\text{obs}} \leq t_{\text{filter}}$.

A measurement with sensor latency $\tau_s$ that is physically captured at $t_{\text{real}} - \tau_s$ arrives at the filter at real-world time $t_{\text{real}}$, which corresponds to filter time $t_{\text{filter}} + \tau_H - \tau_s$. Since $\tau_H \geq \tau_s$, this time is always $\geq t_{\text{filter}}$, guaranteeing that every sensor can be fused at the correct point in the filter's causal timeline with no extrapolation.

The filter holds back fusion until the IMU ring buffer spans the full horizon $\tau_H$, ensuring a complete IMU history is available for any replay needed.

### 5.3 Output Predictor

Running the filter at $t_{\text{filter}} = t_{\text{real}} - \tau_H$ would make state estimates stale by $\tau_H$. The output predictor closes this gap by maintaining a **dead-reckoning state** $\tilde{\mathbf{x}}(t)$ that is propagated forward from $t_{\text{filter}}$ to $t_{\text{real}}$ using buffered IMU measurements:

$$
\tilde{\mathbf{x}}(t_{\text{real}}) = f_{\text{IMU}}\!\left(\hat{\mathbf{x}}(t_{\text{filter}}),\; \{\Delta\boldsymbol{\theta}_k, \Delta\mathbf{v}_k\}_{t_{\text{filter}}}^{t_{\text{real}}}\right)
$$

Each time the EKF completes a fusion update it produces a correction $\Delta\mathbf{x}^* = \hat{\mathbf{x}}^+_{\text{filter}} - \hat{\mathbf{x}}^-_{\text{filter}}$. Applying this correction as a step to the output state would introduce a discontinuity. Instead it is blended in with an exponential complementary filter over the horizon window. For each state group $\ell$ with time constant $\tau_\ell$:

$$
\boldsymbol{\epsilon}_\ell(t) = \Delta\mathbf{x}^*_\ell \cdot e^{-(t - t_{\text{filter}})/\tau_\ell}
$$

$$
\tilde{\mathbf{x}}_\ell(t) = \tilde{\mathbf{x}}_{\text{DR},\ell}(t) + \boldsymbol{\epsilon}_\ell(t)
$$

where $\tilde{\mathbf{x}}_{\text{DR}}$ is the pure dead-reckoning propagation. The time constants $\tau_\ell$ are chosen to be commensurate with $\tau_H$ so that corrections decay to zero by the time the next fusion update arrives, preventing error accumulation in the output path.

---

## 6. Software Optimizations

The following optimizations reduce memory footprint and runtime arithmetic without changing the mathematical result of the filter.

### 6.1 Triangular Covariance Storage

The covariance matrix $\mathbf{P}$ is symmetric by construction: $P_{ij} = P_{ji}$ for all $i, j$. Storing only the upper triangle requires

$$
\frac{n(n+1)}{2} = \frac{21 \times 22}{2} = 231 \text{ floats}
$$

instead of $n^2 = 441$, a 47 % reduction. The element at logical position $(i, j)$ maps to the flat array index

$$
k(i,j) = i\,n - \frac{(i-1)\,i}{2} + j - i, \qquad i \leq j
$$

and access with $i > j$ is redirected to $k(j, i)$ via symmetry. All downstream computations — $\mathbf{K} = \mathbf{P}\mathbf{h}^T/S$ and the rank-1 downdate $\mathbf{P} \mathrel{-}= \mathbf{K}\mathbf{h}\mathbf{P}$ — exploit this layout so that every load resolves to the canonical upper-triangle slot without a branch.

The downdate also only writes the upper triangle. Writing $\mathbf{P} \mathrel{-}= \mathbf{K}\mathbf{h}\mathbf{P}$ in full produces a rank-1 symmetric result; computing only the $\frac{n(n+1)}{2}$ upper entries halves the number of scalar multiply-accumulates in each measurement update.

### 6.2 Closed-Form Covariance Prediction via Symbolic Derivation

The covariance prediction $\mathbf{P}^+ = \mathbf{F}\mathbf{P}\mathbf{F}^T + \mathbf{G}\mathbf{Q}\mathbf{G}^T$ naïvely requires two triple matrix products. Each costs $O(n^3)$ multiply-accumulates; for $n = 21$ this amounts to roughly $2 \times 21^3 \approx 18{,}500$ scalar operations per IMU step.

Instead, the symbolic algebra system computes $\mathbf{F}\mathbf{P}\mathbf{F}^T + \mathbf{G}\mathbf{Q}\mathbf{G}^T$ **once, at derivation time**, with $\mathbf{P}$, the state variables, and $\Delta t$ treated as symbolic quantities. The result is a set of $\frac{n(n+1)}{2} = 231$ closed-form scalar polynomials — one per upper-triangle element of $\mathbf{P}^+$ — which are emitted as C expressions. At runtime the filter evaluates these expressions directly, with each $P^+_{ij}$ computed as a sum of products of the current $P_{kl}$ entries.

The algebraic complexity of each expression (determined by the sparsity of $\mathbf{F}$ and $\mathbf{G}$, not by $n$) is much lower than the naïve $O(n^3)$ count. Only the upper triangle is emitted; the lower triangle is dropped symbolically before code generation, preventing redundant writes.

The same principle applies to every scalar measurement update: $S_i = \mathbf{h}_i\mathbf{P}\mathbf{h}_i^T + r_i$ and $\mathbf{K}_i = \mathbf{P}\mathbf{h}_i^T / S_i$ are derived symbolically and emitted as closed-form expressions, with zero runtime matrix construction.

### 6.3 First-Order Discretization and $O(\Delta t^2)$ Elimination

The continuous-time error-state Jacobian $\mathbf{F}_c$ is discretized as $\mathbf{F} \approx \mathbf{I} + \mathbf{F}_c\,\Delta t$, discarding terms of order $\Delta t^2$ and higher. This is exact at the symbolic level: during derivation, all $\Delta t^2$ terms in the expanded error-state prediction are explicitly substituted to zero before extracting $\mathbf{F}$.

The practical consequence is that many off-diagonal blocks of $\mathbf{F}$ that would be non-zero under a matrix-exponential discretization ($e^{\mathbf{F}_c \Delta t}$) remain zero under first-order truncation. The resulting $\mathbf{F}\mathbf{P}\mathbf{F}^T$ has fewer non-zero symbolic terms, so the generated expressions are shorter and execute faster.

At 250 Hz the step size is $\Delta t = 4\,\text{ms}$; truncation error per step scales as $\tfrac{1}{2}\|\mathbf{F}_c\|^2\Delta t^2$, which is negligible relative to sensor noise.

### 6.4 Unit-Quaternion Identity Simplification

After computing the error-state prediction symbolically, the quaternion normalization constraint $\|\mathbf{q}\|^2 = q_w^2 + q_x^2 + q_y^2 + q_z^2 = 1$ is substituted into every expression before code generation. Combinations like $2q_w^2 + 2q_x^2 + 2q_y^2 + 2q_z^2$ collapse to the constant 2, cancelling entire sub-expressions. This reduces both the generated expression length and the number of state-variable loads required at runtime.

### 6.5 Chain-Rule Composition for Measurement Jacobians

The observation function $h(\mathbf{x})$ is written in terms of the **nominal state** $\hat{\mathbf{x}}$, but the Kalman gain requires the Jacobian with respect to the **error state** $\delta\mathbf{x}$. These are related by the chain rule:

$$
\mathbf{H} = \frac{\partial h}{\partial \mathbf{x}}\,\frac{\partial \mathbf{x}}{\partial \delta\mathbf{x}}
$$

The second factor $\frac{\partial \mathbf{x}}{\partial \delta\mathbf{x}}$ captures the manifold geometry — in particular, the mapping from the 3-vector attitude error $\delta\boldsymbol{\theta}$ to the 4-vector quaternion perturbation. Both partial derivatives are computed symbolically in a single composition step, yielding the exact $\mathbf{h}_i$ vector for each scalar observation. No finite-difference approximation or runtime chain-rule evaluation is needed.

### 6.6 Static Memory Allocation

All filter state — the nominal state vector, the covariance upper triangle, and all sensor ring buffers — is allocated statically at compile time. There is no heap use. The consequences are:

- **Deterministic latency.** Every prediction and update step accesses memory at fixed addresses; there are no allocator calls, no fragmentation, and no worst-case-execution-time (WCET) uncertainty from dynamic allocation.
- **Stack depth known at compile time.** The MCU's limited RAM can be fully analysed statically; no runtime out-of-memory failure mode exists.
- **Cache-friendly layout.** All hot data ($\mathbf{P}$, $\hat{\mathbf{x}}$, IMU ring buffer) is laid out contiguously in BSS, maximising spatial locality.

---

## 7. Comparison with PX4 EKF2

PX4 EKF2 (ECL) is the reference implementation used in commercial autopilots. It uses the same ES-EKF framework with an identical theoretical foundation (Solà 2017, multiplicative quaternion error, scalar sequential updates, ring-buffer delay compensation). The structural differences are driven by scope, not algorithm:

| | Rocket EKF | PX4 EKF2 |
|---|---|---|
| Error states | 21 | 23 |
| Extra states | — | 2 horizontal wind velocity states |
| Sensor suite | IMU, GPS, baro, mag | + optical flow, range finder, airspeed, external vision |
| Redundancy | Single instance | Multiple instances run in parallel; best selected |
| Noise model | Fixed $\mathbf{Q}$, $\mathbf{R}$ | Innovation-based adaptive covariance scaling |
| Yaw fallback | Magnetometer | GPS-velocity heading estimator |

Wind states are irrelevant for a vertically-launched rocket. The 21-state design is fully observable with the available sensor suite and is easier to verify on constrained hardware.
