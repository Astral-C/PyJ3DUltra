import glfw
import itertools
import numpy as np
from OpenGL.GL import *
from camera import Camera
from OpenGL.GL.shaders import compileProgram, compileShader
from pyrr import matrix44, Matrix44, Vector4, Vector3, Quaternion

import J3DUltra as ultra

def window_resize(window, width, height):
    glViewport(0, 0, width, height)

cam = Camera()
keys = [False] * 1024
lastX, lastY = 960, 540
first_mouse = True


def key_callback(window, key, scancode, action, mode):
    if key == glfw.KEY_ESCAPE and action == glfw.PRESS:
        glfw.set_window_should_close(window, True)

    if key >= 0 and key < 1024:
        if action == glfw.PRESS:
            keys[key] = True
        elif action == glfw.RELEASE:
            keys[key] = False


def do_movement():
    if keys[glfw.KEY_W]:
        cam.process_keyboard("FORWARD", 1.0)
    if keys[glfw.KEY_S]:
        cam.process_keyboard("BACKWARD", 1.0)
    if keys[glfw.KEY_A]:
        cam.process_keyboard("LEFT", 1.0)
    if keys[glfw.KEY_D]:
        cam.process_keyboard("RIGHT", 1.0)


def mouse_callback(window, xpos, ypos):
    global first_mouse, lastX, lastY

    if first_mouse:
        lastX = xpos
        lastY = ypos
        first_mouse = False

    xoffset = xpos - lastX
    yoffset = lastY - ypos

    lastX = xpos
    lastY = ypos

    cam.process_mouse_movement(xoffset, yoffset)

# main

if(not glfw.init()):
    raise Exception("Couldn't init GLFW")


glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 4)
glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 6)
glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
glfw.window_hint(glfw.OPENGL_DEBUG_CONTEXT, glfw.TRUE)
glfw.window_hint(glfw.DEPTH_BITS, 24)

window = glfw.create_window(1280, 720, "J3DUltra Test", None, None)

if(not window):
    glfw.terminate()
    raise Exception("Couldn't setup window")


glfw.make_context_current(window)
glfw.set_window_size_callback(window, window_resize)
glfw.set_key_callback(window, key_callback)
glfw.set_cursor_pos_callback(window, mouse_callback)

if(not ultra.init()):
    glfw.terminate()
    raise Exception("Couldn't init J3DUltra")

light1 = ultra.makeLight(
    [0, 0, 0],
    [0, 0, 0],
    [1, 1, 1, 1], 
    [1, 0, 0],
    [1, 0, 0]
)

light2 = ultra.makeLight(
    [1, 0, 0],
    [0, 0, 0],
    [1, 1, 1, 1], 
    [1, 0, 0],
    [1, 0, 0]
)

light3 = ultra.makeLight(
    [0, 0, 0],
    [1, -0.868448, 0.239316],
    [1, 1, 1, 1], 
    [25, 0, -24],
    [0, 0, 1]
)

ultra.setLight(light1, 0)
ultra.setLight(light2, 1)
ultra.setLight(light3, 2)


#test = J3DUltra.J3DModelData()
test = ultra.loadModel("penguin.bdl")
model2 = ultra.loadModel("us_denchi_1.bmd")

model2.setTranslation(200.0, 0.0, 0.0)
model2.setRotation(90.0, 150.0, 0.0)
model2.setScale(2.0, 2.0, 2.0)

glClearColor(0.25, 0.3, 0.4, 1.0)

proj = matrix44.create_perspective_projection_matrix(45.0, 1280/720, 0.1, 100000.0)

while(not glfw.window_should_close(window)):
    glfw.poll_events()
    do_movement()

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    view = cam.get_view_matrix()

    ultra.setCamera(
        proj.ravel().tolist(),
        view.ravel().tolist()
    )

    if(test != None):
        test.render(0.0)

    if(model2 != None):
        model2.render(0.0)

    glfw.swap_buffers(window)

ultra.cleanup()
glfw.terminate()