import numpy as np
from matplotlib import pyplot as plt
from matplotlib.widgets import Slider


def reRange(OldValue, OldMin, OldMax, NewMin, NewMax):
	OldRange = (OldMax - OldMin)
	if (OldRange == 0):
		NewValue = NewMin
	else :
		NewRange = (NewMax - NewMin)  
		NewValue = (((OldValue - OldMin) * NewRange) / OldRange) + NewMin
	return NewValue


def wheels_points(angles, frame_radius=8.2):
	angles = np.radians(angles)
	x = frame_radius * np.cos(angles)
	y = frame_radius * np.sin(angles)
	return x, y

def frame_points(frame_radius=8.2):
	angles = np.linspace(0, 2 * np.pi, 360)
	x = frame_radius * np.cos(angles)
	y = frame_radius * np.sin(angles)
	return x, y

def J(angles, wheel_radius=2.5, frame_radius=8.2):
	angles = np.radians(angles)
	return np.array([[-np.sin(a), np.cos(a), frame_radius] for a in angles]) / wheel_radius

def get_rotation_vector(angular_speed, point_x, point_y, frame_radius=8.2):
	length = frame_radius / 3
	vector_points = np.zeros((4, 2, 2))
	
	for i in range(4):
		vector_points[i, 0] = [point_x[i], point_y[i]]
		if point_y[i] != 0:
			slope = -point_x[i] / point_y[i]
			dx = length if (i < 2) ^ (angular_speed[i] < 0) else -length
			dy = slope * dx
			
			end_point = np.array([dx, dy])
			# get a normalize vector
			long = reRange(abs(angular_speed[i]), 0, 0.5, 0, 10)
			end = long *end_point[:] / np.linalg.norm(end_point[:])
						
			vector_points[i, 1] = [point_x[i] + end[0], point_y[i] + end[1]]
		else:
			vector_points[i, 1] = [point_x[i] + (length if angular_speed[i] > 0 else -length), point_y[i]]
	
	return vector_points

def update(val):
	angular_speed[:] = [s1.val, s2.val, s3.val, s4.val]
	vector_points[:] = get_rotation_vector(angular_speed, x, y)
	v[:] = j_pinv @ angular_speed
	vitesse_vector[:] = 10*v[:2]
	
	vitesse_arrow.set_data([0, vitesse_vector[0]], [0, vitesse_vector[1]])
	rot_speed_text.set_text(f"Robot Rotational Speed: {v[-1]:.2f}")
 
	for i in range(4):
		wheel_vectors[i].set_data([vector_points[i, 0, 0], vector_points[i, 1, 0]],
								  [vector_points[i, 0, 1], vector_points[i, 1, 1]])
	fig.canvas.draw_idle()




if __name__ == '__main__':

	angles = [30, 180 - 30, 180 + 45, -45]
	angular_speed = [0, 0, 0, 0]

	x, y = wheels_points(angles)
	frame_x, frame_y = frame_points()
	vector_points = get_rotation_vector(angular_speed, x, y)

	fig, ax = plt.subplots()
	plt.subplots_adjust(left=0.1, bottom=0.3)
	ax.set_box_aspect(1)
	ax.plot(frame_x, frame_y, 'k')
	ax.set_xlim(-30, 30)
	ax.set_ylim(-30, 30)
	ax.set_axis_off()

	rot_speed_text = ax.text(0, -28, "Robot Rotational Speed: 0.00", fontsize=11, color='black', ha='center')

	wheel_vectors = [ax.plot([], [], 'r')[0] for _ in range(4)]
	vitesse_arrow, = ax.plot([], [], 'g')

	j = J(angles)
	j_pinv = np.linalg.pinv(j)
	v = np.zeros(3)
	vitesse_vector = np.zeros(2)

	ax_slider1 = plt.axes([0.1, 0.15, 0.65, 0.03])
	ax_slider2 = plt.axes([0.1, 0.10, 0.65, 0.03])
	ax_slider3 = plt.axes([0.1, 0.05, 0.65, 0.03])
	ax_slider4 = plt.axes([0.1, 0.00, 0.65, 0.03])

	s1 = Slider(ax_slider1, "Wheel 1", -0.5, 0.5, valinit=0)
	s2 = Slider(ax_slider2, "Wheel 2", -0.5, 0.5, valinit=0)
	s3 = Slider(ax_slider3, "Wheel 3", -0.5, 0.5, valinit=0)
	s4 = Slider(ax_slider4, "Wheel 4", -0.5, 0.5, valinit=0)

	s1.on_changed(update)
	s2.on_changed(update)
	s3.on_changed(update)
	s4.on_changed(update)

	plt.show()
