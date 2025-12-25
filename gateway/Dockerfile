FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libssl-dev \
    nlohmann-json3-dev \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy gateway files
COPY gateway/ .

# Build the gateway
RUN make clean && make

# Expose port
EXPOSE 8080

# Run the gateway
CMD ["./bin/gateway"]
