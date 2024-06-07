#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define URL "http://api:8000/location"

#define CIN(x)  ((x) >= '0' && (x) <= '9')
#define CTN(x)  ((x) - '0')

// Damn vulnerable C stack exhaustion
void stack_exhaustion() {
  char buff[0x1000];
  while (1) {
    stack_exhaustion();
  }
}

void damn_vulnerable_c(char* line, int latitude, int longitude) {

  volatile int size1, size2, size3, size4;
  
  // A bit of computation on our inputs. 
  if (latitude < 0) {
    latitude = -latitude;
  }
  if (longitude < 0) {
    longitude = -longitude;
  }

  size1 = latitude / longitude; // divide by zero
  
  size2 = latitude + longitude; // integer overflow 0x7FFFFFFF+1=0

  size3 = latitude - longitude; // integer underflow 0-1 = -1

  if (size2 == 1000) // Stack exhausion 
    stack_exhaustion();

  size4 = strlen(line) + size2; 

  char* buff1 = (char*)malloc(size4); // Fails when size4 < 0

  // Out of bounds write (heap overflow)
  // when size4 < strlen(line) or <0
  strcpy(buff1, line);  

  char OOBR = buff1[size2]; // Out of bounds read/NULL pointer deref 

  free(buff1);

  if (size2 / 2 == 0) {
    free(buff1);   // Double Free = Free of Memory Not on Heap	
  }
  else {
    if (size2 / 3 == 0) {
      buff1[0] = 'a';     // Out of bounds write b.c. use-after-free
    }
  }

}


// Function to parse a NMEA line and extract latitude and longitude
void parseLatLon(char* line, double* time, double* latitude, double* longitude) {
  int i = 0;
  char* token;

  token = strtok(line, ",");

  // Check message is of assumed message type for our receiver
  if (strncmp(token, "$GPRMC", 6) == 0) { // OOB Read when token = NULL
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
#ifdef TEST
  printf("Command: %s\n", command);
#else 
  system(command);
#endif
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
  char unparsed_line[256];

  // Buffer overflow when line > 256
  while (fgets(line, sizeof(line), file))
  {
    latitude = 0.0;
    longitude = 0.0;
    time = 0.0;
    parseLatLon(line, &time, &latitude, &longitude);

    // Example vulnerable C 
    // Ported from https://github.com/hardik05/Damn_Vulnerable_C_Program/blob/master/imgRead.c
    damn_vulnerable_c(line, (int)latitude, (int)longitude);
    printf("Uploading Latitude: %f, Longitude: %f\n", latitude, longitude);
    upload_position(latitude, longitude);
  }

  fclose(file);
  return 0;
}
