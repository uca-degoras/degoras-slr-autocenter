# 03 · Algoritmos y resultados

Lo que hace cada paso, cuánto tarda y qué precisión da con las imágenes del 2026-10-28. Los análisis completos están en
`experimento/docs/` (los números entre paréntesis son sus documentos).

## 1. Las cámaras

| | RASA 8 + ASI2600MM Pro | Receptor de SFEL + ASI585MC Pro |
|---|---|---|
| Imagen | 6248×4176, RAW8 monocromo | 3840×2160, RAW8 Bayer RGGB |
| Escala | 1,932″/px (focal ~401 mm) | 0,0758″/px (focal ~7,89 m, no los 7,2 m nominales) (12) |
| Campo | 3,36° × 2,25° | ~4,9′ × 2,7′, recortado por el diafragma |
| Orientación | x = +Az, arriba = +El, giro −0,76° (−0,6…−0,8 según la elevación; causa sin determinar) (11, 12) | x = +Az, arriba = +El, giro +1,0° (12) |
| Qué se ve | Cientos de estrellas: se resuelve la placa | Una estrella y casi nada más: se mide, no se resuelve |

Las dos cámaras van fijas a los tubos de la montura alt-az y toman imágenes a la vez. El eje del RASA apunta ~98″ en Az
por delante del de SFEL (11).

## 2. Resolver una imagen del RASA (`solve::Solver`)

**A ciegas** (sin posición; lo que hace `dpot_identify` por defecto):

| Paso | Qué hace | ms |
|---|---|---|
| Binning 3×3 | `detection::binFrame`: 9 veces menos píxeles; las estrellas siguen teniendo 2–3 px | ~5 |
| Fondo | `BackgroundGrid`: mediana y MAD por celdas de 128 px (sigue gradientes como la Luna) | ~1 |
| Detección | `detectStars`: máximos locales a 5σ con dos vecinos encendidos, centroide 5×5, las 200 más brillantes | ~1 |
| Solución | faster-astap: cuadriláteros contra el índice de 7 niveles (4…250 estrellas/grado²) | 3–5 |
| Refinado | faster-astap: segunda pasada contra la D80, con distorsión SIP | ~0,4 |
| Medida | `measure::measureStar` de cada FK5 del campo a resolución completa | ~0,4 |

En total, **12–17 ms** por imagen ya en memoria (media por objetivo, Release, 16 hilos); leer el PNG cuesta ~100 ms
más. Las 72 imágenes del RASA se resuelven todas, a 1,932″/px, y la estrella objetivo cae de media a esta distancia de
donde la predice la solución:

| Objetivo | Imágenes | ms por imagen | Objetivo medido − predicho |
|---|---|---|---|
| ALTAIR (FK5 745) | 50 | 11,6 | 3,6″ (muy saturada) |
| MATAR (FK5 857) | 17 | 11,6 | 1,5″ |
| POLARIS (FK5 907) | 3 | 16,2 | 1,6″ |
| FK51481 (FK5 1481) | 2 | 16,9 | 0,7″ (apenas saturada) |

**Con pista** (`--hint`: posición y radio < 180): la búsqueda en espiral de ASTAP desde esa posición, con su propio
detector. ~90 ms por imagen (Matar); el mismo campo a menos de 3″ (test `IT_SolveRasa`).

**Medida de una estrella** donde la solución la predice: el pico se busca a ±8 px, el fondo es el borde de una ventana
de 101 px, y luego el centroide `hfd()` de ASTAP o, si tiene más de 100 píxeles saturados, el centroide de lo que pasa
de la mitad del pico. En RAW8 las estrellas brillantes saturan y su centroide es la mayor fuente de error: 1–3,5″
(Altair, Polaris, Matar), frente a 0,7″ de FK5 1481, que satura poco (11). Sin SIP, las FK5 del borde del campo fallan
2–5″.

## 3. La estrella de SFEL (`measure::measureSpot`, `dpot_spot`)

Cada celda Bayer 2×2 se suma en un píxel gris, para que el patrón de color no desplace el centroide. Sobre ella:

- **cog:** centro de gravedad de lo que pasa de 3σ sobre el fondo (un anillo alrededor de la estrella), recentrado una
  vez, como el `hfd()` de ASTAP;
- **win:** centroide con ventana gaussiana del tamaño de la propia estrella (el XWIN de SExtractor), el preciso.

En la sesión de Matar (50 imágenes), la estrella está en (1689,6 ± 4,4; 1607,6 ± 18,1) px: el seeing y el seguimiento,
no el método. Respecto al centro de la imagen: Az·cos El −18″, El −40″.

**Diafragma de campo** (`detection::fieldMask` y `fitFieldStop`): la media del plano verde de una sesión deja ver el
círculo iluminado; su centro, (1964,5; 1159,7) px en Matar, es la mejor estimación de dónde cae el eje del telescopio.

**Exceso de verde** (`detection::greenExcess`): G − k·(R + B)/2 de la media, sin la estrella, filtrado paso alto. El haz
de 532 nm solo enciende los píxeles verdes; estrellas y cielo son casi blancos. **Sin probar con el haz:** no hay todavía
imágenes con el láser.

## 4. Apuntado (`pointing`)

- `toHorizon`: J2000 → Az/El con precesión IAU 1976 y tiempo sidéreo medio. Sin nutación (~17″), aberración (~20″) ni
  refracción (~1′ a 45°): vale para la geometría de las cámaras y para diferencias entre puntos cercanos, **no** para
  un modelo de apuntado absoluto.
- `CameraModel` (escala, giro, píxel del eje) convierte un desplazamiento en la imagen a los ejes de la montura;
  `starOffset` da estrella − eje y `tpointPair`, con la lectura de los encoders, el par para TPOINT.
- Lo medido en las cuatro estrellas (11): el eje del RASA apunta +117…+161″ en Az·cos El y +44…+55″ en El de la
  estrella; en SFEL el patrón de Az sigue +10″ − 87″·cos El, compatible con IA más CA o con NPAE más CA: con cuatro
  estrellas no se separan.

**La hora de cada imagen** importa: un segundo son 15″ de ángulo horario. `images/frame_times.csv` tiene la hora a la
que se escribió cada PNG, al milisegundo, del reloj del PC de captura (su exactitud absoluta no está comprobada). Sin
ese fichero, la hora es la del inicio de la sesión (al segundo) o la del nombre (al minuto).

## 5. Lo que queda abierto

- **Satélites:** nada hecho todavía, salvo `findBlobs`.
- **Haz láser:** el mapa de exceso de verde, sin imágenes con el haz para probarlo.
- **Tiempo real:** lectura de cámara en directo, un hilo por cámara, presupuesto por imagen.
- **De la estación** (11, 12):
  - el píxel del C-SPAD (su campo de 50″ es del log ILRS de 2025 y no está confirmado);
  - una serie de estrellas con encoders y UTC, repartidas en Az y El, para separar los términos del modelo;
  - una cruz de *jog* de la montura para calibrar el giro;
  - imágenes RAW16 o menos saturadas;
  - imágenes con el láser.
- **Apuntado de precisión:** lugar aparente y observado completos (SuperNOVAS).
