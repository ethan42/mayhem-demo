FROM gcc:latest
RUN apt-get update && apt-get install -y --no-install-recommends build-essential curl netcat-traditional
WORKDIR /app
COPY . .
RUN gcc -o gps_uploader gps_uploader.c

CMD ["./wait-for-it.sh", "api", "--", "/app/gps_uploader", "gps_data.txt"]
