import glfw
import itertools
import numpy as np
from OpenGL.GL import *
from camera import OrbitCamera as Camera
from OpenGL.GL.shaders import compileProgram, compileShader
from pyrr import matrix44, Matrix44, Vector4, Vector3, Quaternion
from imgui.integrations.glfw import GlfwRenderer
import imgui

from tkinter import Tk
from tkinter.filedialog import askopenfilename

import J3DUltra as ultra
from J3DUltra import J3DLight

import pyjkernel

# Hide root window
Tk().withdraw()

cam = Camera(distance=1000.0, pitch=30.0, yaw=45.0)

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

if(not ultra.init()):
    glfw.terminate()
    raise Exception("Couldn't init J3DUltra")

light1 = J3DLight([0, 0, 0], [0, 0, 0], [1, 1, 1, 1], [1, 1, 1], [1, 1, 1])
light3 = J3DLight([0, 0, 0], [1, -0.868448, 0.239316], [1, 1, 1, 1], [1, 1, 1], [1, 1, 1])

ultra.setLight(light1, 0)
ultra.setLight(light3, 1)
ultra.setLight(light1, 2)
ultra.setLight(light1, 3)
ultra.setLight(light1, 4)
ultra.setLight(light1, 5)
ultra.setLight(light1, 6)
ultra.setLight(light1, 7)

model = None

imgui.create_context()
imgui.get_io().fonts.get_tex_data_as_rgba32()
impl = GlfwRenderer(window)

while(not glfw.window_should_close(window)):
    glfw.poll_events()
    impl.process_inputs()

    if glfw.get_key(window, glfw.KEY_W) == glfw.PRESS:
        cam.rotate(1.0, 0.0)
    if glfw.get_key(window, glfw.KEY_S) == glfw.PRESS:
        cam.rotate(-1.0, 0.0)
    if glfw.get_key(window, glfw.KEY_A) == glfw.PRESS:
        cam.rotate(0.0, 1.0)
    if glfw.get_key(window, glfw.KEY_D) == glfw.PRESS:
        cam.rotate(0.0, -1.0)
    
    if glfw.get_key(window, glfw.KEY_Q) == glfw.PRESS:
        if glfw.get_key(window, glfw.KEY_LEFT_SHIFT) == glfw.PRESS:
            cam.target[1] += 1.0
            cam.update()
        else:
            cam.zoom(5)
    if glfw.get_key(window, glfw.KEY_E) == glfw.PRESS:
        if glfw.get_key(window, glfw.KEY_LEFT_SHIFT) == glfw.PRESS:
            cam.target[1] -= 1.0
            cam.update()
        else:
            cam.zoom(-5)


    glClearColor(0.25, 0.3, 0.4, 1.0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    width, height = glfw.get_framebuffer_size(window)
    
    glViewport(0, 0, width, height)
    imgui.get_io().display_size = width, height

    proj = matrix44.create_perspective_projection_matrix(45.0, width/height, 0.1, 100000.0)

    ultra.setCamera(
        proj.ravel().tolist(),
        cam.view_matrix.ravel().tolist()
    )

    if(model != None):
        model.render()

    ultra.render(0.0, [cam.position[0], cam.position[1], cam.position[2]])

    imgui.new_frame()

    if(imgui.begin_main_menu_bar()):
        if(imgui.begin_menu("File", True)):
            clicked_open, selected_open = imgui.menu_item(
                "Open", "Cmd+O", False, True
            )

            if(clicked_open):
                filename = askopenfilename()
                if('.szs' in filename or '.arc' in filename):
                    arc = pyjkernel.from_archive_file(filename)

                    foundModel = False
                    for file in arc.list_files(arc.root_name):
                        if('.bmd' in file.name or '.bdl' in file.name):
                            model = ultra.loadModel(data=file.data)
                            foundModel = True
                            break

                    if(not foundModel):
                        print(f"No model found in archive {filename}")

                elif('.bdl' in filename or '.bmd' in filename):
                    model = ultra.loadModel(path=filename)
                else:
                    print("Couldn't load model!")

            imgui.end_menu()
        imgui.end_main_menu_bar()

    imgui.render()
    impl.render(imgui.get_draw_data())
    imgui.end_frame()

    glfw.swap_buffers(window)

ultra.cleanup()
glfw.terminate()