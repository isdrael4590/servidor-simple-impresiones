# Servidor simple de impresiones

## Dependencias de software

- [Conan](https://conan.io/)
- [Python](https://www.python.org/) >= 3.10
- [CPPRest](https://microsoft.github.io/cpprestsdk/index.html)
- [CMake](https://cmake.org/) >= 3.8

## Hardware

- Impresora Zebra [ZD421](https://www.zebra.com/la/es/support-downloads/printers/desktop/zd421.html)

### Desarrollo

- [Visual Studio C++ Community 2022](https://visualstudio.microsoft.com/vs/community/)

## Instrucciones

- Descargue el [driver](https://www.zebra.com/la/es/support-downloads/printers/desktop/zd421.html) de la impresora Zebra a su disposición.
- Instalelo en su computador y seleccione que la conexión sea por USB
- Asegúrese que su impresora se encuentre encendidad y conectada.

### Configuración de la impresora de etiquetas

- Encienda la impresora y conéctela vía USB, Ethernet o Wifi a la red.
- Descargue la aplicación [Zebra Setup Utilities](https://www.zebra.com/us/en/support-downloads/software/printer-software/printer-setup-utilities.html) para Windows, para instalar los drivers
- Seleccione la impresora, con el modelo que tiene a mano. Por ejemplo, Zebra ZD421-203 dpi
- Termine la instalación
- Verificar en Windows, que la impresora esté disponible en la utilidad de impresoras y escáneres, Inicio > Configuración > Dispositivos > Impresoras y escáneres.

### Configuraciones para el Programa Zebra Setup Utilities

_Nota: Estas configuraciones son las recomendadas, para este proyecto en modo impresión de etiquetas_

#### Configuración de la página

- Seleccionar el ancho y alto de la etiqueta que tiene a mano en pulgadas, por ejemplo 2.28"x1.575" para las teiquetas
- Seleccionar el tipo de Medio como: "Etiquetas con espacios"
- Elegir la rotación como: "0º - Formato Vertical"

#### Configuración de impresión

- Velocidad: 5.0"/s
- Oscuridad: 22
- Modo de impresión: Tranferencia termal**
_Modo de operación*
- Modo: "Arranque"

## Para programar

### Primera vez

- `servidor-simple-impresiones`
- Cree un nuevo ambiente de desarrollo, en el CMD con `python -m venv servidor-impresion-env`
- Active el ambiente creado con `.\servidor-impresion-env\Scripts\activate.bat`
- Instalar las librerías necesarias con `pip install -r requirements.txt`
- `conan profile detect --force`
- `conan install . --output-folder=build --build=missing`
- `conan install . --output-folder=build --build=missing -s build_type=Debug`
- `cmake --preset x64-release-ms`
- `cmake --build --preset x64-release-ms --target install`

## Usabilidad en una estación de trabajo Windows

- Abrir el programa como administrador
- Crear una nueva regla en el Firewall de Windows
  - Seleccionar la opción `puerto`
    - Añadir el puerto `3000`
    - Permitir la conexión y añadir un nombre adecuado

## Preguntas frequentes

P. ¿Por qué me sale el Error de la impresora, `Error - No accesible`?
R. Esto es debido a que la impresora trata de refrescar su estado después de cada impresión, se puede resolver como se indica aquí [link](https://supportcommunity.zebra.com/s/article/000020654?language=en_US)
