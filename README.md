# Cliente impresiones

## Instalar OpenSSL para windows

- Conan
- Python 3.10
- `cd cliente-impresi�n`
- En el CMD`python -m venv servidor-impresion-env`
- `.\servidor-impresion-env\Scripts\activate.bat`
- `pip install -r requirements.txt`
- `conan profile detect --force`
- `conan install . --output-folder=build --build=missing`
- `conan install . --output-folder=build --build=missing -s build_type=Debug`
- `cmake --preset x64-release-ms`
- `cmake --build --preset x64-release-ms --target install`

Select conan release in visual Studio

Abrir puerto 3000
Usar como administrador
