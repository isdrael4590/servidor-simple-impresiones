# Cliente impresiones
# Instalar OpenSSL para windows
- Conan
- Python 3.10
- `cd cliente-impresión` 
- En el CMD`python -m venv cliente-impresion`
- `.\cliente-impresion\Scripts\activate.bat`
- `pip install -r requirements.txt`
- `conan profile detect --force`
- `conan install . --output-folder=build --build=missing -s build_type=Debug -s build_type=Release`
- `cmake --preset conan-default`

Select conan release in visual Studio