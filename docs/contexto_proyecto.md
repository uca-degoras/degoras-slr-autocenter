# Contexto del proyecto: centrado automático por imagen – Estación SLR del ROA

Última actualización: 18/11/2026 (paquete dpot del ROA y respuestas de Ángel Vera del 18/11/2026).

## 1. Dominio
**SLR (Satellite Laser Ranging).** La estación dispara pulsos láser a satélites con retrorreflectores y mide el tiempo de ida y vuelta (d = c·t/2) con precisión milimétrica. Usos: órbitas, geodesia, sistemas de referencia y, cada vez más, basura espacial. La estación San Fernando (SFEL) forma parte de la red ILRS.

**El problema.** La montura sigue al objeto con predicciones orbitales, pero queda un desvío residual respecto al eje del telescopio receptor. Hoy lo corrige el operador a mano. Queremos medir automáticamente, en cada fotograma, dónde está el objeto (y el haz láser, si se ve) respecto al centro de la imagen, y dar el desvío en píxeles y en segundos de arco Az/El.

## 2. Telescopios y cámaras (confirmado por el ROA)
Ambos telescopios van en la misma montura (la "U"); cada cámara está fija a su telescopio.

| | RASA-8 + ASI2600MM Pro | SFEL-RX600 + ASI585MC Pro |
|---|---|---|
| Papel | **Apuntador**: campo amplio. Apuntado inicial del haz láser y centrado inicial del objeto/estrella | **Receptor** (telescopio principal, espejo de 600 mm): centrado fino |
| Cámara | Monocroma, 6248 × 4176, píxel 3,76 µm | Color (Bayer **RGGB**, por confirmar), 3840 × 2160, píxel 2,9 µm |
| Focal efectiva | 400 mm | ~7,89 m (nominal 7,2 m) |
| Escala | 1,932″/px (plate solving) | **0,0758 ± 0,0003″/px** (parejas de estrellas Gaia DR3) |
| Campo | 3,35° × 2,24° | 4,85′ × 2,73′ |
| Ejes de imagen | x = +Az, arriba = +El | x = +Az, arriba = +El |
| Giro respecto a la montura | −0,76° ± 0,09° | +1,0° ± 0,2° |
| Paridad | Normal (sin espejo) | Normal (sin espejo) |
| Eje del telescopio | Centro de la imagen | **Desconocido** (falta el píxel del detector C-SPAD) |
| Láser visible | Sí | Sí, pero bastante más difícil de ver |

Ojo: en el mensaje de Discord aparece "0,0003″/px" para la ASI585MC; es la incertidumbre. La escala es 0,0758″/px (206,265 × 2,9 µm / 7890 mm ≈ 0,0758).

**Convención del giro (resumen de calibración de Ángel):** es el ángulo de la cruz Az/El de la montura vista en la imagen, positivo en sentido antihorario, y se aplica a la imagen. Con +1,0° (receptor) la dirección +El se inclina 1° hacia la izquierda; con −0,76° (RASA), 0,76° hacia la derecha. Comprobable con sus números de Polaris y Matar.

**Consecuencias:**
- **Conversión px → Az/El**: hay que aplicar escala **y** giro de cada cámara. "Arriba = +El" implica que la fila de píxel crece hacia −El (signo invertido en y). Ángel expresa los desvíos en el cielo como **dAz·cos(El)** y dEl; para la montura (encoder) dAz = (dAz·cos El)/cos El. Confirmar cuál quiere en la salida.
- **Muestreo**: con seeing ~2,7″, una estrella ocupa ~36 px de FWHM en la ASI585MC (muy sobremuestreada: se puede reducir resolución para ganar velocidad sin perder precisión) y ~1,4 px en la RASA (submuestreada: ahí el centroide subpíxel sí es crítico).
- **Saturación**: muchas estrellas brillantes saturan en RAW8. El FWHM de una estrella saturada no es el seeing, y el centroide puede sesgarse. Hay que detectar y marcar saturación.
- **Centro de imagen ≠ eje del receptor ≠ centro del diafragma**: el diafragma de campo de SFEL está desplazado unos (+45…+53, +41…+80) px del centro, y el píxel del C-SPAD no se conoce. El punto de referencia debe ser **configurable** (por defecto, centro de imagen).
- **Agrupación 2×2**: todo el material está en Bin 1, sin agrupar. Ángel cree que la calibración también se hizo sin agrupación (lo revisará), aunque sus imágenes muestran picos de 1020 = 4 × 255. Agrupar puede dar una solución buena y más rápida: queda como experimento.

## 3. Datos
- **No hay FITS**: el sistema real trabaja con imágenes en memoria, que no se almacenan. Para pruebas: **PNG y vídeos**. CFITSIO no es necesario.
- **RAW8** por ahora; RAW16 más adelante.
- Vídeos reales de pasadas: pendientes. En ellos los operadores mueven el haz y la montura, y el objeto "se vuelve loco" mientras la montura se estabiliza: **hay que detectar y descartar esos tramos**.
- Ejemplos de calibración (28/10/2026): series de Polaris, Matar, Altair y FK5 1481, con capturas PNG `CapObj_XXXX.PNG` de ambas cámaras.
- Coordenadas de la estación (ILRS): 36,465° N, 6,205° W, 98 m.

**Formato de las capturas de ASICap** (ejemplo: `2026-10-28-1918_6-CapObj_0000.PNG`, Altair, ASI2600MM):
- Nombre: `AAAA-MM-DD-HHMM_d-CapObj_NNNN.PNG`. La hora parece seguir el formato WinJUPOS: `1918_6` = 19:18 + 6 décimas de minuto = 19:18:36, en UTC (coincide con las series "…_19_18_..Z" de Ángel). `NNNN` = número de fotograma dentro de la serie. Confirmar.
- El `.txt` empieza con una cabecera que **identifica la cámara** (`[ZWO ASI2600MM Pro]`) y sigue con líneas `Clave = Valor`, con unidades dentro del valor (`100ms`, `-0.5 C`, `6248 * 4176`). Incluye además `Brightness = 1` (offset o nivel de negro del sensor), `Hardware Bin = OFF`, `High Speed Mode = OFF`, `USB Limit = 100` y parámetros de autoexposición (no usada). La serie PNG de ejemplo es de sensor completo (`StartX = 0`, `StartY = 0`, 6248 × 4176) y su `.txt` no trae `StartCapture`/`EndCapture`/`FrameCount`. El `.txt` del vídeo AVI añade `Output Format = *.AVI`, `StartCapture`, `EndCapture`, `TimeZone = 0`, `FrameCount = 226` y `Capture Limit = No Limit`. **La ganancia cambia entre series** (606 en la serie PNG, 500 en el AVI): es un dato por serie, no fijo. La hora del nombre del AVI (`1916_5` = 19:16:30) frente a `StartCapture` (19:16:31,237 = 5,2 décimas de minuto) es compatible con truncar y con redondear, pero el análisis del paquete del ROA (17/11) lo resuelve: en 6 de 8 sesiones los dos criterios difieren y las 6 coinciden con **truncar**.
- Cada serie trae un fichero `.txt` con los ajustes de cámara: exposición (100 ms), ganancia (606), Bin 1, área de captura y su origen (StartX/StartY), formato RAW8, Debayer NONE, temperatura del sensor (−0,5 °C), **Flip = Horizon** y **Timestamp Frames = OFF**.
- **Vídeos AVI** (ejemplo: `2026-10-28-1916_5-CapObj.AVI`, ASI2600MM, Altair): el `.txt` añade `StartCapture = 2026-10-28T19:16:31.237Z`, `EndCapture = 2026-10-28T19:17:35.494Z`, `TimeZone = 0` y `FrameCount = 226`. Eso da ~3,5 fps (≈0,286 s entre fotogramas, con 100 ms de exposición: el límite lo pone la lectura USB del sensor de 26 MP). Confirma que el nombre usa HHMM_décimas de minuto en UTC (1916_5 = 19:16:30). Tamaño: 226 × 6248 × 4176 bytes ≈ 5,9 GB sin comprimir.
- La configuración PoC compila opencv4 **sin la opción ffmpeg** (olvido reconocido por Ángel): probablemente `cv::VideoCapture` no podrá abrir estos AVI sin compresión. Comprobar con `cv::getBuildInformation()` (apartado Video I/O). Sugerencia de Ángel: probar **QtMultimedia** (está en el entorno, con FFmpeg). Ojo: QtMultimedia está pensado para reproducir y puede convertir los fotogramas a color; hay que verificar que entrega los valores RAW8 sin alterar. Alternativas: convertir a secuencia de PNG, o un lector AVI propio (contenedor RIFF con fotogramas sin comprimir, sencillo). En Python, `opencv-python` sí trae FFmpeg.
- **Registro de posiciones de la montura**: debería estar en la carpeta con todas las imágenes del primer día de pruebas. Cruzando sus tiempos con los de las capturas se obtienen Az/El de cada fotograma. Si no está, avisar al ROA.
- **No hay hora por fotograma** (Timestamp Frames OFF) ni datos de la montura (Az/El). La hora de cada fotograma solo se puede aproximar a partir de la hora de inicio de la serie y el número de fotograma.
- **Flip = Horizon**: la imagen está volteada horizontalmente al capturar. La convención "x = +Az" de la calibración vale para imágenes con este volteo; si la fuente (p. ej. DegorasASI) no lo aplica, el signo de x cambia. El volteo debe ser un parámetro de la calibración de cada cámara.
- Para estrellas, la elevación se puede calcular a partir de la hora UTC, las coordenadas de la estación y la posición de catálogo de la estrella (p. ej. con Astropy). Para satélites haría falta la predicción orbital o la telemetría de la montura.

## 4. Método de referencia del ROA (experimentos de calibración)
Ángel usó una librería C++ que reimplementa ASTAP y un detector rápido propio; os pasará el código y se puede integrar directamente.
- Detección: fondo + 3σ.
- Centroide: centroide en ventana (ventana de 4σ) y centro de gravedad, para comparar.
- Seeing: FWHM de estrellas no saturadas (mediana 2,7″ esa noche).
- Overlay de referencia: estrella principal (rojo), otras estrellas (verde), trayectoria en todos los fotogramas (amarillo), centro de imagen (magenta), diafragma (naranja), campo del C-SPAD (cian), ejes +Az/+El, escala en arcsec y texto con centroide, desvío en px y en arcsec.

## 5. Requisitos de la aplicación
- **Aplicación de experimentación**, escritorio, **Qt** simple. Sin interfaces web.
- **Entrada**: ficheros (imágenes y vídeos). En el sistema real se leerá de las cámaras con **DegorasASI** (wrapper del ROA sobre el SDK de ZWO). No es obligatorio integrarlo, pero **la arquitectura debe permitirlo fácilmente** (interfaz común de fuente de fotogramas).
- **Una sola aplicación con tres modos**: reconocimiento de **estrellas**, de **satélite** y del **haz láser** (propuesta de Ángel, "lo más elegante").
- **Salida**: estructura C++ **serializable a JSON** (nlohmann-json, ya en el entorno). Mostrada como etiquetas y como overlay sobre el vídeo/imagen. Campos mínimos: desvío respecto al centro en **píxeles y en arcsec**, con el acimut **en las dos formas**: del cielo (dAz·cos El) y del encoder (dAz); posición del centroide, estado de detección; ampliable (seeing/FWHM, haz láser si se detecta, etc.).
- **Satélites: trabajo relativo al centro de la cámara.** No hace falta su posición real en el cielo: con la escala calibrada (″/px) basta la distancia al centro.
- **Señal de "montura aplicando offsets / en tránsito"**: hoy no existe; el ROA la añadirá a su montura. La aplicación debe estar preparada para recibirla y **dejar de actuar mientras esté activa**. El filtrado de maniobras por análisis de imagen es complementario y también válido.
- **Volteo (Flip)**: la calibración se hizo con la misma configuración que el material recibido (Flip = Horizon): no hay que voltear nada. DegorasASI es configurable y aplicará el volteo que haga falta.
- **Red (ZMQ)**: solo si da tiempo. Ojo: zeromq/cppzmq no están en la configuración PoC-proc-video.
- **Haz láser**: lo hacemos nosotros. Color → canal verde; monocroma → por forma o temporal. No siempre se verá; si se detecta, se añade a la salida.
- **Plate solving**: librería **faster-astap** (github.com/Pop-Vlad/faster-astap), port C++17 del solver de ASTAP, con detección de estrellas y resolución de placas funcionales. Ángel le ha hecho un port de vcpkg y está desarrollando una versión mejorada propia; mientras tanto, usar faster-astap más la algoritmia mejorada que enviará (junto con los índices de estrellas vía Mega). El ROA también empieza ahora con esto: no tiene software previo propio de detección de estrellas.
  - Dos ejecutables: `astap_solve` (réplica exacta de `astap_cli`, 8 veces más rápido) y `astap_index_solve` (índice de todo el cielo: resuelve en milisegundos sin saber hacia dónde apuntaba el telescopio, útil para blind solving y campos amplios).
  - Paquete de Python (`pip install faster-astap`): útil para experimentos y para resolver imágenes ya en memoria.
  - **Licencia MPL 2.0**: copyleft por fichero. Los ficheros de faster-astap que se modifiquen deben seguir publicándose bajo MPL; el código propio que solo la use puede tener otra licencia.
  - Con las posiciones de la montura: **local solving** (acotado). Sin ellas: **blind solving**. Una vez resuelta una placa, el re-solving es rápido.
  - ASTAP como programa gráfico es solo para stacking.
- Para satélites no hay herramienta conocida: toca investigar (papers, OpenCV, librería ASTAP).
- **Python + OpenCV** permitido para experimentos.
- Tiempo real de operador (~100 ms/fotograma); arranque de 4–5 s aceptable.
- Pérdida de blanco: estado "perdido", último valor congelado, sin excepciones ni cierres.

## 6. Flujo de procesado (propuesta)
1. **Estabilidad**: detectar maniobras de la montura/haz (salto brusco del centroide, emborronamiento, cambio global de la imagen) y marcar el fotograma como "inestable" sin usarlo para el vector.
2. **Preprocesado**: fondo (mediana móvil o por zonas), ruido, máscara del diafragma, saturación.
3. **Detección y centroide**: fondo + kσ, centroide en ventana; para SFEL, opcionalmente superpíxel/binning.
4. **Desvío**: respecto al punto de referencia configurable, en px y en arcsec Az/El (escala + giro + signos).
5. **Haz láser**: verde en la 585MC; forma/temporal en la 2600MM.
6. **Salida**: estructura → JSON, etiquetas y overlay.

Máquina de estados: *buscando → siguiendo → inestable (maniobra) → siguiendo*, y *perdido* (último valor congelado).

## 7. Entorno de desarrollo
- Generador: github.com/AngelVeraHerrera/WindowsDevDrive-Env-Setup. Configuración oficial: **`drivenv-cfg_proc-video_v1.0.0.json`** (Degoras Project PoC-proc-video, 9/11/2026).
- **Toolchain recomendado: ucrt64** (GCC 16.2.0). El código debería compilar también con clang64.
- **Letra libre** por desarrollador (las rutas salen del `.env`). En el PC de Juan: **S:**, entorno `DegorasPoCProcVideo`.
- Sin módulo Hyper-V (Windows Home, o Pro sin activarlo): `force_diskpart: true`. Ángel pide feedback de ese camino: el paso 1 aborta por falta de Hyper-V antes de llegar a su propia alternativa con diskpart.
- Generación completa: ~6 h 30 min, pico de 17,5 GB de RAM (OpenCV con GCC). Con 16 GB de RAM, bajar concurrencia o importar un disco exportado (`tools\Export-DevDrive.ps1`).
- Versiones instaladas: Qt 6.11.2, OpenCV 4.14.0, spdlog 1.17.0, fmt 12.2.0, nlohmann-json, curl 8.23.0, openssl 3.6.5.
- El VHDX se monta con una tarea al arrancar; si S: no aparece, montar el `.vhdx` de `C:\DevDrives` con doble clic.
- Lanzadores: `S:\env\launcher\degoraspocprocvideo_ucrt64_env_launcher.bat` y `..._qtcreator.bat`. Builds en `S:\bt\<triplet>`. CMake + Ninja con el toolchain de vcpkg. QMake prohibido.

## 8. Arquitectura propuesta
1. **frame_source**: interfaz "siguiente fotograma + marca de tiempo + metadatos de cámara". Implementaciones: PNG (secuencia), vídeo. Futuro: DegorasASI.
2. **core** (C++ puro, sin Qt): estabilidad, preprocesado, detección, centroide, haz, conversión px → arcsec, máquina de estados. Devuelve siempre un resultado con estado.
3. **calibration**: parámetros por cámara (escala, giro, ejes, punto de referencia), cargados de un fichero JSON.
4. **astrometry**: adaptador a faster-astap (y después a la librería mejorada del ROA, con la misma interfaz), fuera del bucle de 100 ms.
5b. **mount_signal**: entrada para la señal "montura aplicando offsets / en tránsito" (hoy simulada o manual); mientras está activa, el core no actúa.
Modos de la aplicación: estrellas / satélite / haz láser, sobre el mismo core.
5. **result**: estructura de salida + serialización JSON.
6. **gui** (Qt Widgets): vídeo, overlay y etiquetas; procesado en otro hilo.
7. **cli + tests**: procesar series sin GUI y volcar CSV/JSON para la memoria de I+D.

## 9. Preguntas abiertas
Resueltas el 11/11/2026: salida de acimut (ambas formas), registro de la montura (en la carpeta de imágenes), señal de maniobra (la añadirá el ROA), volteo (no hace falta), FFmpeg (probar QtMultimedia), agrupación 2×2 (no se usa; experimentar), plate solving (faster-astap).

Resueltas después (resumen de calibración): convención del giro (ver §2); píxel del C-SPAD (desconocido, primer punto de "qué falta" en la calibración).

Pendientes:
- Localizar el registro de posiciones de la montura en la carpeta de imágenes; si no está, avisar.
- Cuándo se medirá el píxel del C-SPAD.
- Patrón Bayer: Ángel cree que es RGGB. Con el volteo horizontal (ancho par) el mosaico guardado pasaría a GRBG. Se comprueba con la prueba de los subplanos sobre un PNG real.
- Pregunta 3 mal entendida ("detección de estrellas evitando basura espacial"): reformularla si sigue haciendo falta (¿cómo distinguir estrella de satélite/basura en la imagen? ¿cómo validar el modo estrellas?).
- Formato en que llegará la señal de "montura aplicando offsets" (para dejar el hueco preparado).
- Envío de la algoritmia mejorada, el port de vcpkg de faster-astap y los índices.
- ¿Añadirá FFmpeg a opencv4 si QtMultimedia no sirve?
- Si al final hay ZMQ, añadir zeromq/cppzmq a la configuración.

## 10. Organización
- Comunicación: Discord con Ángel Vera (entorno, arquitectura, ASTAP) y los técnicos de la estación.
- Plazo: final del cuatrimestre.
- Registro de experimentos desde el primer día (técnica, parámetros, datos, resultado, decisión).
- **Uso de la IA**:  La IA solo se usa para resolver dudas, sugerir ideas de algoritmos, revisar y hacer pruebas rápidas.

## 11. Material entregado por el ROA (17/11/2026) y astrometría precisa
**Paquete de Ángel (PoF de estrellas):** librería C++17 con CMake y presets del entorno. Incluye:
- faster-astap como código fuente, detrás de `solve/solver.h`.
- Algoritmos rápidos del experimento portados: detección, resolución de placa en 12–17 ms, medida de estrellas FK5, la estrella del receptor de SFEL, Az/El y modelo de cámara.
- Tests y dos herramientas: `dpot_identify` y `dpot_spot`.
- Imágenes reales del 28/10/2026 (RASA-8 y receptor SFEL) **con la hora de cada una**.
- El experimento original completo (documentos, C++ y Python).
- No incluye las bases de estrellas de ASTAP: se descargan según `databases/LEEME.txt`.

Es el punto de partida del modo estrellas. Antes de seguir con los contratos propios (calibración, salida), hay que leer su API (`pointing`, modelo de cámara, `solve`) y alinear el diseño con ella, para no duplicarla.

**Az/El observados de estrellas: usar SuperNOVAS**, no la aproximación de `pointing/horizon.h`, que no aplica nutación, aberración ni refracción.
- Instalación: `vcpkg install supernovas` en la shell de **cada** toolchain (ucrt64 y clang64). Enlazar desde la capa `pointing` con `find_package(supernovas CONFIG REQUIRED)`.
- Posición de la estación: `pointing::sfelSite()`.
- Parámetros de orientación de la Tierra (interpolar linealmente a la hora de cada imagen):

| Fecha | xp (″) | yp (″) | UT1−UTC (s) |
|---|---|---|---|
| 2026-10-28, 0h UTC | 0,144251 | 0,318440 | −0,0517904 |
| 2026-10-29, 0h UTC | 0,142830 | 0,318192 | −0,0526037 |

  TAI−UTC = 37 s. Hacia las 19:12 UTC del 28/10 (fracción ≈ 0,8): xp ≈ 0,143114″, yp ≈ 0,318242″, UT1−UTC ≈ −0,052441 s.
- **Comprobar en la documentación de SuperNOVAS las unidades de cada función** (″ o mas). Según su guía, desde la v1.8 los segundos intercalares van fijados en la librería y se ignora el parámetro de entrada.
- **Refracción:** presión, temperatura y humedad de esa noche, de un histórico meteorológico. Si no se encuentran, atmósfera estándar a nivel del mar. La presión debe ser la de la estación (98 m), no la reducida al nivel del mar (QNH) de los METAR.

**Hallazgos al analizar el paquete (17/11/2026):**
- Librería estática C++17 `DegorasOpticalTracking` (namespace `dpot`), 8 capas con reglas de dependencia comprobadas por un test. No usa vcpkg ni OpenCV: faster-astap y stb van en `thirdparty/`. Compila con ucrt64 sin warnings; 13 tests pasan; `IT_SolveRasa` necesita la base D80 (1,3 GB, no incluida).
- Píxeles: base 0, x a la derecha, y hacia abajo, centro del píxel superior izquierdo en (0,0). Ángulos en radianes salvo sufijo Deg/Arcsec.
- `common::Frame`: `width`, `height`, `bitDepth`, `vector<uint16_t> px`; los datos de 8 bits van ×257 y `raw(x,y)` devuelve el valor original.
- `pointing::CameraModel{arcsecPerPixel, rollDeg (antihorario), mirrored, axisX, axisY}` con las dos cámaras fijas en código; `frameToMount`/`starOffset` → `MountOffset{dAzCosEl, dEl}` en arcsec (falta dAz del encoder). El giro de la RASA se documenta como dependiente de la elevación (−0,6° a −0,8°). `sfelSite()` = 36,465258° N, 6,205305° W, 98,2684 m.
- `solve::Solver` (faster-astap): sin excepciones, un solve a la vez por instancia; 12–17 ms por imagen RASA en memoria según su documentación (no verificado). Leer un PNG de 26 MP cuesta ~100 ms.
- **El `.txt` de ASICap es por fotograma** (`<nombre>.PNG.txt`; 367 para 372 PNG) y **los ajustes cambian dentro de una serie** (p. ej. 5 ganancias distintas en 17 fotogramas de RASA/Matar).
- **Hora por fotograma:** `images/frame_times.csv` (`path,utc`, ISO 8601 con ms y Z): hora de escritura del PNG según el reloj del PC de captura, exactitud absoluta sin comprobar; el primer fotograma se escribe 0,2–1,1 s después del inicio de sesión. Respaldos: carpeta de sesión (inicio al segundo, UTC) y nombre del fichero (truncado). Periodos medidos: RASA 0,285 s; SFEL 0,240 s.
- **`.txt` de la ASI585MC:** cabecera `[ZWO ASI585MC Pro]`, `Debayer Type = RGGB` con `Raw Format = ON` (mosaico en bruto) y **`Flip = None`** (la 2600MM usa `Horizon`).
- SuperNOVAS en la baseline del Dev Drive: **v1.7.2** (Unlicense, depende de curl). Lo de los segundos intercalares fijos es de la v1.8: revisar la documentación de la 1.7.2.
- **Licencias:** el paquete es **GPL-3.0-or-later** (Degoras Project Team); faster-astap MPL-2.0 (con un parche mínimo); stb MIT/dominio público; catálogo IAU-CSN CC con atribución. Si se enlaza o copia el paquete, lo que se distribuya debe ser GPL-3.0-or-later.

**Respuestas de Ángel (18/11/2026):**
- **Licencia:** "similar" a la del paquete → el repo del grupo irá bajo **GPL-3.0-or-later** (necesario para combinarlo con `dpot`). El copyright de nuestro código puede ir **a nombre de los autores**; al integrarlo, el ROA nos mencionará como colaboradores en el software principal.
- **Giro de la RASA:** basta con el **valor fijo** (−0,76°); no hace falta modelar su dependencia con la elevación.
- **Paquete `dpot`:** no tiene repositorio propio; es una librería de prueba de concepto. **Libertad completa**: podemos ponerla donde queramos, integrarla como queramos y modificarla. (Los ficheros de faster-astap siguen bajo MPL-2.0: conservar su licencia y atribución.)

**SuperNOVAS (23/11/2026):** el port de la baseline (1.7.2) no compila con MinGW (`novas-mutex.h` busca `<threads.h>`, que MSYS2 no trae). Ángel facilitó un **overlay port de SuperNOVAS 1.8.0** con el parche `mingw-srwlock.patch` (en `_WIN32` usa la API de bloqueo de Windows). Se instala con la feature C++: `vcpkg install "supernovas[cpp]" --overlay-ports=<repo>/vcpkg-overlays`, en los dos toolchains. Al ser la 1.8, los segundos intercalares van fijados en la librería (comprobar en su `novas.h`).

