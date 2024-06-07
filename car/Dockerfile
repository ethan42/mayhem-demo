FROM gcc:latest
RUN apt-get update && apt-get install -y --no-install-recommends curl netcat-traditional libc6-dbg

# Copy in source code
WORKDIR /app
COPY src src

ENV DEBUG true
ARG TEST_ARG=TRUE
ENV TEST=$TEST_ARG
# TEST vs PROD builds
RUN if [ "$TEST" = "TRUE" ]; then \
  gcc -o gps_uploader -g -DTEST -ftrapv src/gps_uploader.c; \
  else \
  gcc -o gps_uploader src/gps_uploader.c; \
  fi

# Some demo example GPS data
COPY gps_data.txt /testsuite/gps_data.txt
COPY gps_data.txt .

CMD ["/app/src/wait-for-it.sh", "api", "--", "/app/gps_uploader", "/testsuite/gps_data.txt"]
