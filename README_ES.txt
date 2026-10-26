TSMANIA TUNE — fuente v0.2 experimental
======================================

Este es un prototipo experimental de corrección de afinación monofónica. Incluye:
- interfaz roja/negra y parámetros automatizables/guardables;
- seguimiento básico de tono monofónico;
- selección de tonalidad y escala mayor/menor/cromática;
- corrección de tono aproximada mediante el procesador PitchShifter de JUCE;
- controles de velocidad, humanize/flex como ajustes iniciales de comportamiento.

ESTADO Y LIMITACIONES IMPORTANTES
---------------------------------
Esta versión NO es un producto terminado ni una alternativa equivalente a Auto-Tune Pro.
No se ha compilado ni probado en FL Studio desde este entorno. La detección por bloques y
el cambio de tono pueden producir saltos, artefactos, falsas detecciones y latencia. Los
controles Formant y parte de los parámetros son todavía visuales/experimentales; no se
debe asumir que realizan procesamiento equivalente al de un afinador comercial. No se
garantiza latencia cero ni calidad profesional. Prueba primero con copias de audio y
no uses en una entrega comercial sin validar.

Requisitos de compilación en Windows 10 x64
-------------------------------------------
1. Instala Visual Studio Community y selecciona “Desarrollo para el escritorio con C++”.
2. Incluye MSVC, Windows 10 SDK y C++ CMake tools for Windows.
3. Instala Git para Windows.
4. Desde la carpeta de este proyecto, ejecuta en una terminal:

   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release

CMake descarga JUCE 8.0.6 de su repositorio durante la configuración inicial; requiere
conexión a internet. Si la compilación falla, revisa la salida completa de CMake/MSBuild.

Resultado esperado si compila correctamente:
   build/TSMANIA_TUNE_artefacts/Release/VST3/TSMANIA TUNE.vst3

Instalación manual
------------------
Copia la carpeta “TSMANIA TUNE.vst3” a:
   C:\\Program Files\\Common Files\\VST3\\
Luego abre FL Studio > Options > Manage plugins > Find installed plugins.

Antes de usar
-------------
- Empieza con un proyecto duplicado y búfer ASIO moderado.
- Comprueba afinación, ruido/artefactos, estabilidad y latencia con voces reales.
- Si aparecen fallos de audio, retira el plugin y vuelve a la versión anterior.
- No se incluye un instalador binario compilado en este ZIP; se distribuye el código fuente.


OPCIÓN SIN INSTALAR VISUAL STUDIO EN TU PC (COMPILACIÓN EN LA NUBE)
-------------------------------------------------------------------
Se añadió .github/workflows/build-windows.yml para compilar en un runner de Windows
mediante GitHub Actions. Esto evita instalar Visual Studio localmente, pero requiere
una cuenta de GitHub y subir este proyecto a un repositorio propio. Después entra a la
pestaña Actions, ejecuta “Build TSMANIA TUNE (Windows VST3)” y descarga el artefacto
TSMANIA_TUNE_VST3_Windows_x64. La compilación puede fallar y debe revisarse; el flujo
no convierte este prototipo en un producto profesional validado automáticamente.
