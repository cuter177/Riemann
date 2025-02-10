#include<iostream>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <iomanip>


#include"node.h"
#include"ExpressionParser.h"
#include"toPostFix.h"
#include"Utils.h"
#include"cutIntegral.h"
#include "Dominio.h"

using namespace std;

int main()
{

       cout<<"\n"<<"***************************************************************************"<<endl;
       cout<<"\n"<<"Bienvenido al programa para aproximar integrales por sumatoria de Reiemann"<<endl;
       cout<<"\n"<<"***************************************************************************"<<endl;
       cout<<"\n"<<"Para insertar una integral se debe de hacer de la siguiente fomra:"<<endl;
       cout<<"integral(ln(abs(x+1)),1,e,0.0001)"<<endl;
       cout<<"Siendo lo primero la funcion que deseas integrar, y separado por comas, <limite inferior>, <limite superior>, <valor de delta de x> "<<endl;
       cout<<"Funciones que acepta el programa:"<<endl;
       cout<<"sin(x)  cos(x), tan(x), sinh(x), cosh(x), tanh(x), arcoseneno (x) = asin(x), arococoseno(x) = acos(x), arcotangente(x) = atan(x) "<<endl;
       cout<<"ln(x), exponencial = E(x)= e^x, valor absoluto = abs(x), raiz cuadrada = r(x), raiz cubica = c(x)"<<endl;
       cout<<"Se puede poner en los limite las costantes pi y e. Se puede operar con ella: e^2, e^5, pi/2, pi*2, etc."<<endl;
       cout<<"para poner un producto se tiene que ocupar el <*>, ejemplo: 2*x^3"<<endl;





        cortarIntegral integral;
    char answer{};

    std::cout<<"\n" << "Presiona enter :" << std::endl;
    do
    {


        string entrada;


        cin.ignore();
        cout << "Introduce tu integral: "<<endl;

        getline(cin, entrada);

        integral.cortar(entrada);

        cout<<""<<endl;

        integral.mostrarDatos();

        string expresion = integral.getArg();
        string& r_expresion = expresion;

        double xi;
        double sumatoria;

        double limInferior = stod(integral.getLimI());
        double limSuperior = stod(integral.getLimS());
        double deltaDeX = stod(integral.getDeltaX());

        for (double i=limInferior; i<=limSuperior; i+=deltaDeX)
        {
            string limite = to_string(i);
            string& r_limite = limite;

            toPostFix x(getMathExpression(r_expresion,r_limite));
            Expression_Parser myTree(x.getPostFixExpression());
            std::shared_ptr<node> tree = myTree.toTree();

         xi = myTree.evaluateExpressionTree(tree) * deltaDeX;

            //cout<<"xi: "<<xi<<endl;

            sumatoria += xi;
        }

        cout <<"El valor de la sumatoria es: "<< std::setprecision(15)<< sumatoria <<endl;

        Dominio dominio(r_expresion, 0.001);
        dominio.calcularDominio();
        dominio.guardarEnJson("Datos.json");
        dominio.guardarRectangulosJson("Rectangulo.json", limInferior, limSuperior, deltaDeX);

        xi = 0;
        sumatoria = 0;

        cout << "\n"<<"¿Introducir otra expresion matematica? (Y/N):";cin >> answer;cout<<"\n";


        //system("clear");
    }
    while (tolower(answer) == 'y');
    return 0;
}
