FROM alpine:edge
RUN apk update && \
	apk add --no-cache build-base~=0.5
WORKDIR /usr/local/bin
COPY aa3d.c .
COPY Makefile .
RUN make
