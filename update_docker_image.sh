# Helper script to update the docker image for the mbedutils_dev container
docker build -t registry.ndandclicky.com/braunelectronics/dev_containers/mbedutils_dev:latest .
docker push registry.ndandclicky.com/braunelectronics/dev_containers/mbedutils_dev:latest