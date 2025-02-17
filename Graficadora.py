import json
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np

# Lista de puntos para graficar la función (si se tiene)
puntos = []

# Lista que almacenará los vértices de cada rectángulo
rectangulos = []

# Variables para zoom y desplazamiento (pan)
zoom = 19.19434249577509
pan_x, pan_y = 0.0, 0.0

# Variables para el manejo del mouse (para pan)
mouse_pressed = False
mouse_x, mouse_y = 0, 0

def cargar_rectangulos_json():
    """
    Carga el JSON de rectángulos y guarda, para cada uno, la lista de vértices.
    Se espera que cada rectángulo tenga una clave "vertices" con 4 subarreglos.
    """
    global rectangulos
    try:
        with open("datos/Rectangulo.json", "r") as f:
            data = json.load(f)
            for rect in data["rectangulos"]:
                vertices = rect.get("vertices", None)
                if vertices is not None:
                    rectangulos.append(vertices)
        print("Cantidad de rectángulos cargados:", len(rectangulos))
    except Exception as e:
        print(f"Error al cargar 'Rectangulo.json': {e}")

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
        with open("datos/Datos.json", "r") as file:
            datos = json.load(file)
            puntos = [(p["x"], p["y"]) for p in datos["puntos"]]
    except Exception as e:
        print(f"Error al cargar 'Datos.json': {e}")

def init_gl():
    """ Configuración inicial de OpenGL. """
    glClearColor(0.15, 0.15, 0.15, 1)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    # Puedes ajustar los límites según tus necesidades
    gluOrtho2D(-100, 100, -100, 100)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def draw_axes():
    """ Dibuja los ejes X e Y con números adaptativos según el zoom """
    global zoom

    # Guarda todos los atributos para aislar el estado (incluyendo color)
    glPushAttrib(GL_ALL_ATTRIB_BITS)
    glColor3f(1, 1, 1)  # Fuerza el color blanco para los ejes

    glBegin(GL_LINES)
    # Eje X
    glVertex2f(-100, 0)
    glVertex2f(100, 0)
    # Eje Y
    glVertex2f(0, -100)
    glVertex2f(0, 100)
    glEnd()

    # Determinar la separación de los números en función del zoom
    base_spacing = 10
    if zoom > 5:
        base_spacing = 1
    if zoom > 20:
        base_spacing = 0.5
    if zoom > 50:
        base_spacing = 0.1
    spacing = max(base_spacing, 10 / zoom)

    # Dibujar números en el eje X
    x_start = int(-100 / spacing) * spacing
    x_end = int(100 / spacing) * spacing
    x_range = np.arange(x_start, x_end + spacing, spacing)
    for i in x_range:
        if abs(i) < 1e-3:
            i = 0
        glRasterPos2f(i, -3 / zoom)
        text = f"{i:.1f}" if zoom > 20 else f"{int(i)}"
        for char in text:
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, ord(char))

    # Dibujar números en el eje Y
    y_start = int(-100 / spacing) * spacing
    y_end = int(100 / spacing) * spacing
    y_range = np.arange(y_start, y_end + spacing, spacing)
    for i in y_range:
        if abs(i) < 1e-3:
            i = 0
        glRasterPos2f(-5 / zoom, i)
        text = f"{i:.1f}" if zoom > 20 else f"{int(i)}"
        for char in text:
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, ord(char))

    glPopAttrib()  # Restaura todos los atributos


def draw_function():
    """ Dibuja la función en verde, conectando los puntos sin saltos grandes. """
    if not puntos:
        return

    glColor3f(0, 1, 0)
    glBegin(GL_LINE_STRIP)
    threshold = 10  # Diferencia máxima permitida para evitar saltos grandes
    last_x, last_y = puntos[0]
    for x, y in puntos[1:]:
        if abs(x) > 0.05:  # Evita valores cercanos a x = 0
            if abs(y - last_y) > threshold:
                glEnd()
                glBegin(GL_LINE_STRIP)
            glVertex2f(x, y)
            last_x, last_y = x, y
    glEnd()

def draw_rectangles():
    """ Dibuja los rectángulos en rojo utilizando los vértices cargados desde el JSON """
    glColor3f(1, 1, 0)
    for vertices in rectangulos:
        glBegin(GL_QUADS)
        # Se asume que 'vertices' es una lista de 4 sublistas [x, y, z]
        for vertex in vertices:
            glVertex3f(vertex[0], vertex[1], vertex[2])
        glEnd()

def display():
    """ Función de renderizado. """
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()

    #print(f"zoom = {zoom}, pan_x = {pan_x}, pan_y = {pan_y}")

    # Aplicar zoom y desplazamiento (pan)
    glTranslatef(pan_x, pan_y, 0)
    glScalef(zoom, zoom, 1)

    draw_grid()
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
           
def update_motion(value):
    glutPostRedisplay()
    glutTimerFunc(1000 // 60, update_motion, 0)  # Llamada cada 1/60 segundos

def mouse_motion(x, y):
    """ Actualiza el desplazamiento (pan) al mover el mouse con el botón presionado. """
    global pan_x, pan_y, mouse_x, mouse_y
    if mouse_pressed:
        dx = (x - mouse_x) / 4.0
        dy = (mouse_y - y) / 4.0  # Invertir 'y' para que el movimiento sea natural
        if abs(dx) > 0.5 or abs(dy) > 0.5:  # Redibuja solo si el cambio es significativo
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


def draw_grid():
    """ Dibuja una cuadricula en el fondo """
    glPushAttrib(GL_ALL_ATTRIB_BITS)
    # Aseguramos que la iluminación esté desactivada para que el color se aplique correctamente
    glDisable(GL_LIGHTING)
    # Establece un color gris claro para la cuadricula
    glColor3f(0.3, 0.3, 0.3)

    glBegin(GL_LINES)
    # Define el espaciado de la cuadricula (por ejemplo, cada 10 unidades)
    grid_spacing = 1

    # Líneas verticales
    for x in np.arange(-100, 100 + grid_spacing, grid_spacing):
        glVertex2f(x, -100)
        glVertex2f(x, 100)

    # Líneas horizontales
    for y in np.arange(-100, 100 + grid_spacing, grid_spacing):
        glVertex2f(-100, y)
        glVertex2f(100, y)
    glEnd()

    glPopAttrib()


def main():
    # Cargar datos: rectángulos y (opcionalmente) la función
    cargar_rectangulos_json()
    cargar_datos_funcion()  # Opcional, si cuentas con 'Datos.json'

    # Inicializar GLUT y crear la ventana
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
    glutInitWindowSize(800, 600)
    glutCreateWindow("Rectángulos Riemann desde JSON".encode("utf-8"))
    init_gl()

    # Ajustar aquí los valores iniciales de zoom y pan
    zoom = 19.19434249577509
    pan_x = 0.0
    pan_y = 0.0

    # Registrar callbacks de GLUT
    glutDisplayFunc(display)
    glutKeyboardFunc(keyboard)
    glutMouseFunc(mouse_click)
    glutMotionFunc(mouse_motion)
    glutMouseWheelFunc(mouse_wheel)

    glutMainLoop()

if __name__ == "__main__":
    main()

