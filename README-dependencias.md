# Dependencias para compilar y ejecutar QWBFS Manager (Qt6)

Este documento describe las dependencias necesarias en Linux (Ubuntu/Debian) para compilar y ejecutar `qwbfsmanager` con Qt6.

## 1) Paquetes requeridos

Instala los paquetes base de compilación, Qt6 y librerías del sistema:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  qmake6 \
  qt6-base-dev \
  qt6-base-dev-tools \
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

Si este comando devuelve versiones (por ejemplo `1.x.x`), las librerías estan correctamente instaladas.

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
