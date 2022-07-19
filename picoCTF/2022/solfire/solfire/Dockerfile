FROM rust@sha256:0372c87d4372dc1ae23caaf287f643f7c5ca799b40405efbb1bdf67588cc2629

RUN apt-get update -y && apt-get install libudev-dev -y

WORKDIR /app
COPY Cargo.toml Cargo.lock ./
COPY src ./src

ARG FLAG
ENV FLAG=$FLAG

RUN cargo build --release

COPY solfire.so ./

CMD [ "./target/release/solfire" ]


