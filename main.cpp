#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <iomanip>
#include <fcntl.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define SLEEP_FUNC Sleep
#else
    #include <unistd.h>
    #define SLEEP_FUNC sleep
    #include <sys/types.h>
    #include <sys/wait.h>

    // Definir PATH_MAX si no está definido
    #ifndef PATH_MAX
        #define PATH_MAX 4096
    #endif
#endif

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
    cout << "\n***************************************************************************" << endl;
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

    cout << "\nPresiona enter... " << endl;

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

        SLEEP_FUNC(2);  // Dormir 2 segundos

        // --- Ejecución del script de Python ---
        // Usar ruta relativa
        #ifdef _WIN32
        std::wstring commandLine =
            L"\"python\" "
            L"\"./Graficadora.py\"";  // Ruta relativa al script Python en la raíz del proyecto
        #else
        std::wstring commandLine =
            L"\"python3\" "
            L"\"./Graficadora.py\"";  // Ruta relativa al script Python en la raíz del proyecto
        #endif

        wcout << L"Ejecutando comando: " << commandLine << endl;

        // Copiamos la cadena a un buffer modificable
        std::vector<wchar_t> cmdBuffer(commandLine.begin(), commandLine.end());
        cmdBuffer.push_back(0);  // Asegura la terminación nula

        // En Windows obtenemos el directorio de trabajo
        #ifdef _WIN32
            wchar_t currentDir[MAX_PATH];
            if (!GetCurrentDirectoryW(MAX_PATH, currentDir)) {
                wcerr << L"Error al obtener el directorio actual." << endl;
            }
        #else
            char currentDir[PATH_MAX];
            if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
                cerr << "Error al obtener el directorio actual." << endl;
            }
        #endif

        // Verificar la existencia de archivos en Windows
        #ifdef _WIN32
            if (GetFileAttributesW(L"./python") == INVALID_FILE_ATTRIBUTES)
                wcerr << L"Error: No se encontró python.exe en la ruta especificada." << endl;
            if (GetFileAttributesW(L"./Graficadora.py") == INVALID_FILE_ATTRIBUTES)
                wcerr << L"Error: No se encontró Graficadora.py en la ruta especificada." << endl;
        #endif

        // Convertir wstring a string
        std::string commandStr(commandLine.begin(), commandLine.end());

        // Ejecución en Unix/Linux
        #ifdef _WIN32
            STARTUPINFO si = {0};
            PROCESS_INFORMATION pi = {0};

            if (!CreateProcessW(
             nullptr,           // No se especifica un módulo por separado
             commandLine.data(),  // Buffer de comando
             nullptr,           // Atributos de proceso por defecto
             nullptr,           // Atributos de hilo por defecto
             FALSE,             // No heredar handles
             0,                 // Flags de creación
             nullptr,           // Usar el entorno actual
             nullptr,           // Directorio de trabajo: se usa el directorio actual
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
        #else
            // Convertir comando wstring a char* para system()
            int ret = system(commandStr.c_str());
            if (ret != 0) {
                cerr << "Error al ejecutar el comando Python" << endl;
            }
        #endif

        xi = 0;
        sumatoria = 0;

        cout << "\n¿Introducir otra expresion matematica? (Y/N):";
        cin >> answer;
        cout << "\n";

    } while (tolower(answer) == 'y');

    return 0;
}
