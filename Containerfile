FROM alt:latest

RUN apt-get update && apt-get install -y \
    gcc make pkg-config libjansson-devel libcurl-devel \
    && apt-get clean

WORKDIR /app
COPY . .

RUN make clean && make

# JSON array - arguments are passed correctly!
ENTRYPOINT ["./alttest"]
CMD ["x86_64", "sisyphus", "p11"]
