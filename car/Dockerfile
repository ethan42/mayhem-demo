FROM gcc:latest
RUN apt-get update && apt-get install -y --no-install-recommends build-essential curl netcat-traditional
WORKDIR /app
COPY gps_uploader.c .
ENV DEBUG true
ARG TEST_ARG=TRUE
ENV TEST=$TEST_ARG
# TEST vs PROD builds
RUN if [ "$TEST" = "TRUE" ]; then \
  gcc -DTEST -o gps_uploader -g gps_uploader.c; \
  else \
  gcc -o gps_uploader gps_uploader.c; \
  fi


WORKDIR /testsuite
COPY gps_data.txt .

CMD ["./wait-for-it.sh", "api", "--", "/app/gps_uploader", "/testsuite/gps_data.txt"]
