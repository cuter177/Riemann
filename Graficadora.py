import json
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np
from pyrr import Matrix44
from pyrr.matrix44 import create_from_axis_rotation

# Variables globales
# Si tienes puntos para la gráfica de la función, se cargarán desde otro archivo (opcional)
puntos = []

# Lista que almacenará las matrices de transformación (una por rectángulo)
rectangulos_matrices = []

# Variables para zoom y desplazamiento (pan)
zoom = 1.0
pan_x, pan_y = 0.0, 0.0

# Variables para el manejo del mouse (para pan)
mouse_pressed = False
mouse_x, mouse_y = 0, 0


def cargar_rectangulos_json():
    global rectangulos_matrices
    try:
        with open("Rectangulo.json", "r") as f:
            data = json.load(f)
            for rect in data["rectangulos"]:
                traslacion = Matrix44.from_translation(rect["transformacion"]["traslacion"])
                escala = Matrix44.from_scale(rect["transformacion"]["escala"])
                rotacion = create_from_axis_rotation(
                    np.array(rect["transformacion"]["rotacion"][1:4], dtype=np.float32),
                    rect["transformacion"]["rotacion"][0]
                )
                # La matriz final: traslación * rotación * escala
                model = traslacion * rotacion * escala
                rectangulos_matrices.append(model)
        print("Cantidad de rectángulos cargados:", len(rectangulos_matrices))
    except Exception as e:
        print(f"Error al cargar 'rectangulos.json': {e}")



def cargar_datos_funcion():
    """
    (Opcional) Carga puntos para dibujar la función desde 'Datos.json'.
    Se espera que tenga la estructura:

    {
       "puntos": [
           {"x": valor, "y": valor},
           ...
       ]
    }
    """
    global puntos
    try:
        with open("Datos.json", "r") as file:
            datos = json.load(file)
            puntos = [(p["x"], p["y"]) for p in datos["puntos"]]
    except Exception as e:
        print(f"Error al cargar 'Datos.json': {e}")


def init_gl():
    """ Configuración inicial de OpenGL. """
    glClearColor(0, 0, 0, 1)  # Fondo negro

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    # Configura una proyección ortográfica. Ajusta estos valores según tus necesidades.
    gluOrtho2D(-50, 50, -50, 50)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()


def draw_axes():
    """ Dibuja los ejes X e Y con números adaptativos que incluyen decimales si el zoom es alto """
    global zoom

    glColor3f(1, 1, 1)
    glBegin(GL_LINES)
    # Eje X
    glVertex2f(-100, 0)
    glVertex2f(100, 0)
    # Eje Y
    glVertex2f(0, -100)
    glVertex2f(0, 100)
    glEnd()

    # Determinar la separación de los números en los ejes basada en el zoom
    base_spacing = 10  # Separación base de los números
    if zoom > 5:
        base_spacing = 1  # Muestra enteros más cercanos
    if zoom > 20:
        base_spacing = 0.5  # Muestra decimales más cercanos
    if zoom > 50:
        base_spacing = 0.1  # Muestra decimales más pequeños

    # Redondear la separación a un número útil
    spacing = max(base_spacing, 10 / zoom)  

    # Dibujar números en el eje X
    x_start = int(-100 / spacing) * spacing
    x_end = int(100 / spacing) * spacing
    x_range = np.arange(x_start, x_end + spacing, spacing)
    
    for i in x_range:
        if abs(i) < 1e-3:  # Evita mostrar -0.0
            i = 0
        glRasterPos2f(i, -3 / zoom)  # Ajustar la posición del texto según el zoom
        text = f"{i:.2f}" if zoom > 20 else f"{int(i)}"  # Mostrar decimales si el zoom es alto
        for char in text:
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ord(char))

    # Dibujar números en el eje Y
    y_start = int(-100 / spacing) * spacing
    y_end = int(100 / spacing) * spacing
    y_range = np.arange(y_start, y_end + spacing, spacing)
    
    for i in y_range:
        if abs(i) < 1e-3:
            i = 0
        glRasterPos2f(-5 / zoom, i)  # Ajustar la posición del texto según el zoom
        text = f"{i:.2f}" if zoom > 20 else f"{int(i)}"
        for char in text:
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ord(char))



def draw_function():
    """ Dibuja la función en verde (si se cargaron puntos). """
    if not puntos:
        return
    glColor3f(0, 1, 0)
    glBegin(GL_LINE_STRIP)
    for x, y in puntos:
        glVertex2f(x, y)
    glEnd()


def draw_rectangles():
    """
    Para cada matriz de transformación (calculada a partir del JSON),
    se aplica la transformación y se dibuja un cuadrilátero unitario.

    Se asume que el cuadrilátero base está definido en el rango:
      (-0.5, -0.5) a (0.5, 0.5)
    De esta forma, al aplicar la escala y traslación definidas,
    el rectángulo se ubicará correctamente (por ejemplo, si en C++ se calculó
    la traslación como [x + deltaX/2, altura/2, 0]).
    """
    glColor3f(1, 0, 0)  # Color rojo para los rectángulos
    for model in rectangulos_matrices:
        glPushMatrix()
        # OpenGL espera la matriz en formato columna mayor.
        # Convertimos la matriz de Pyrr a un arreglo de 16 floats y la transponemos.
        glMultMatrixf(model.astype('float32').T)

        glBegin(GL_QUADS)
        glVertex2f(-0.5, -0.5)
        glVertex2f(0.5, -0.5)
        glVertex2f(0.5, 0.5)
        glVertex2f(-0.5, 0.5)
        glEnd()

        glPopMatrix()


def display():
    """ Función de renderizado. """
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()

    # Aplicar zoom y desplazamiento (pan)
    glTranslatef(pan_x, pan_y, 0)
    glScalef(zoom, zoom, 1)

    draw_axes()
    draw_function()
    draw_rectangles()

    glutSwapBuffers()


def keyboard(key, x, y):
    """ Maneja el zoom con las teclas '+' y '-' """
    global zoom
    if key == b'+':
        zoom *= 1.1
    elif key == b'-':
        zoom /= 1.1
    glutPostRedisplay()


def mouse_click(button, state, x, y):
    """ Registra el estado del mouse para permitir el desplazamiento (pan). """
    global mouse_pressed, mouse_x, mouse_y
    if button == GLUT_LEFT_BUTTON:
        if state == GLUT_DOWN:
            mouse_pressed = True
            mouse_x, mouse_y = x, y
        elif state == GLUT_UP:
            mouse_pressed = False


def mouse_motion(x, y):
    """ Actualiza el desplazamiento (pan) al mover el mouse con el botón presionado. """
    global pan_x, pan_y, mouse_x, mouse_y
    if mouse_pressed:
        dx = (x - mouse_x) / 100.0
        dy = (mouse_y - y) / 100.0  # Se invierte 'y' para que el movimiento sea natural
        pan_x += dx
        pan_y += dy
        mouse_x, mouse_y = x, y
        glutPostRedisplay()


def mouse_wheel(button, state, x, y):
    """ Maneja el zoom con la rueda del mouse (si GLUT lo soporta). """
    global zoom
    if state == 1:  # Scroll up
        zoom *= 1.1
    elif state == -1:  # Scroll down
        zoom /= 1.1
    glutPostRedisplay()


def main():
    # Cargar datos: rectángulos (y función, si tienes)
    cargar_rectangulos_json()
    cargar_datos_funcion()  # Opcional, si cuentas con 'Datos.json'

    # Inicializar GLUT y crear la ventana
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
    glutInitWindowSize(800, 600)
    glutCreateWindow("Rectángulos Riemann con Pyrr y OpenGL".encode("utf-8"))


    init_gl()

    # Registrar callbacks de GLUT
    glutDisplayFunc(display)
    glutKeyboardFunc(keyboard)
    glutMouseFunc(mouse_click)
    glutMotionFunc(mouse_motion)
    # Algunas implementaciones de GLUT soportan mouse wheel:
    glutMouseWheelFunc(mouse_wheel)

    glutMainLoop()


if __name__ == "__main__":
    main()

