# YT-DLP GUI Modern

Una interfaz gráfica moderna para **yt-dlp**, escrita en **C++17** con **GTK4 (gtkmm 4)**.  
Permite descargar **video, audio o ambos**, elegir calidad y formato, y gestionar múltiples URLs de manera sencilla.

---

## Características

- Descargar **video + audio**, solo video o solo audio.
- Elegir **calidad de video**: 240p, 360p, 480p, 720p, 1080p, 1440p, 2160p.
- Elegir **formato de video**: mp4, webm, mkv, avi, mov, flv o formato original.
- Elegir **calidad de audio**: Peor, 128k, 192k, 256k, 320k o Mejor.
- Elegir **formato de audio**: mp3, aac, flac, m4a, opus, vorbis o formato original.
- Guardado automático de la **última carpeta de descargas**.  
- Barra de progreso por descarga y progreso general.
- Notificaciones cuando las descargas finalizan.
- Soporte para múltiples URLs al mismo tiempo.

---

## Requisitos

- **Linux**  
- **C++17 compatible compiler** (GCC >= 9 recomendado)  
- **Meson** y **Ninja** u otro compilador de preferencia(no soportado oficialmente)
- **gtkmm-4.0**  
- **yt-dlp** (binario colocado en `external/yt-dlp` para la instalación automática)

---

## Instalación

Clona el repositorio:

```bash
git clone https://github.com/tuusuario/yt-dlp-gui-gtk.git
cd yt-dlp-gui-gtk
```

## Compilar con Meson
```bash
meson setup build
meson compile -C build
sudo meson install -C build
```
---
Esto instalará **yt-dlp-gui-gtk** en /usr/local/bin (o tu prefijo definido)

---
## Estructura del proyecto
.
├── build/                  # Directorio de compilación (Meson)
├── external/yt-dlp         # Binario de yt-dlp
├── main.cpp                # Código principal
├── meson.build             # Build configuration
└── README.md

---
## Licencia
MIT License.

---
## Notas:
 - La aplicación es solo para linux.
 - Asegurate de tener **yt-dlp actualizado** para evitar errores de descarga
 - La barra de progreso puede tardar en reflejarse según la salida de yt-dlp.

---
# Espero que esta herramienta te sea de ayuda
