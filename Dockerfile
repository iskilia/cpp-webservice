# Build stage
FROM ubuntu:24.04 AS builder

# Install build dependencies with newer GCC
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    gcc-13 \
    g++-13 \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the application
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:24.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -r -s /bin/false appuser

# Copy binary from build stage
COPY --from=builder /app/build/cpp-web-service /usr/local/bin/cpp-web-service
COPY --from=builder /app/build/cpp-web-service /app/cpp-web-service

# Set ownership and permissions
RUN chown appuser:appuser /app/cpp-web-service && \
    chmod +x /app/cpp-web-service

# Switch to non-root user
USER appuser
WORKDIR /app

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Start the service
CMD ["./cpp-web-service", "8080"]