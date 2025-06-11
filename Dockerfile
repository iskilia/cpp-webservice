# ============================================================================
# Dockerfile - Multi-stage build for AWS deployment
# ============================================================================

# Build stage
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the application
RUN if [ -d "build" ] ; then echo Build folder already exists ; else mkdir build ; fi
RUN cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    ca-certificates \
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
