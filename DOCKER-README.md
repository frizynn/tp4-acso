# TP4 - Desarrollo en x86_64 Linux desde Mac M1

Este documento explica cómo desarrollar y probar el TP4 en arquitectura x86_64 Linux usando Docker desde tu Mac M1.

## 🚀 Setup Rápido

```bash
# 1. Configurar permisos (solo una vez)
chmod +x setup.sh && ./setup.sh

# 2. Construir entorno de desarrollo
./dev.sh build

# 3. Ejecutar todos los tests
./dev.sh test
```

## 📁 Estructura del Proyecto

```
tp4-acso/
├── src/
│   ├── ej1/ring.c      # Ejercicio 1: Comunicación en anillo
│   └── ej2/shell.c     # Ejercicio 2: Shell con pipes
├── tests/              # Suite completa de tests
├── Dockerfile          # Entorno x86_64 Linux
├── dev.sh             # Script principal de desarrollo
├── test-x86.sh        # Script de testing específico
└── setup.sh           # Configuración inicial
```

## 🛠️ Scripts Disponibles

### `./dev.sh` - Script Principal

```bash
./dev.sh build      # Construir imagen Docker x86_64
./dev.sh run        # Entorno interactivo de desarrollo
./dev.sh test       # Ejecutar todos los tests
./dev.sh test-ring  # Solo tests del anillo
./dev.sh test-shell # Solo tests del shell
./dev.sh clean      # Limpiar imágenes Docker
./dev.sh help       # Mostrar ayuda
```

### `./test-x86.sh` - Testing Específico

Ejecuta tests automáticos y manuales dentro del contenedor:
- Compilación de ambos ejercicios
- Tests automáticos completos
- Tests manuales con casos específicos
- Verificación de arquitectura

## 🐳 Uso con Docker

### Desarrollo Interactivo

```bash
# Entrar al entorno de desarrollo
./dev.sh run

# Dentro del contenedor:
cd src/ej1
make && ./ring 5 3 2

cd ../ej2  
make && ./shell
```

### Testing Automatizado

```bash
# Ejecutar todos los tests
./dev.sh test

# Ejecutar tests específicos
./dev.sh test-ring
./dev.sh test-shell
```

### Compilación Manual

```bash
# En el contenedor Docker:
cd src/ej1
make clean && make
./ring 5 3 2  # Debería imprimir: 8

cd ../ej2
make clean && make
printf 'echo "test" | cat\nexit\n' | ./shell
```

## 🔧 Características del Código

### Ejercicio 1: Ring Communication

**Archivo:** `src/ej1/ring.c`

**Características corregidas para x86_64:**
- Gestión correcta de pipes entre procesos
- Manejo de memoria compatible con arquitectura 64-bit
- Sincronización mejorada entre procesos padre e hijos
- Validación robusta de argumentos

**Uso:**
```bash
./ring <n> <valor_inicial> <proceso_inicio>

# Ejemplos:
./ring 5 3 2    # 5 procesos, valor inicial 3, inicia en proceso 2
./ring 3 -2 1   # 3 procesos, valor inicial -2, inicia en proceso 1
```

### Ejercicio 2: Shell with Pipes

**Archivo:** `src/ej2/shell.c`

**Características:**
- Shell interactivo completo
- Soporte para pipes múltiples (cmd1 | cmd2 | cmd3)
- Comando built-in `exit`
- Manejo de entrada desde terminal y pipes
- Gestión de procesos hijos

**Funcionalidades:**
```bash
Shell> echo "hello world"
Shell> ls | grep .c
Shell> ps | grep shell | wc -l
Shell> exit
```

## 🧪 Suite de Tests

### Tests Automáticos

- **Ring Tests:** 11 tests (7 básicos + 4 avanzados)
- **Shell Tests:** 12 tests (8 básicos + 4 avanzados)
- **Coverage:** Casos edge, valores negativos, pipes múltiples

### Ejecutar Tests

```bash
# Todos los tests
cd tests && make test

# Tests individuales
make test-ring
make test-shell
make test-advanced
```

## 🚨 Resolución de Problemas

### Docker no inicia

```bash
# Verificar que Docker Desktop esté ejecutándose
docker info

# Si hay problemas, reinicia Docker Desktop
```

### Tests fallan

```bash
# Limpiar y reconstruir
./dev.sh clean
./dev.sh build
./dev.sh test
```

### Problemas de arquitectura

```bash
# Verificar que estás usando x86_64
./dev.sh run
uname -m  # Debería mostrar: x86_64
```

## 📋 Checklist de Entrega

- [ ] Código compila sin warnings en x86_64
- [ ] Todos los tests automáticos pasan
- [ ] Tests manuales funcionan correctamente  
- [ ] Manejo correcto de memoria (sin leaks)
- [ ] Gestión adecuada de procesos e hijos
- [ ] Documentación completa

## 🎯 Comandos de Desarrollo Comunes

```bash
# Setup inicial (solo una vez)
./setup.sh

# Desarrollo diario
./dev.sh test              # Verificar que todo funciona
./dev.sh run               # Entorno de desarrollo interactivo

# Antes de entregar
./dev.sh clean && ./dev.sh test  # Test completo y limpio
```

## 📝 Notas para el Servidor

El código está optimizado para ejecutarse en servidores x86_64 Linux. Las principales consideraciones:

1. **Compatibilidad de Arquitectura:** Código probado específicamente en x86_64
2. **Gestión de Memoria:** Manejo correcto de punteros de 64-bit
3. **Syscalls:** Uso de system calls estándar de Linux
4. **Compilación:** Flags de compilación optimizados para el entorno objetivo

## 🆘 Soporte

Si encuentras problemas:

1. Verifica que Docker Desktop esté ejecutándose
2. Ejecuta `./dev.sh clean && ./dev.sh build`
3. Revisa los logs de compilación en busca de errores
4. Usa `./dev.sh run` para debugging interactivo
