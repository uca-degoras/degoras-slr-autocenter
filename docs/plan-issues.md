# Plan de issues – Fases 1 y 2

Issues del proyecto. Cada issue describe la tarea; el diseño se redacta en `docs/diseno/`, el
código en `libs/`, `apps/` o `prototypes/`, y todo se revisa mediante pull request.

**Milestones:** issues 1–9 → *Fase 1 – Contratos* (diseño). Issues 10–17 → *Fase 2 – Pipeline con
datos de calibración* (implementación y prototipos que ya se pueden empezar).
**Referencias:** apartados (§) de `docs/contexto_proyecto.md` (versión del 18/11/2026).
**Reparto:** por carga de trabajo, no por equipos de ámbito (ver la tabla del final). Cada persona
redacta o programa al menos una issue y revisa varias. Los responsables son orientativos y se
pueden cambiar de común acuerdo.
**Orden recomendado:** Fase 1: 1 → 3 → 2 → 4 → 5 → 6; la 7, la 8 y la 9 en paralelo.
Fase 2: la 13 y la 16 pueden empezar ya; la 10, la 11 y la 12 cuando se apruebe la 1; la 14 y la 15
cuando exista la 13; la 17 cuando el ROA responda sobre SuperNOVAS.
**Revisión:** basta con una aprobación para fusionar (regla de `main`). Cuando una issue tiene
varios revisores, se espera la opinión de todos antes de fusionar; los marcados como
opcionales no bloquean. Quien hace una PR no la aprueba.

---

## 1. Contrato de la interfaz de fuente de fotogramas

- **Labels:** `diseño`, `docs`
- **Responsable:** Juan · **Revisores:** Javi, Ramón
- **Estado:** actualizado con el paquete `dpot` (Frame de `dpot`, `.txt` y ajustes por fotograma, hora desde `frame_times.csv`); en revisión (`docs/diseno/fuente_fotogramas.md`).
- **Objetivo:** Documentar el contrato de `frame_source` que permita sustituir los ficheros (PNG, vídeo) por DegorasASI sin tocar el core.
- **Requisito que cubre:** §3 (formato de las capturas, hora estimada, metadatos del `.txt`), §5 (Entrada; la arquitectura debe permitir DegorasASI) y §8.1.
- **Depende de:** —
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Recoge qué entrega la fuente por fotograma (imagen en bruto, profundidad de bits y nivel de saturación, fase Bayer, marca de tiempo con su calidad, metadatos de serie) y cómo se señalan el fin, los fotogramas ilegibles y los fallos sin excepciones.
  - [ ] Indica las implementaciones previstas (secuencia PNG, vídeo, futuro DegorasASI) y justifica la ausencia de dependencia de Qt.
  - [ ] Define el ritmo de reproducción (Fast / Realtime) y la política de descarte.
  - [ ] Alineado con la librería `dpot` (§11): qué tipo de imagen se usa (`dpot::common::Frame` o `cv::Mat`) y cómo se convierte sin copias innecesarias.
  - [ ] Las dudas sobre el formato de vídeo, FFmpeg, el patrón Bayer y DegorasASI están listadas como preguntas al ROA.

## 2. Contrato de salida (resultado y JSON)

- **Labels:** `diseño`, `docs`, `pregunta-ROA`
- **Responsable:** Dani · **Revisores:** Jose (cumplimiento del §5 y del DERS), Juan
- **Objetivo:** Documentar la estructura de resultado que produce el core y su serialización a JSON (nlohmann-json).
- **Requisito que cubre:** §5 (Salida) y §8.5.
- **Depende de:** 1 (índice, hora y calidad del fotograma; descartados) y 3 (conversión px → arcsec).
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Cubre los campos mínimos de §5: desvío en píxeles y en arcsec, acimut en sus dos formas (dAz·cos El y dAz), posición del centroide y estado de detección, y cómo se amplía (FWHM, haz láser, saturación…).
  - [ ] Define cómo se representa el estado, incluidos «perdido» con el último valor congelado e «inestable».
  - [ ] Define qué se publica cuando no se conoce la elevación (dAz no calculable) o el fotograma es ilegible.
  - [ ] Lista como pregunta al ROA cualquier detalle del formato JSON que deban consumir ellos.

## 3. Contrato de calibración por cámara

- **Labels:** `diseño`, `docs`, `pregunta-ROA`
- **Responsable:** Juan · **Revisores:** Manulo, Adriana
- **Objetivo:** Documentar los parámetros de calibración de cada cámara, la conversión de píxeles a arcsec Az/El y el formato del fichero JSON que los carga, partiendo del `pointing::CameraModel` de `dpot` (escala, giro, paridad y eje ya implementados) y completando lo que le falta.
- **Requisito que cubre:** §2 (tabla de telescopios y cámaras y sus consecuencias) y §8.3.
- **Depende de:** 1 (`camera_id`, recorte StartX/StartY, volteo de adquisición) y 8 (uso de `dpot`).
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Cada parámetro está ligado a la fila de la tabla de §2 de la que procede, con su incertidumbre.
  - [ ] Recoge la convención del giro confirmada por el ROA (ángulo de la cruz Az/El en la imagen, antihorario positivo, valor fijo por cámara) y la de los ejes, con la fórmula px → arcsec y un ejemplo numérico por cámara.
  - [ ] Añade lo que `CameraModel` no tiene: carga desde JSON, `camera_id`, volteo de adquisición (distinto de `mirrored`), tamaño del sensor y dAz del encoder.
  - [ ] Define los puntos de referencia (centro de imagen por defecto, diafragma, eje del receptor) y cómo les afecta el recorte.
  - [ ] Define qué ocurre si el `camera_id` no tiene calibración o no casa con la serie.
  - [ ] Queda claro qué valores están confirmados y cuáles no (eje del receptor/C-SPAD, patrón Bayer).
  - [ ] Las dudas abiertas están listadas como preguntas al ROA.

## 4. Máquina de estados del seguimiento

- **Labels:** `diseño`, `docs`
- **Responsable:** Juan · **Revisores:** Ramón, Mireya; opcional: Santi (requisito de robustez, §5)
- **Objetivo:** Documentar los estados del seguimiento y las condiciones de transición entre ellos.
- **Requisito que cubre:** §5 (pérdida de blanco sin excepciones ni cierres) y §6 (estabilidad y máquina de estados).
- **Depende de:** 1 (fotogramas ilegibles y descartados) y 2 (qué se publica en cada estado).
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Incluye diagrama de estados (Mermaid) con los estados de §6: buscando, siguiendo, inestable y perdido.
  - [ ] Cada transición indica qué evento la dispara, con sus parámetros configurables, y qué se publica en el resultado.
  - [ ] Define la prioridad entre «inestable» y la señal de montura activa (issue 5).
  - [ ] Indica cómo se comprobará en un experimento que no hay cierres ni excepciones al perder el blanco.

## 5. Contrato de la señal de montura

- **Labels:** `diseño`, `docs`, `pregunta-ROA`
- **Responsable:** Guille · **Revisores:** Adrián, Jose
- **Objetivo:** Documentar la entrada `mount_signal` («montura aplicando offsets / en tránsito») y el comportamiento del core mientras está activa.
- **Requisito que cubre:** §5 (señal de montura) y §8 (`mount_signal`).
- **Depende de:** 4.
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Indica qué hace el core mientras la señal está activa y al desactivarse.
  - [ ] Define cómo se simula o se activa a mano mientras el ROA no la tenga.
  - [ ] El formato en que el ROA enviará la señal figura como pregunta pendiente para Discord.

## 6. Arquitectura de módulos y dependencias

- **Labels:** `diseño`, `docs`
- **Responsable:** Guille, con Juan · **Revisores:** Javi, Dani
- **Objetivo:** Cerrar la arquitectura de la fase: módulos, responsabilidades, dependencias entre targets CMake y modos de la aplicación (estrellas, satélite, haz láser).
- **Requisito que cubre:** §5 (una aplicación con tres modos; Qt solo en la interfaz) y §8 (módulos).
- **Depende de:** 1–5.
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Diagrama (Mermaid) de módulos y dependencias, con Qt solo en `apps/gui`.
  - [ ] Indica qué hilo ejecuta cada parte (lectura, procesado, GUI).
  - [ ] Indica dónde encajan `libs/dpot` y faster-astap, y qué implican sus licencias (GPL-3.0-or-later del repo; MPL-2.0 en los ficheros de faster-astap).

## 7. Plantilla del registro de experimentos y formato de volcado de la CLI

- **Labels:** `diseño`, `experimento`, `docs`
- **Responsable:** Mireya y Adriana · **Revisores:** Ramón, Manulo
- **Objetivo:** Definir la plantilla común del registro de experimentos y el formato CSV/JSON que vuelca la CLI, para comparar técnicas y alimentar la memoria de I+D.
- **Requisito que cubre:** §8.7 (cli + tests) y §10 (registro de experimentos desde el primer día).
- **Depende de:** 2 (campos del resultado).
- **Criterios de terminado:**
  - [ ] Plantilla en `docs/experimentos/` con: técnica, parámetros, datos usados, métrica, resultado, coste por fotograma y decisión (también para lo descartado).
  - [ ] Formato de volcado de la CLI definido (columnas del CSV o esquema JSON).
  - [ ] Revisado por el equipo y por los testers.

## 8. Modo estrellas: adopción de la librería `dpot` y Az/El con SuperNOVAS

- **Labels:** `diseño`, `mapeo-estrellas`, `docs`
- **Responsable:** Guille · **Revisores:** Adrián, Juan
- **Objetivo:** Decidir y documentar cómo se usa la librería del ROA (`DegorasOpticalTracking`, `dpot`) en el modo estrellas, y diseñar el cálculo de Az/El observados con SuperNOVAS en lugar de `pointing/horizon.h`.
- **Requisito que cubre:** §4 (método de referencia del ROA), §5 (plate solving, modo estrellas) y §11 (paquete del ROA y astrometría precisa).
- **Depende de:** integración de `dpot` en `libs/dpot/` (PR de integración). Relacionada con la 3 (el `CameraModel` de `dpot` cubre parte de la calibración).
- **Criterios de terminado:**
  - [ ] Documento en `docs/diseno/` revisado y aprobado por el equipo.
  - [ ] Lista qué partes de `dpot` se usan tal cual (detección, medida, plate solving, modelo de cámara) y qué falta para el modo estrellas.
  - [ ] Base D80 descargada según `databases/LEEME.txt` (fuera del repo) y `IT_SolveRasa` ejecutado; tiempo de solve medido y anotado en el registro de experimentos.
  - [ ] Cálculo de Az/El con SuperNOVAS 1.7.2 diseñado: unidades de cada función comprobadas en `novas.h`, EOP del §11 interpolados por imagen, TAI−UTC y posición de `pointing::sfelSite()`.
  - [ ] Refracción con presión (en la estación, no QNH), temperatura y humedad de la noche del 28/10, citando la fuente; si no se encuentran, atmósfera estándar.
  - [ ] SuperNOVAS instalado en ucrt64 y clang64, con el comando documentado en el README.

## 9. Diseño de la interfaz: bocetos de los tres modos y overlay

- **Labels:** `diseño`, `gui`, `docs`
- **Responsable:** Dani · **Revisores:** Mireya, Juan
- **Objetivo:** Diseñar la interfaz Qt de la aplicación: los tres modos (estrellas, satélite y haz láser), las etiquetas de resultado y estado, y el overlay sobre la imagen.
- **Requisito que cubre:** §4 (overlay de referencia del ROA), §5 (aplicación Qt simple, tres modos, salida como etiquetas y overlay) y §8.6 (gui).
- **Depende de:** 2 (campos de la salida que muestra) y 4 (estados que se indican al operador).
- **Criterios de terminado:**
  - [ ] Bocetos en `docs/diseno/` revisados y aprobados por el equipo.
  - [ ] El overlay sigue la referencia del ROA (§4): estrella principal en rojo, otras en verde, trayectoria en amarillo, centro de imagen en magenta, diafragma en naranja, campo del C-SPAD en cian, ejes +Az/+El y escala en arcsec.
  - [ ] Indica cómo se muestran los estados (buscando, siguiendo, inestable, perdido) y el número de fotogramas descartados.
  - [ ] Lista los campos de la salida que necesita la interfaz (entrada para la issue 2).
  - [ ] Esqueleto Qt Widgets que muestra una imagen con un overlay de prueba, sin lógica de procesado (Qt solo en `apps/gui`).

---

# Fase 2 – Pipeline con datos de calibración

Implementación y prototipos. Reglas comunes: PR pequeñas (un tema por PR), tests con datos
sintéticos de solución conocida, compilar con ucrt64 y clang64, y cada técnica probada con su
entrada en `docs/experimentos/` (también lo descartado).

## 10. Lector del `.txt` de ASICap y metadatos por fotograma

- **Labels:** `imagen-apuntado`
- **Responsable:** Javi · **Revisores:** Adriana, Manulo
- **Objetivo:** Implementar el lector del `.txt` por fotograma descrito en `fuente_fotogramas.md` (§3.1): cabecera con el modelo, `Clave = Valor` con unidades, claves obligatorias y opcionales, avisos.
- **Requisito que cubre:** §3 y §11 del contexto; contrato de la issue 1.
- **Depende de:** 1 (aprobada).
- **Criterios de terminado:**
  - [ ] Código en `libs/frame_source` sin Qt y sin excepciones hacia fuera.
  - [ ] Identificación de cámara (configuración → cabecera → tamaño) con la comprobación cruzada y la del recorte.
  - [ ] Reglas de aviso: `Raw Format`/`Debayer`, `Timestamp Frames = ON`, claves duplicadas; claves desconocidas ignoradas.
  - [ ] Tests con los tres `.txt` reales de `tests/data/asicap/` y con casos sintéticos de error.

## 11. `PngSequenceSource` y `MemoryFrameSource`

- **Labels:** `imagen-apuntado`
- **Responsable:** Manulo · **Revisores:** Mireya, Javi
- **Objetivo:** Implementar la fuente de secuencias PNG sobre `dpot` (`io::loadFrame`) y la fuente en memoria para tests, según el contrato de la issue 1.
- **Requisito que cubre:** §3, §5 (Entrada) y §8.1.
- **Depende de:** 1 (aprobada) y 10 (lector del `.txt`).
- **Criterios de terminado:**
  - [ ] Orden por `NNNN`, índice relativo al primero, huecos como `Corrupt`, duplicados avisados.
  - [ ] Validación de la cabecera PNG (IHDR), ajustes por fotograma y herencia si falta el `.txt`.
  - [ ] Hora por prioridad: `frame_times.csv` → carpeta de sesión → nombre, con calidad e incertidumbre.
  - [ ] `skip_to` sin decodificar; vista `cv::Mat` sin copia sobre el `Frame` de `dpot`.
  - [ ] Tests: orden, huecos, corruptos, horas y «ningún caso lanza».

## 12. `PacedSource` (ritmo Fast / Realtime)

- **Labels:** `imagen-apuntado`
- **Responsable:** Adrián · **Revisores:** Adriana, Ramón
- **Objetivo:** Implementar el decorador de ritmo del contrato: Fast sin descarte; Realtime entrega el fotograma del instante actual, salta con `skip_to` y cuenta `dropped` y `dropped_decoded`.
- **Requisito que cubre:** §5 (~100 ms por fotograma, simulador en vivo).
- **Depende de:** 1 (aprobada). Se prueba con `MemoryFrameSource` (11).
- **Criterios de terminado:**
  - [ ] Reloj inyectable; Realtime sin periodo falla en `open()`; `request_stop()` despierta las esperas.
  - [ ] Tests con reloj simulado, incluido el de «los saltados no se decodifican».

## 13. Generador de imágenes sintéticas de posición conocida

- **Labels:** `experimento`
- **Responsable:** Ramón · **Revisores:** Adriana, Adrián
- **Objetivo:** Script (Python, marcado PRUEBA) que genera series PNG con su `.txt` y un fichero de verdad (posición real de cada objeto): fondo con ruido, estrellas puntuales o en trazo, satélite tenue, traza del haz y saturación, para las dos cámaras (escala, FWHM, Bayer RGGB o mono).
- **Requisito que cubre:** §2 (muestreo: ~1,4 px FWHM en la RASA, ~36 px en la 585MC) y §10 (registro de experimentos).
- **Depende de:** —
- **Criterios de terminado:**
  - [ ] Parámetros configurables y semilla fija (reproducible).
  - [ ] Series de ejemplo pequeñas en `tests/data/` y el resto generadas bajo demanda.
  - [ ] Documentado en `prototypes/python/` y en el registro de experimentos.

## 14. Prototipo: detección y centroide del satélite

- **Labels:** `experimento`, `imagen-apuntado`
- **Responsable:** Ramón · **Revisores:** Mireya, Juan
- **Objetivo:** Probar en Python técnicas de fondo, detección y centroide subpíxel (umbral kσ, centroide en ventana, centro de gravedad, ajuste gaussiano), apoyándose en lo que ya hace `dpot` (`detectStars`, `findBlobs`, `measureSpot`), y distinguir satélite de estrella por elongación y movimiento.
- **Requisito que cubre:** §5 (modo satélite, relativo al centro), §6 (flujo de procesado).
- **Depende de:** 13. Se valida también con las series reales del 28/10 (estrellas).
- **Criterios de terminado:**
  - [ ] Error del centroide medido frente a la verdad sintética, por técnica y por cámara.
  - [ ] Falsos positivos y negativos medidos; coste por fotograma estimado.
  - [ ] Saturación detectada en bruto, antes de filtrar.
  - [ ] Entradas en el registro de experimentos con la técnica elegida y las descartadas.

## 15. Prototipo: detección del haz láser

- **Labels:** `experimento`, `imagen-apuntado`
- **Responsable:** Javi · **Revisores:** Mireya, Dani
- **Objetivo:** Probar en Python la detección de la traza verde: canal verde en la ASI585MC (mosaico RGGB) y forma o diferencia temporal en la ASI2600MM (mono). Evaluar el `greenExcess` de `dpot`.
- **Requisito que cubre:** §5 (haz láser: no siempre se verá; si se detecta, se añade a la salida) y §6.
- **Depende de:** 13 (traza sintética). Datos reales con haz: pendientes del ROA.
- **Criterios de terminado:**
  - [ ] Detección y posición medidas frente a la verdad sintética; casos sin haz sin falsos positivos.
  - [ ] Coste por fotograma estimado y limitaciones documentadas.
  - [ ] Entradas en el registro de experimentos.

## 16. Compilación reproducible y verificación de `dpot`

- **Labels:** `mapeo-estrellas`
- **Responsable:** Adrián · **Revisores:** Manulo, Guille
- **Objetivo:** `CMakePresets.json` en la raíz (ucrt64 y clang64, Debug y Release, como los de `dpot`) para evitar el problema del PATH, y verificar `dpot` con datos: base D80 descargada fuera del repo, `IT_SolveRasa` e `IT_SpotSfel` ejecutados.
- **Requisito que cubre:** §7 (entorno) y §11 (paquete del ROA).
- **Depende de:** —
- **Criterios de terminado:**
  - [ ] Presets funcionando en los dos toolchains; README con la forma de compilar.
  - [ ] Tests de integración ejecutados; tiempo de plate solving medido (¿12–17 ms?) y anotado en el registro de experimentos.

## 17. SuperNOVAS: instalación y cálculo de Az/El observados

- **Labels:** `mapeo-estrellas`
- **Responsable:** Manulo · **Revisores:** Adrián, Guille
- **Objetivo:** Dejar SuperNOVAS instalable en los dos toolchains (solución del ROA o overlay port en el repo con `SUPERNOVAS_USE_PTHREAD`) e implementar el cálculo de Az/El observados según el diseño de la issue 8.
- **Requisito que cubre:** §11 (SuperNOVAS, EOP, refracción).
- **Depende de:** respuesta del ROA sobre el fallo de `threads.h` y diseño de la issue 8.
- **Criterios de terminado:**
  - [ ] Instalación documentada en el README y probada en ucrt64 y clang64.
  - [ ] Az/El de estrellas FK5 de las series del 28/10 comparados con la aproximación de `pointing/horizon.h` y anotados en el registro de experimentos.

---

## Reparto de carga

| Persona | Rol | Redacta / programa | Revisa |
|---|---|---|---|
| Juan | Diseñador | 1, 3, 4 (y 6 con Guille) | 2, 8, 9, 14 |
| Guille | Diseñador | 5, 6, 8 | 16, 17 |
| Dani | Diseñador | 2, 9 | 6, 15 |
| Javi | Programador | 10, 15 | 1, 6, 11 |
| Ramón | Programador | 13, 14 | 1, 4, 7, 12 |
| Manulo | Programador | 11, 17 | 3, 7, 10, 16 |
| Adrián | Programador | 12, 16 | 5, 8, 13, 17 |
| Mireya | Tester | 7 (con Adriana) | 4, 9, 11, 14, 15 |
| Adriana | Tester | 7 (con Mireya) | 3, 10, 12, 13 |
| Jose | Analista | DERS y DAFO | 2, 5 |
| Santi | Analista | DERS y actas | 4 (opcional) |
| Antonio | Líder | Supervisión y documento final | — |

