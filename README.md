# Cliente impresiones
# Instalar OpenSSL para windows
- Conan
- Python 3.10
- `cd cliente-impresi�n` 
- En el CMD`python -m venv servidor-impresion-env`
- `.\servidor-impresion-env\Scripts\activate.bat`
- `pip install -r requirements.txt`
- `conan profile detect --force`
- `conan install . --output-folder=build --build=missing -s build_type=Debug`
- `cmake --preset conan-default`

Select conan release in visual Studio


Descargar el wkthmltopdf https://wkhtmltopdf.org/downloads.html 