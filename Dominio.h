#ifndef DOMINIO_H
#define DOMINIO_H

#include <string>
#include <vector>
#include <utility>

class Dominio {
private:
    std::string expresion;
    double deltaX;

    // Métodos privados
    double f(double x);
    double derivada(double x, double h = 1e-5);
    std::vector<std::pair<double, double>> detectarIntervalosContinuos();

public:
    // Constructor
    Dominio(std::string exp, double dx);

    // Métodos públicos
    void calcularDominio();
    void guardarEnJson(const std::string& filename);

    void guardarRectangulosJson(const std::string &filename, double limInferior, double limSuperior, double deltaX);
};

#endif // DOMINIO_H


