import numpy as np
import pyrr

class OrbitCamera:
    def __init__(self, distance=10.0, pitch=0.0, yaw=0.0, target=np.zeros(3)):
        self.distance = distance
        self.pitch = pitch
        self.yaw = yaw
        self.target = target
        self.update()

    def update(self):
        # Calculate the camera's position based on distance, pitch, and yaw
        cos_pitch = np.cos(np.radians(self.pitch))
        sin_pitch = np.sin(np.radians(self.pitch))
        cos_yaw = np.cos(np.radians(self.yaw))
        sin_yaw = np.sin(np.radians(self.yaw))

        offset = self.distance * np.array([cos_pitch * sin_yaw, sin_pitch, cos_pitch * cos_yaw])
        self.position = self.target + offset

        # Calculate the view matrix
        self.view_matrix = pyrr.matrix44.create_look_at(self.position, self.target, [0, 1, 0])

    def rotate(self, d_pitch, d_yaw):
        self.pitch += d_pitch
        self.yaw += d_yaw
        self.update()

    def zoom(self, delta_distance):
        self.distance -= delta_distance
        self.update()

    def set_target(self, target):
        self.target = target
        self.update()