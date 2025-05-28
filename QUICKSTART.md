# 🚀 QUICKSTART - TP4-ACSO

## ⚡ **Ejecución Inmediata**

### **En servidor Linux x86_64:**
```bash
./run-tests.sh --native all
```

### **En macOS/desarrollo:**
```bash
./run-tests.sh --docker all
```

### **Auto-detección:**
```bash
./run-tests.sh all
```

## 🎯 **Verificar Extra Credit**

```bash
# Compilar shell
cd src/ej2 && make

# Probar casos específicos
echo 'ls | grep .zip' | SHELL_DEBUG=1 ./shell
echo 'ls | grep ".zip"' | SHELL_DEBUG=1 ./shell  
echo 'ls | grep ".png .zip"' | SHELL_DEBUG=1 ./shell

# Tests exhaustivos
cd ../../tests/ej2 && make && ./test_quotes_comprehensive
```
