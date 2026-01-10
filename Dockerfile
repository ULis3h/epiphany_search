FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the application
RUN make

# Expose port
EXPOSE 8080

# Run the server
CMD ["./epiphany_search"]
