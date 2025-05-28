# TP4-ACSO - Shell & Ring Communication

## 🎯 **Proyecto Completo - Arquitectura y Sistemas Operativos**

Este proyecto implementa dos ejercicios fundamentales de sistemas operativos:
- **EJ1**: Comunicación en anillo entre procesos usando pipes
- **EJ2**: Shell interactivo con soporte completo de pipes y **manejo avanzado de comillas**

## ✨ **Características Destacadas**

### 🐚 **Shell con Extra Credit (EJ2)**
✅ **Manejo completo de comillas** - funciona exactamente como Linux:
- `ls | grep .zip` (sin comillas)
- `ls | grep ".zip"` (con comillas) 
- `ls | grep ".png .zip"` (patrones con espacios en comillas)

✅ **Funcionalidades avanzadas**:
- Pipes múltiples: `cmd1 | cmd2 | cmd3`
- Manejo robusto de memoria y procesos
- Modo debug con `SHELL_DEBUG=1`
- Gestión de señales (Ctrl+C)
- Compatibilidad total con bash

### 🔄 **Ring Communication (EJ1)**
✅ **Comunicación robusta entre procesos**:
- Arquitectura en anillo con pipes
- Sincronización correcta de procesos
- Manejo de valores negativos y casos edge
- Optimizado para x86_64

## 🚀 **Ejecución Rápida**

### **Opción 1: Auto-detección (Recomendado)**
```bash
./run-tests.sh all          # Detecta automáticamente Docker vs nativo
./run-tests.sh shell        # Solo tests del shell
./run-tests.sh ring         # Solo tests del ring
```

### **Opción 2: Servidor Linux x86_64 (Nativo)**
```bash
./run-tests.sh --native all       # Ejecución nativa completa
./run-tests.sh --native shell     # Solo shell en modo nativo
./run-tests.sh --native benchmark # Benchmark de rendimiento
```

### **Opción 3: Docker (Multiplataforma)**
```bash
./run-tests.sh --docker all       # Fuerza Docker x86_64
./run-tests.sh --docker shell     # Solo shell en Docker
```

## 📁 **Estructura del Proyecto**

```
tp4-acso/
├── src/
│   ├── ej1/
│   │   ├── ring.c              # Ring communication implementation
│   │   └── Makefile
│   └── ej2/
│       ├── shell.c             # Shell with advanced quote handling
│       └── Makefile
├── tests/
│   ├── ej1/                    # Ring tests (basic + advanced + extreme)
│   ├── ej2/                    # Shell tests (basic + extra credit + comprehensive)
│   │   ├── test_shell.c
│   │   ├── test_shell_advanced.c
│   │   ├── test_shell_extra_credit.c
│   │   └── test_quotes_comprehensive.c  # Tests exhaustivos de comillas
│   └── test_integration.c      # Integration tests
├── run-tests.sh               # Script principal (Docker/Native)
├── Dockerfile                 # Entorno x86_64 Ubuntu 22.04
└── DOCKER-README.md          # Documentación Docker específica
```

## 🛠️ **Compilación Manual**

### **EJ1 - Ring Communication**
```bash
cd src/ej1
make clean && make
./ring <n> <valor_inicial> <proceso_inicio>

# Ejemplos:
./ring 5 3 2    # 5 procesos, valor inicial 3, inicia en proceso 2
./ring 3 -2 1   # 3 procesos, valor inicial -2, inicia en proceso 1
```

### **EJ2 - Shell Interactivo**
```bash
cd src/ej2
make clean && make
./shell

# Ejemplos en el shell:
Shell> echo "hello world"
Shell> ls | grep .c
Shell> ls | grep ".zip"           # Extra credit con comillas
Shell> ls | grep ".png .zip"      # Extra credit con espacios
Shell> ps | grep shell | wc -l
Shell> exit
```

## 🧪 **Suite de Tests Completa**

### **Tests del Shell (EJ2)**
- **`test_shell`**: Tests básicos de funcionalidad
- **`test_shell_advanced`**: Tests avanzados y casos edge
- **`test_shell_extra_credit`**: Tests específicos de comillas
- **`test_quotes_comprehensive`**: Tests exhaustivos de compatibilidad bash

### **Tests del Ring (EJ1)**
- **`test_ring`**: Tests básicos de comunicación
- **`test_ring_advanced`**: Tests avanzados y edge cases
- **`test_ring_extreme`**: Tests de estrés y robustez

### **Ejecutar Tests Específicos**
```bash
# Tests individuales
cd tests/ej2 && make && ./test_quotes_comprehensive
cd tests/ej1 && make && ./test_ring_extreme

# Suite completa
./run-tests.sh all                    # Todos los tests + benchmark
./run-tests.sh shell                  # Solo tests del shell
./run-tests.sh ring                   # Solo tests del ring
./run-tests.sh benchmark              # Solo benchmark de rendimiento
```

## 🎯 **Verificación de Extra Credit**

Para verificar que el shell maneja correctamente las comillas:

```bash
# Compilar shell
cd src/ej2 && make

# Probar casos específicos del enunciado
echo 'ls | grep .zip' | SHELL_DEBUG=1 ./shell
echo 'ls | grep ".zip"' | SHELL_DEBUG=1 ./shell  
echo 'ls | grep ".png .zip"' | SHELL_DEBUG=1 ./shell

# Ejecutar tests exhaustivos
cd ../../tests/ej2
make && ./test_quotes_comprehensive
```

**Salida esperada con `SHELL_DEBUG=1`:**
```
Shell> Command 0: ls
Command 1: grep ".zip"
archivo.zip
Shell> 
```

## 🖥️ **Compatibilidad de Sistemas**

| Sistema | Modo Recomendado | Comando |
|---------|------------------|---------|
| **Linux x86_64** | Native | `./run-tests.sh --native all` |
| **macOS (Intel/M1)** | Docker | `./run-tests.sh --docker all` |
| **Windows + WSL** | Docker | `./run-tests.sh --docker all` |
| **Auto-detección** | Auto | `./run-tests.sh all` |

## 📊 **Resultados de Tests**

### **Estado Actual: ✅ 100% PASSED**

**Shell Tests (EJ2):** 5/5 ✅
- debug_test ✅
- test_shell ✅  
- test_shell_advanced ✅
- test_shell_extra_credit ✅
- test_quotes_comprehensive ✅

**Ring Tests (EJ1):** 4/4 ✅
- test_ring ✅
- test_ring_advanced ✅
- test_ring_advanced_strict ✅
- test_ring_extreme ✅

## 🏆 **Extra Credit Verificado**

✅ **Casos del enunciado completamente implementados:**

1. `ls | grep .zip` → Funciona (sin comillas)
2. `ls | grep ".zip"` → Funciona (con comillas)
3. `ls | grep ".png .zip"` → Funciona (espacios en comillas)

✅ **Casos adicionales verificados:**
- Comillas vacías: `echo "" | cat`
- Múltiples comillas: `echo "hello" | grep "hello"`
- Caracteres especiales: `ls | grep "[1]"`
- Espacios preservados: `echo " hello world " | cat`

## 🚀 **Deploy en Servidor Linux x86_64**

```bash
# 1. En el servidor, ejecutar tests
ssh usuario@servidor
cd /ruta/tp4-acso/
./run-tests.sh --native all

# 3. Verificar shell interactivo
cd src/ej2 && make && ./shell
```

## 🐳 **Docker (Desarrollo Multiplataforma)**

```bash
# Construir imagen
docker build --platform linux/amd64 -t tp4-shell .

# Entorno interactivo
docker run --platform linux/amd64 -it --rm \
  -v "$(pwd):/tp4" -w /tp4 tp4-shell bash

# Tests automatizados
./run-tests.sh --docker all
```

## 📝 **Logs y Debugging**

- **Logs automáticos**: `test-logs/test-run-TIMESTAMP.log`
- **Modo debug shell**: `SHELL_DEBUG=1 ./shell`
- **Limpieza**: `./run-tests.sh clean`

## 🎓 **Desarrollo y Testing**

### **Ciclo de desarrollo recomendado:**
```bash
# 1. Hacer cambios en src/ej2/shell.c
# 2. Probar rápidamente
cd src/ej2 && make && echo 'ls | grep ".c"' | ./shell

# 3. Tests exhaustivos
./run-tests.sh shell

# 4. Verificación completa
./run-tests.sh all
```

### **Para debugging:**
```bash
# Shell con debug
cd src/ej2 && echo 'comando' | SHELL_DEBUG=1 ./shell

# Tests individuales con más detalle
cd tests/ej2 && ./test_quotes_comprehensive

# Verificar arquitectura en Docker
docker run --rm tp4-shell uname -m  # Debe mostrar: x86_64
```

