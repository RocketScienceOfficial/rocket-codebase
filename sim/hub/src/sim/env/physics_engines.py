import numpy as np
import pandas as pd
import struct
from dataclasses import dataclass
from sim.utils import quat, geo
from sim.connection.network import TCPSocket


@dataclass
class PhysicsEngineInput:
    fin_states: np.ndarray
    airbrake_angle: float


@dataclass
class PhysicsEngineOutput:
    acc: np.ndarray  # NED frame
    vel: np.ndarray  # NED frame
    pos: np.ndarray  # NED frame
    w: np.ndarray  # FRD frame
    q: np.ndarray  # FRD -> NED rotation quaternion


class PhysicsEngineInterface:
    def __init__(self, dt: float):
        self.time = 0.0
        self.dt = dt

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        pass

    def finished(self) -> bool:
        pass


class SimpleIntegratorScenarioInterface:
    def get_net_acc(self, time: float, current_state: PhysicsEngineOutput, input: PhysicsEngineInput) -> np.ndarray:
        pass

    def finished(self) -> bool:
        pass


class OpenRocketSimScenario(SimpleIntegratorScenarioInterface):
    def __init__(self, file_path: str, preferred_time: float | None = None):
        df = pd.read_csv(file_path)

        self.times = df[[c for c in df.columns.values if "Time" in c][0]].values
        self.accs = df[[c for c in df.columns.values if "Vertical acceleration" in c][0]].values

        self.time = 0.0
        self.preferred_time = preferred_time

    def get_net_acc(self, time: float, current_state: PhysicsEngineOutput, input: PhysicsEngineInput) -> np.ndarray:
        self.time = time

        # Get acc and convert to NED frame
        return np.interp(time, self.times, self.accs) * np.array([0.0, 0.0, -1.0])

    def finished(self) -> bool:
        return self.time >= (min(self.preferred_time, self.times[-1]) if self.preferred_time is not None else self.times[-1])


class SimpleIntegratorPhysicsEngine(PhysicsEngineInterface):
    def __init__(self, dt: float, scenario: SimpleIntegratorScenarioInterface):
        super().__init__(dt)

        self.acc = np.array([0.0, 0.0, 0.0])
        self.vel = np.array([0.0, 0.0, 0.0])
        self.pos = np.array([0.0, 0.0, 0.0])
        self.w = np.array([0.0, 0.0, 0.0])
        self.q = np.array([1.0, 0.0, 0.0, 0.0])

        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)
        self.scenario = scenario

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        self.acc = self.scenario.get_net_acc(self.time, self.current_state, input)

        self.pos = self.pos + self.vel * self.dt + self.acc * 0.5 * self.dt**2
        self.vel = self.vel + self.acc * self.dt

        omega_q = np.array([0.0, self.w[0], self.w[1], self.w[2]])
        q_dot = 0.5 * quat.quat_multiply(self.q, omega_q)
        self.q = self.q + q_dot * self.dt
        n = np.linalg.norm(self.q)

        if n > 0.0:
            self.q = self.q / n

        self.time += self.dt
        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)

        return self.current_state

    def finished(self) -> bool:
        return self.scenario.finished()


class SimulinkPhysicsEngine(PhysicsEngineInterface):
    def __init__(self, dt: float, model: str):
        super().__init__(dt)

        self.model = model

        self.server = TCPSocket(name="simulink", ip="localhost", port=12360, is_server=True, blocking=True)
        self.is_finished = False

    def _enu_to_ned_v3(self, vec: np.ndarray) -> np.ndarray:
        return np.array([vec[1], vec[0], -vec[2]])

    def _enu_to_ned_q(self, q: np.ndarray) -> np.ndarray:
        return np.array([q[0], q[2], q[1], -q[3]])

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        if self.model == "acs":
            self.server.send_raw(struct.pack("<dd", *input.fin_states[:2]))
        elif self.model == "airbrake":
            self.server.send_raw(struct.pack("<d", input.airbrake_angle))
        else:
            raise ValueError(f"Unknown model: {self.model}")

        data = self.server.receive_raw(128)

        if data is None:
            self.is_finished = True

            return None
        else:
            self.time += self.dt

            return PhysicsEngineOutput(
                acc=self._enu_to_ned_v3(np.array(struct.unpack("<3d", data[0:24]))),
                vel=self._enu_to_ned_v3(np.array(struct.unpack("<3d", data[24:48]))),
                pos=self._enu_to_ned_v3(np.array(struct.unpack("<3d", data[48:72]))),
                w=self._enu_to_ned_v3(np.array(struct.unpack("<3d", data[72:96]))),
                q=self._enu_to_ned_q(np.array(struct.unpack("<4d", data[96:128])))
            )

    def finished(self) -> bool:
        return self.is_finished
