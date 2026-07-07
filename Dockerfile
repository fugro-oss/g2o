# Builds the native g2o library and runs the .NET test suite on Linux.
#
#   docker build -t fugro-g2o .
#   docker run --rm fugro-g2o
#
# The image produced by the `build` stage contains the Linux native library at
# /app/build/bin/libfugro_g2o.so; use `docker build --output` with the `native`
# stage to extract it for packaging.

FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build

RUN apt-get update && \
    apt-get install -y --no-install-recommends cmake g++ make && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Build the native library first; it changes less often than the managed code.
COPY CMakeLists.txt ./
COPY External ./External
COPY Native ./Native
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j

# Build the managed library and tests.
COPY Fugro ./Fugro
RUN dotnet build Fugro/Test/Fugro.G2O.Test.csproj -c Release

# Optional stage to export just the native library:
#   docker build --target native --output type=local,dest=out .
FROM scratch AS native
COPY --from=build /app/build/bin/libfugro_g2o.so /

# Default stage: run the tests.
FROM build AS test
CMD ["dotnet", "test", "Fugro/Test/Fugro.G2O.Test.csproj", "-c", "Release", "--no-build"]
