# Image name (change this to your Docker Hub or registry path)
IMAGE_NAME = pinggy/pinggy_core_build
# Optional: tag (e.g., latest or from git commit)
TAG = latest

# Full image reference
FULL_IMAGE = $(IMAGE_NAME):$(TAG)

all: build-local

# Create a new builder instance if one doesn't exist
.PHONY: builder
builder:
	docker buildx create --use --name multiarch-builder || true
	docker buildx inspect --bootstrap

# Build and push for amd64 and arm64
.PHONY: build
build: builder
	docker buildx build \
		--platform linux/amd64,linux/arm64 \
		--tag $(FULL_IMAGE) \
		--push \
		. \
		-f dockerbuild.Dockerfile

# Just build (no push)
.PHONY: build-local
build-local: builder
	docker buildx build \
		--platform linux/amd64,linux/arm64 \
		--tag $(FULL_IMAGE) \
		--load \
		. \
		-f dockerbuild.Dockerfile

# Clean up builder
.PHONY: clean
clean:
	docker buildx rm multiarch-builder || true
