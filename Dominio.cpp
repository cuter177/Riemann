#include "Dominio.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <memory>
#include "node.h"
#include "ExpressionParser.h"
#include "toPostFix.h"
#include "Utils.h"
#include <nlohmann/json.hpp>


using namespace std;
using json = nlohmann::json;

// Constructor
Dominio::Dominio(string exp, double dx) : expresion(exp), deltaX(dx) {}

// Evalúa la función en un punto x
double Dominio::f(double x) {
    try {
        string limite = to_string(x);
        toPostFix postfix(getMathExpression(expresion, limite));
        Expression_Parser parser(postfix.getPostFixExpression());
        shared_ptr<node> tree = parser.toTree();
        return parser.evaluateExpressionTree(tree);
    } catch (...) {
        return NAN;
    }
}

// Aproximación numérica de la derivada
double Dominio::derivada(double x, double h) {
    return (f(x + h) - f(x)) / h;
}

// Detecta los intervalos continuos de la función en [-100, 100]
vector<pair<double, double>> Dominio::detectarIntervalosContinuos() {
    vector<pair<double, double>> intervalos;
    double inicio = -100;
    bool enIntervalo = false;

    for (double x = -100; x <= 100; x += deltaX) {
        double fx = f(x);
        double df = derivada(x);

        if (isnan(fx) || isinf(fx) || fabs(df) > 1e5) {
            if (enIntervalo) {
                intervalos.push_back({inicio, x - deltaX});
                enIntervalo = false;
            }
        } else {
            if (!enIntervalo) {
                inicio = x;
                enIntervalo = true;
            }
        }
    }

    if (enIntervalo) {
        intervalos.push_back({inicio, 100});
    }

    return intervalos;
}

// Calcula el dominio e imprime los intervalos continuos
void Dominio::calcularDominio() {
    vector<pair<double, double>> intervalos = detectarIntervalosContinuos();
    cout << "Dominio de la función en [-100, 100]:\n";
    for (const auto& intervalo : intervalos) {
        cout << "[" << intervalo.first << ", " << intervalo.second << "]\n";
    }
}

// Guarda los puntos en un archivo JSON


namespace fs = std::filesystem;

void Dominio::guardarEnJson(const std::string& filename) {
    // Ruta base del proyecto
    //std::wstring directorioProyecto = LR"(/home/luis/Riemann)";

    // Crear la ruta completa
    fs::path rutaCompleta = fs::path("datos") / filename; //ruta relativa

    // Crear directorio si no existe
    if (!fs::exists(rutaCompleta.parent_path())) {
        fs::create_directories(rutaCompleta.parent_path());
    }

    // Abrir archivo de salida
    std::ofstream archivo(rutaCompleta);
    if (!archivo) {
        std::cerr << "Error al abrir el archivo en: " << rutaCompleta.string() << std::endl;
        return;
    }

    // Obtener los intervalos continuos
    std::vector<std::pair<double, double>> intervalos = detectarIntervalosContinuos();

    // Escribir JSON
    archivo << "{\n  \"puntos\": [\n";
    bool primerPunto = true;

    for (const auto& intervalo : intervalos) {
        for (double x = intervalo.first; x <= intervalo.second; x += this->deltaX) {
            double fx = f(x);
            if (!std::isnan(fx) && !std::isinf(fx)) {
                if (!primerPunto) archivo << ",\n";
                archivo << "    {\"x\": " << x << ", \"y\": " << fx << "}";
                primerPunto = false;
            }
        }
    }

    archivo << "\n  ]\n}\n";
    archivo.close();

    std::cout << "Archivo JSON guardado en: " << rutaCompleta.string() << std::endl;
}

void Dominio::guardarRectangulosJson(const std::string& filename, double limInferior, double limSuperior, double deltaX) {
    json jsonData;
    jsonData["rectangulos"] = json::array();

    const double epsilon = 1e-9;
    const int totalPasos = static_cast<int>((limSuperior - limInferior) / deltaX + epsilon);

    for (int i = 0; i <= totalPasos; ++i) {
        double x = limInferior + i * deltaX;
        double altura = f(x);

        if (std::isnan(altura) || std::isinf(altura))
            continue;

        // Calcular los vértices para el rectángulo:
        // Suponiendo que la base está en y=0 y la altura es 'altura'
        double xIzq = x;            // coordenada x izquierda
        double xDer = x + deltaX;     // coordenada x derecha
        double yInf = 0.0;            // coordenada y inferior
        double ySup = altura;         // coordenada y superior

        json rectangulo;
        // Se guardan los vértices en el siguiente orden:
        // inferior izquierda, inferior derecha, superior derecha y superior izquierda.
        rectangulo["vertices"] = json::array({
            { xIzq, yInf, 0.0 },
            { xDer, yInf, 0.0 },
            { xDer, ySup, 0.0 },
            { xIzq, ySup, 0.0 }
        });

        jsonData["rectangulos"].push_back(rectangulo);
    }

    // Guardar en archivo (usando ruta relativa al ejecutable)
    //std::wstring directorioProyecto = LR"(/home/luis/Riemann)";
    //fs::path rutaCompleta = fs::path(directorioProyecto) / filename; // Combina el directorio y el nombre del archivo
    fs::path rutaCompleta = fs::path("datos") / filename; //ruta relativa

    std::ofstream archivo(rutaCompleta);
    if (archivo.is_open()) {
        archivo << jsonData.dump(4);
        archivo.close();
        std::cout << "JSON generado en: " << rutaCompleta.string() << std::endl;
    } else {
        std::cerr << "Error al abrir el archivo: " << rutaCompleta << std::endl;
    }
}





