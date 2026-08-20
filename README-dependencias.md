# Dependencias para compilar y ejecutar QWBFS Manager (Qt6)

Este documento describe las dependencias necesarias para compilar y ejecutar `qwbfsmanager` con Qt6 en Linux y Windows.

---

# Linux (Ubuntu/Debian)

## 1) Paquetes requeridos

Instala los paquetes base de compilación, Qt6 y librerías del sistema:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  qmake6 \
  qt6-base-dev \
  qt6-base-dev-tools \
  qt6-l10n-tools \
  pkg-config \
  libssl-dev \
  libudev-dev \
  libx11-dev \
  libxext-dev
```

## 2) Verificación rápida de dependencias

Comprueba que X11/Xext estén disponibles para el linker:

```bash
pkg-config --modversion x11 xext
```

Si este comando devuelve versiones (por ejemplo `1.x.x`), las librerías están correctamente instaladas.

## 3) Compilación

Desde la raíz del proyecto:

```bash
qmake6 qwbfs.pro
make -j"$(nproc)"
```

### un solo comando

```bash
qmake6 qwbfs.pro && make -j"$(nproc)" && ./bin/qwbfsmanager
```

El binario generado queda en:

`bin/qwbfsmanager`

## 4) Ejecución

```bash
./bin/qwbfsmanager
```

## 5) Problemas comunes

- `cannot find -lXext`: instala `libxext-dev` y `libx11-dev`.
- `openssl/md5.h: No such file or directory`: instala `libssl-dev`.
- `qmake6: command not found`: instala `qmake6` y `qt6-base-dev`.

---

# Windows (MSYS2 / MinGW64)

Compilación probada con [MSYS2](https://www.msys2.org/) y el entorno **MINGW64**.

## 1) Paquetes requeridos

Abre una shell **MSYS2 MINGW64** e instala:

```bash
pacman -S --needed \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-make \
  mingw-w64-x86_64-pkgconf \
  mingw-w64-x86_64-qt6-base \
  mingw-w64-x86_64-qt6-tools \
  mingw-w64-x86_64-openssl
```

Asegúrate de tener `qmake6` y el compilador en el `PATH` de esa shell:

```bash
export PATH=/mingw64/bin:/usr/bin:$PATH
which qmake6 g++ mingw32-make
```

## 2) Compilación

Desde la raíz del proyecto (ruta MSYS2, por ejemplo `/c/Projects/tools/qwbfsmanager`):

```bash
export PATH=/mingw64/bin:/usr/bin:$PATH
qmake6 qwbfs.pro
mingw32-make -j$(nproc)
```

### Traducciones (opcional pero recomendado)

Los selectores de idioma leen archivos `.qm`. Genéralos y colócalos junto al ejecutable:

```bash
lrelease translations/*.ts
mkdir -p bin/translations
cp -f translations/*.qm bin/translations/
# si existen traducciones de Fresh:
cp -f fresh.git/translations/*.qm bin/translations/ 2>/dev/null || true
```

El binario generado queda en:

`bin/qwbfsmanager.exe`

## 3) Ejecución

### Desde CMD

```cmd
cd C:\Projects\tools\qwbfsmanager
set PATH=C:\msys64\mingw64\bin;%PATH%
bin\qwbfsmanager.exe
```

### Desde PowerShell

```powershell
cd C:\Projects\tools\qwbfsmanager
$env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
.\bin\qwbfsmanager.exe
```

### Desde MSYS2 MINGW64

```bash
export PATH=/mingw64/bin:/usr/bin:$PATH
./bin/qwbfsmanager.exe
```

El `PATH` a `C:\msys64\mingw64\bin` es necesario para que Windows encuentre las DLL de Qt y MinGW.

## 4) Paquete portable (zip de release)

Para generar el mismo artefacto que publica GitHub Actions:

```bash
export PATH=/mingw64/bin:/usr/bin:$PATH
./.github/scripts/build-windows.sh
# salida: dist/qwbfsmanager-<version>-win64.zip
```

El zip incluye `qwbfsmanager.exe`, DLLs de Qt/OpenSSL/MinGW y `translations/`. Al extraerlo se puede ejecutar sin configurar el `PATH`.

## 5) Problemas comunes

- Falta `qmake6` / `g++`: abre la shell **MINGW64** (no la MSYS genérica) e instala los paquetes del apartado 1.
- La app no arranca o falta una DLL (`Qt6Core.dll`, etc.): falta `C:\msys64\mingw64\bin` en el `PATH` (o usa el zip portable del apartado 4).
- Pocos idiomas en el selector: faltan los `.qm`; ejecuta `lrelease` y cópialos a `bin/translations/`.
- `cannot find -lcrypto` / OpenSSL: instala `mingw-w64-x86_64-openssl`.
