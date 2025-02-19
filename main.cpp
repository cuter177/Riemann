#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <iomanip>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

// Incluye tus otros headers personalizados
#include "node.h"
#include "ExpressionParser.h"
#include "toPostFix.h"
#include "Utils.h"
#include "cutIntegral.h"
#include "Dominio.h"

using namespace std;

int main()
{
    //cout << "\n***************************************************************************" << endl;
    cout << "\nBienvenido al programa para aproximar integrales por sumatoria de Reiemann" << endl;
    cout << "\n***************************************************************************" << endl;
    cout << "\nPara insertar una integral se debe de hacer de la siguiente forma:" << endl;
    cout << "integral(ln(abs(x+1)),1,e,0.0001)" << endl;
    cout << "Siendo lo primero la función que deseas integrar, y separado por comas, <limite inferior>, <limite superior>, <valor de delta de x>" << endl;
    cout << "Funciones que acepta el programa:" << endl;
    cout << "sin(x), cos(x), tan(x), sinh(x), cosh(x), tanh(x), arcoseneno(x) = asin(x), arococoseno(x) = acos(x), arcotangente(x) = atan(x)" << endl;
    cout << "ln(x), exponencial = E(x)= e^x, valor absoluto = abs(x), raiz cuadrada = r(x), raiz cubica = c(x)" << endl;
    cout << "Se puede poner en los límites las constantes pi y e. Se puede operar con ellas: e^2, e^5, pi/2, pi*2, etc." << endl;
    cout << "Para poner un producto se tiene que ocupar el <*>, ejemplo: 2*x^3" << endl;

    cortarIntegral integral;
    char answer{};

    cout << "\nPresiona enter :" << endl;

    // Bucle principal
    do
    {
        string entrada;
        cin.ignore();
        cout << "Introduce tu integral: " << endl;
        getline(cin, entrada);

        integral.cortar(entrada);
        cout << "" << endl;
        integral.mostrarDatos();

        string expresion = integral.getArg();
        string &r_expresion = expresion;

        double xi = 0;
        double sumatoria = 0;

        double limInferior = stod(integral.getLimI());
        double limSuperior = stod(integral.getLimS());
        double deltaDeX = stod(integral.getDeltaX());

        for (double i = limInferior; i <= limSuperior; i += deltaDeX)
        {
            string limite = to_string(i);
            string &r_limite = limite;

            toPostFix x(getMathExpression(r_expresion, r_limite));
            Expression_Parser myTree(x.getPostFixExpression());
            std::shared_ptr<node> tree = myTree.toTree();

            xi = myTree.evaluateExpressionTree(tree) * deltaDeX;
            sumatoria += xi;
        }

        cout << "El valor de la sumatoria es: " << setprecision(15) << sumatoria << endl;

        // Genera los archivos JSON con los datos
        Dominio dominio(r_expresion, 0.001);
        dominio.calcularDominio();
        dominio.guardarEnJson("Datos.json");
        dominio.guardarRectangulosJson("Rectangulo.json", limInferior, limSuperior, deltaDeX);

        Sleep(2000);

        // --- Ejecución del script de Python ---
        // Actualizamos la ruta de python.exe a Python313
        std::wstring commandLine =
            L"\"C:\\Users\\Pop90\\AppData\\Local\\Programs\\Python\\Python313\\python.exe\" "
            L"\"C:\\Users\\Pop90\\Documents\\Riemann_4.1\\Graficadora.py\"";

        wcout << L"Ejecutando comando: " << commandLine << endl;

        // Copiamos la cadena a un buffer modificable
        std::vector<wchar_t> cmdBuffer(commandLine.begin(), commandLine.end());
        cmdBuffer.push_back(0);  // Asegura la terminación nula

        wchar_t currentDir[MAX_PATH];
        if (!GetCurrentDirectoryW(MAX_PATH, currentDir)) {
            wcerr << L"Error al obtener el directorio actual." << endl;
            // Puedes asignar un valor por defecto o manejar el error
        }

        // (Opcional) Verificar que ambos archivos existan:
        if (GetFileAttributesW(L"C:\\Users\\Pop90\\AppData\\Local\\Programs\\Python\\Python313\\python.exe") == INVALID_FILE_ATTRIBUTES)
            wcerr << L"Error: No se encontró python.exe en la ruta especificada." << endl;
        if (GetFileAttributesW(L"C:\\Users\\Pop90\\OneDrive - Benem\u00E9rita Universidad Aut\u00F3noma de Puebla\\Universidad\\Pimer Semestre\\M\u00E9todolog\u00EDa de la Progamaci\u00F3n\\Riemann_3.04\\Graficadora.py") == INVALID_FILE_ATTRIBUTES)
            wcerr << L"Error: No se encontró Graficadora.py en la ruta especificada." << endl;

        // Configurar la estructura para la creación del proceso
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessW(
         nullptr,           // No se especifica un módulo por separado
         cmdBuffer.data(),  // Buffer modificable con el comando
         nullptr,           // Atributos de proceso por defecto
         nullptr,           // Atributos de hilo por defecto
         FALSE,             // No heredar handles
         0,                 // Flags de creación
         nullptr,           // Usar el entorno actual
         currentDir,        // Directorio de trabajo: se usa el directorio actual
         &si,
         &pi))
        {
            DWORD err = GetLastError();
            wcerr << L"Error al ejecutar CreateProcessW: " << err << endl;
        }
        else
        {
            // Espera a que el proceso termine y obtiene el código de salida
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            wcout << L"Código de retorno: " << exitCode << endl;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        xi = 0;
        sumatoria = 0;

        cout << "\n¿Introducir otra expresion matematica? (Y/N):";
        cin >> answer;
        cout << "\n";

    } while (tolower(answer) == 'y');

    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.