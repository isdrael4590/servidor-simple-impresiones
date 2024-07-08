# Servidor simple de impresiones

## Dependencias de software

- [Conan](https://conan.io/)
- [Python](https://www.python.org/) >= 3.10
- [CPPRest](https://microsoft.github.io/cpprestsdk/index.html)
- [CMake](https://cmake.org/) >= 3.8

### Desarrollo

- [Visual Studio C++ Community 2022](https://visualstudio.microsoft.com/vs/community/)

## Instalación

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
    - Permitir la conexión y añadir un nombre adeciadp
