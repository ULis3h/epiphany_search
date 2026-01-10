#!/bin/bash
set -e

# Build the image
echo "Building Docker image..."
docker build -t epiphany_search .

# Check if a container is already running
if [ "$(docker ps -q -f name=epiphany_container)" ]; then
    echo "Stopping existing container..."
    docker stop epiphany_container
fi

# Run the container
echo "Starting container on port 8080..."
docker run --rm -d -p 8080:8080 --name epiphany_container epiphany_search

echo "Deployment complete! Access at http://localhost:8080"
