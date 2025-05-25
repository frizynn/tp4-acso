# Dockerfile para TP4 - Arquitectura y Sistemas Operativos
# Entorno x86_64 Linux para desarrollo y testing

ARG PLATFORM=linux/amd64
FROM --platform=${PLATFORM} ubuntu:22.04

# Evitar prompts interactivos durante la instalación
ENV DEBIAN_FRONTEND=noninteractive

# Actualizar el sistema e instalar herramientas de desarrollo
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    gdb \
    make \
    valgrind \
    strace \
    htop \
    procps \
    grep \
    vim \
    nano \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Crear usuario no root para desarrollo
RUN useradd -m -s /bin/bash developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Establecer directorio de trabajo
WORKDIR /tp4

# Cambiar al usuario developer
USER developer

# Configurar entorno
ENV HOME=/home/developer
ENV USER=developer

# Punto de entrada por defecto
CMD ["/bin/bash"]
