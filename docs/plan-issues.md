# Plan de issues – Fase 1 – Contratos

Issues de la fase de contratos del proyecto. Cada issue describe la tarea; el contenido del
diseño se redacta en `docs/diseno/` y se revisa mediante pull request.

**Milestone de todas:** Fase 1 – Contratos
**Referencias:** apartados (§) de `docs/contexto_proyecto.md` (versión del 18/11/2026).
**Orden recomendado:** 1 → 3 → 2 → 4 → 5 → 6. La 7, la 8 y la 9 pueden ir en paralelo.
**Revisión:** basta con una aprobación para fusionar (regla de `main`). Cuando una issue tiene
varios revisores, se espera la opinión de todos antes de fusionar; los marcados como
opcionales no bloquean.

---

## 1. Contrato de la interfaz de fuente de fotogramas

- **Labels:** `diseño`, `docs`
- **Responsable:** Juan · **Revisores:** Javi, Ramón
- **Estado:** primera versión en revisión (`docs/diseno/fuente_fotogramas.md`). Pendiente de actualizar con el paquete `dpot` del ROA (§11): `.txt` por fotograma, ajustes por fotograma, `frame_times.csv`, ASI585MC sin volteo y tipo de imagen de `dpot`.
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
- **Responsable:** Juan · **Revisores:** Jose (cumplimiento del §5 y del DERS), Dani (lo que muestra la interfaz)
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
- **Responsable:** Juan · **Revisores:** Javi, Manulo (equipo de mapeo de estrellas, que también usa la calibración)
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
- **Responsable:** Juan · **Revisores:** Ramón, Mireya; opcionalmente Santi u otro analista, centrado en el requisito de robustez (§5)
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
- **Responsable:** Juan · **Revisores:** Javi
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
- **Responsable:** Guille y Juan · **Revisores:** Ramón, Adrián (uno por equipo)
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
- **Responsable:** Mireya y Adriana · **Revisores:** Javi, Manulo
- **Objetivo:** Definir la plantilla común del registro de experimentos y el formato CSV/JSON que vuelca la CLI, para comparar técnicas y alimentar la memoria de I+D.
- **Requisito que cubre:** §8.7 (cli + tests) y §10 (registro de experimentos desde el primer día).
- **Depende de:** 2 (campos del resultado).
- **Criterios de terminado:**
  - [ ] Plantilla en `docs/experimentos/` con: técnica, parámetros, datos usados, métrica, resultado, coste por fotograma y decisión (también para lo descartado).
  - [ ] Formato de volcado de la CLI definido (columnas del CSV o esquema JSON).
  - [ ] Revisado por el equipo y por los testers.

## 8. Modo estrellas: adopción de la librería `dpot` y Az/El con SuperNOVAS

- **Labels:** `diseño`, `mapeo-estrellas`, `docs`
- **Responsable:** Guille · **Revisores:** Manulo, Adrián
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
- **Responsable:** Dani · **Revisores:** Juan, Mireya
- **Objetivo:** Diseñar la interfaz Qt de la aplicación: los tres modos (estrellas, satélite y haz láser), las etiquetas de resultado y estado, y el overlay sobre la imagen.
- **Requisito que cubre:** §4 (overlay de referencia del ROA), §5 (aplicación Qt simple, tres modos, salida como etiquetas y overlay) y §8.6 (gui).
- **Depende de:** 2 (campos de la salida que muestra) y 4 (estados que se indican al operador).
- **Criterios de terminado:**
  - [ ] Bocetos en `docs/diseno/` revisados y aprobados por el equipo.
  - [ ] El overlay sigue la referencia del ROA (§4): estrella principal en rojo, otras en verde, trayectoria en amarillo, centro de imagen en magenta, diafragma en naranja, campo del C-SPAD en cian, ejes +Az/+El y escala en arcsec.
  - [ ] Indica cómo se muestran los estados (buscando, siguiendo, inestable, perdido) y el número de fotogramas descartados.
  - [ ] Lista los campos de la salida que necesita la interfaz (entrada para la issue 2).
  - [ ] Esqueleto Qt Widgets que muestra una imagen con un overlay de prueba, sin lógica de procesado (Qt solo en `apps/gui`).

