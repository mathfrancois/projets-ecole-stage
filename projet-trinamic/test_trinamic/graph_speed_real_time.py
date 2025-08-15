import serial
import struct
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Slider
import time

import io
import random

class FakeSerial:
    def __init__(self, *args, **kwargs):
        self.buffer = io.BytesIO()
        self.current_speeds = [0.0, 0.0, 0.0, 0.0]
        self.target_speeds = [0.0, 0.0, 0.0, 0.0]
        self.header_next = True

    def write(self, data):
        # Décodage des vitesses cibles envoyées par le client
        if data[:2] == struct.pack('BB', 0xAA, 0xBB):
            self.target_speeds = list(struct.unpack('<ffff', data[2:]))

    def read(self, size=1):
        if size == 2:
            return struct.pack('BB', 0xAA, 0xBB)

        elif size == 16:
            # Simule l'évolution progressive des vitesses
            for i in range(4):
                error = self.target_speeds[i] - self.current_speeds[i]
                max_step = 0.5  # Limite de changement de vitesse par lecture (accélération max)
                if abs(error) < max_step:
                    self.current_speeds[i] = self.target_speeds[i]
                else:
                    self.current_speeds[i] += max_step if error > 0 else -max_step

            return struct.pack('<ffff', *self.current_speeds)

        return b'\x00' * size

    def close(self):
        pass

SERIAL_PORT = "/dev/ttyUSB0"
BAUDRATE = 115200

# try:
#     ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
#     print(f"Connexion établie avec {SERIAL_PORT} à {BAUDRATE} bauds")
# except serial.SerialException:
#     print(f"Erreur: Impossible d'ouvrir {SERIAL_PORT}")
#     exit()

try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
    print(f"Connexion établie avec {SERIAL_PORT} à {BAUDRATE} bauds")
except serial.SerialException:
    print(f"Port {SERIAL_PORT} introuvable, simulation activée.")
    ser = FakeSerial()


# Données
temps = []
v1, v2, v3, v4 = [], [], [], []
v1_target, v2_target, v3_target, v4_target = [], [], [], []

# Graphiques
fig, axs = plt.subplots(2, 2, figsize=(10, 6))  # pas de constrained_layout
fig.suptitle("Vitesses des 4 roues")
axes_list = axs.flat
lines = []

for i, ax in enumerate(axes_list):
    line, = ax.plot([], [], label=f"Roue {i+1}")
    ax.set_xlim(0, 5)
    ax.set_ylim(0, 3)
    ax.set_xlabel("Temps (s)")
    ax.set_ylabel("Vitesse (m/s)")
    ax.legend()
    #lines.append(line)
    target_line, = ax.plot([], [], 'r--', label="Cible")  # Ligne rouge en pointillé
    lines.append((line, target_line))  # Tuple: (vitesse mesurée, consigne)

sliders = []
for i, ax in enumerate(axs.flat):
    # Obtenir la position du subplot
    pos = ax.get_position()
    
    # Ajouter un axe pour le slider juste en dessous (on décale y0 un peu)
    slider_ax = fig.add_axes([pos.x0, pos.y0 - 0.05, pos.width, 0.03])
    
    # Créer le slider
    slider = Slider(slider_ax, f'Vitesse {i+1} (m/s)', 0, 2, valinit=0.0)
    sliders.append(slider)

start_time = time.time()
HEADER = struct.pack('BB', 0xAA, 0xBB)  # Header de synchronisation

target_values = [slider.val for slider in sliders]

def send_slider_values():
    values = [s.val for s in sliders]
    data = struct.pack('<ffff', *values)   # Encodage little-endian de 4 floats
    ser.write(HEADER + data)               # Envoi du message complet


def update(frame):
    global target_values

    # Update target_values from sliders
    for i in range(4):
        target_values[i] += 0.1 * (sliders[i].val - target_values[i])

    send_slider_values()

    h = ser.read(len(HEADER))
    if h != HEADER:
        print(f"expected : {HEADER} | received : {h}")

    data = ser.read(16)
    if len(data) == 16:
        s1, s2, s3, s4 = struct.unpack('<ffff', data)
        t = time.time() - start_time
        temps.append(t)
        v1.append(s1)
        v2.append(s2)
        v3.append(s3)
        v4.append(s4)

        v1_target.append(target_values[0])
        v2_target.append(target_values[1])
        v3_target.append(target_values[2])
        v4_target.append(target_values[3])

        all_vs = [v1, v2, v3, v4]
        all_targets = [v1_target, v2_target, v3_target, v4_target]

        for i in range(4):
            line, target_line = lines[i]
            line.set_data(temps, all_vs[i])
            target_line.set_data(temps, all_targets[i])
            axes_list[i].set_xlim(max(0, t - 5), t)

        for ax in axes_list:
            ax.relim()
            ax.autoscale_view()

ani = animation.FuncAnimation(fig, update, interval=16)  # 16ms = ~60 FPS
plt.show()

# stop motors by artificially modifying sliders to send 0 speed
for s in sliders:
    s.val = 0
send_slider_values()
ser.close()
