#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define URL "http://api:8000/location"

// Function to parse a NMEA line and extract latitude and longitude
void parseLatLon(char* line, double* time, double* latitude, double* longitude) {
  int i = 0;
  char* token;

  token = strtok(line, ",");

  // Check message is of assumed message type for our receiver
  if (strncmp(token, "$GPRMC", 6) == 0) {
    // Read pos status field. 
    token = strtok(NULL, ",");
    *time = atof(token);
    token = strtok(NULL, ","); // pos_status field. 
    if (token != NULL && token[0] != 'A') {
      printf("Skipping record with invalid position.\n");
      return;
    }
  }
  else if (strncmp(token, "$GPGGA", 6) == 0) {
    token = strtok(NULL, ",");
    *time = atof(token);
    // There is no pos status field; continue on. 
  }
  else {
    printf("Unsupported format line: %s\n", line);
    return;
  }


  while (token != NULL) {
    switch (i) {
    case 1: // latitude
      *latitude = atof(token) / 100.0;
      break;
    case 2: // latitude direction (N/S)
      if (token[0] == 'S')
        *latitude = -(*latitude);
      break;
    case 3: // longitude
      *longitude = atof(token) / 100.0;
      break;
    case 4: // longitude direction (E/W)
      if (token[0] == 'W')
        *longitude = -(*longitude);
      break;
    }
    token = strtok(NULL, ",");
    i++;
  }
}

// Function to upload position data to API server
void upload_position(double latitude, double longitude)
{
  char command[256];
  snprintf(command, sizeof(command),
    "curl -X POST -H \"Content-Type: application/json\" -d '{\"latitude\": %.6f, \"longitude\": %.6f}' %s",
    latitude, longitude, URL);
  printf("Command: %s\n", command);
  system(command);
}

int main(int argc, char* argv[])
{
  double latitude = 0.0, longitude = 0.0, time = 0.0;

  if (argc != 2)
  {
    printf("Usage: %s <nmea_data_file>\n", argv[0]);
    return 1;
  }
  const char* GPS_FILE_PATH = argv[1];

  FILE* file = fopen(GPS_FILE_PATH, "r");
  if (file == NULL)
  {
    perror("Error opening GPS file");
    return 1;
  }

  char line[256];
  while (fgets(line, sizeof(line), file))
  {
    latitude = 0.0;
    longitude = 0.0;
    time = 0.0;
    parseLatLon(line, &time, &latitude, &longitude);
    //printf("Time: %f, Latitude: %f, Longitude: %f\n", time, latitude, longitude);
    upload_position(latitude, longitude);
  }

  fclose(file);
  return 0;
}
