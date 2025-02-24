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
#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#else
  #include <unistd.h>
  #include <fcntl.h>
#endif

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

// Constructor
Dominio::Dominio(std::string exp, double dx) : expresion(exp), deltaX(dx) {}

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

// Detecta los intervalos continuos de la función en un rango dado
std::vector<std::pair<double, double>> Dominio::detectarIntervalosContinuos(double start, double end) {
    std::vector<std::pair<double, double>> intervalos;
    double inicio = start;
    bool enIntervalo = false;

    // Precompute function values and derivatives
    std::vector<double> fx_values;
    std::vector<double> df_values;
    for (double x = start; x <= end; x += deltaX) {
        fx_values.push_back(f(x));
        df_values.push_back(derivada(x));
    }

    for (size_t i = 0; i < fx_values.size(); ++i) {
        double x = start + i * deltaX;
        double fx = fx_values[i];
        double df = df_values[i];

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
        intervalos.push_back({inicio, end});
    }

    return intervalos;
}

// Define the calcularDominio method
void Dominio::calcularDominio() {
    // Implementation of calcularDominio
    // This method should contain the logic to calculate the domain of the function
}

// Guarda los puntos en un archivo JSON en tiempo real
void Dominio::guardarEnJsonTiempoReal(const std::string& filename, double start, double end, double zoom, double panx, double pany) {
    fs::path rutaRelativa = fs::path("datos") / filename;
    if (!fs::exists(rutaRelativa.parent_path()))
        fs::create_directories(rutaRelativa.parent_path());

    std::ofstream archivo(rutaRelativa);
    if (!archivo) {
        std::cerr << "Error opening file: " << rutaRelativa.string() << std::endl;
        return;
    }

    std::vector<std::pair<double, double>> intervalos = detectarIntervalosContinuos(start, end);
    archivo << "{\n  \"puntos\": [\n";
    bool primerPunto = true;
    for (const auto& intervalo : intervalos) {
        for (double x = intervalo.first; x <= intervalo.second; x += deltaX) {
            double fx = f(x);
            if (!std::isnan(fx) && !std::isinf(fx)) {
                if (!primerPunto) archivo << ",\n";
                archivo << "    {\"x\": " << x << ", \"y\": " << fx << "}";
                primerPunto = false;
            }
        }
    }
    archivo << "\n  ]\n}\n";
    archivo.flush();
    archivo.close();
    std::cout << "Archivo JSON guardado en: " << rutaRelativa.string() << std::endl;
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

        double xIzq = x;
        double xDer = x + deltaX;
        double yInf = 0.0;
        double ySup = altura;

        json rectangulo;
        rectangulo["vertices"] = json::array({
            { xIzq, yInf, 0.0 },
            { xDer, yInf, 0.0 },
            { xDer, ySup, 0.0 },
            { xIzq, ySup, 0.0 }
        });

        jsonData["rectangulos"].push_back(rectangulo);
    }

    fs::path rutaRelativa = fs::path("datos") / filename;

    std::ofstream archivo(rutaRelativa);
    if (archivo.is_open()) {
        archivo << jsonData.dump(4);
        archivo.close();
        std::cout << "JSON generado en: " << rutaRelativa.string() << std::endl;
    } else {
        std::cerr << "Error al abrir el archivo: " << rutaRelativa << std::endl;
    }
}
