FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libssl-dev \
    libsqlite3-dev \
    sqlite3 \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy server files
COPY server/ .
COPY database/ /app/database/

# Build the server
RUN make clean && make

# Expose port
EXPOSE 8888

# Run the server
CMD ["./bin/server"]
